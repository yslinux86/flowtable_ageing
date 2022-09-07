#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>

#include "ageing.h"
#include "flow.h"
#include "hashtable.h"


//最新位置
extern uint8_t fresh_class;

//线程开关
static bool ageing_flag[AGEING_CLASS_NUM];
static pthread_mutex_t ageinglock[AGEING_CLASS_NUM];

//这里 老化链表数组 和 待处理数据链表数组均为头部节点，头部节点为特殊节点，不保存实际数据，保存该链表的节点数
//老化数组，有 AGEING_CLASS_NUM 个老化层级
static GList* ageinglist[AGEING_CLASS_NUM];

void update_ageing_list(node_data_t* nodedata)
{
    uint32_t class = nodedata->my_class;
    uint8_t this_class = fresh_class;

    pthread_mutex_lock(&ageinglock[class]);
    ageinglist[class] = g_list_remove(ageinglist[class], nodedata);
    pthread_mutex_unlock(&ageinglock[class]);

    pthread_mutex_lock(&ageinglock[this_class]);
    ageinglist[this_class] = g_list_append(ageinglist[this_class], nodedata);
    pthread_mutex_unlock(&ageinglock[this_class]);
    nodedata->my_class = this_class;
}

//将流追加到最新老化待处理 双链表头节点后面
void append_ageing_list(node_data_t* nodedata)
{
    uint8_t this_class = nodedata->my_class;

    pthread_mutex_lock(&ageinglock[this_class]);
    ageinglist[this_class] = g_list_append(ageinglist[this_class], nodedata);
    pthread_mutex_unlock(&ageinglock[this_class]);
}

static void foreach_func_cb(gpointer data, gpointer userdata)
{
    node_data_t* nodedata = (node_data_t*)data;
    uint8_t class = nodedata->my_class;
    uint32_t index = nodedata->index;

    print_nodedata("\nFREE node\n", nodedata);
    if (index >= HASHTABLE_SIZE)
    {
        printf("\ninvalid index\n");
        print_nodedata("\nINVALID node\n", nodedata);
    }

    pthread_mutex_lock(&ageinglock[class]);
    ageinglist[class] = g_list_remove(ageinglist[class], data);
    pthread_mutex_unlock(&ageinglock[class]);

    destroy_bucket_node(data, index);
}

uint32_t get_size_ageinglist(uint8_t class)
{
    uint32_t length = g_list_length(ageinglist[class]);
    return length;
}

//创建 AGEING_CLASS_NUM 个线程，每个线程处理相应层级的待处理数据链表 和 老化链表，互不干涉
//将待处理数据链表中的数据添加到老化链表
//到了老化时机，删除老化链表的所有节点
//扫描本老化链表，将action标记为remove的节点移出，放到相应的待处理数据链表中
static void* process_flow(void* arg)
{
    uint8_t class = *(uint8_t*)arg;
    char thread_name[16];

    //设置线程名称
    sprintf(thread_name, "processflow_%u", class);
    // printf("thread name=%s\n", thread_name);
    prctl(PR_SET_NAME, thread_name);

    while (ageing_flag[class])
    {
        if (class == (fresh_class + 1) % AGEING_CLASS_NUM)
        {
            if (g_list_length(ageinglist[class]))
            {
                //老化所有节点
                printf("------------------- prepare to DELETE my class %u ageing list, fresh_class=%u, length = %u\n", 
                    class, fresh_class, g_list_length(ageinglist[class]));

                g_list_foreach(ageinglist[class], foreach_func_cb, NULL);
            }
        }
        else
        {
            sleep(1);
        }
    }

    printf("%s exit\n", thread_name);
    return NULL;
}

int start_ageing_work()
{
    uint8_t index;
    pthread_t pid[AGEING_CLASS_NUM];
    int ret;

    for (index = 0; index < AGEING_CLASS_NUM; index++)
    {
        pthread_mutex_init(&ageinglock[index], NULL);
    }

    for (index = 0; index < AGEING_CLASS_NUM; index++)
    {
        ageing_flag[index] = 1;
        ret = pthread_create(&pid[index], NULL, process_flow, &index);
        assert(ret == 0);
        usleep(100);
    }

    return ret;
}

void stop_ageing_work()
{
    unsigned char i;

    for (i = 0; i < AGEING_CLASS_NUM; i++)
    {
        ageing_flag[i] = 0;
    }
}
