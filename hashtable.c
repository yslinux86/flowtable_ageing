#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "hashtable.h"
#include "flow.h"
#include "ageing.h"

static bool runing_flag;

//最新位置，定时增长，老化位置为最新位置的下一位置
extern uint8_t fresh_class;

static GList *hash_bucket[HASHTABLE_SIZE];    //哈希桶


//将流插入到哈希表和老化链表中
void insert_hashtbucket_agelist(node_data_t* flowinfo)
{
    uint32_t index = flowinfo->index;

    hash_bucket[index] = g_list_append(hash_bucket[index], flowinfo);
    append_ageing_list(flowinfo);
}

void destroy_bucket_node(gpointer data, uint32_t index)
{
    if (index >= HASHTABLE_SIZE)
        return;

    if (hash_bucket[index])
    {
        hash_bucket[index] = g_list_remove(hash_bucket[index], data);
        if (data)
        {
            node_data_t* node = (node_data_t*)data;
            g_free(node);
        }
    }
    else
    {
        printf("IMPOSSIBLE\n");
    }
}

/* newnode为新生成的五元组节点数据，oldnode为已存在于哈希链表中的旧数据 */
static gint compare_node(gconstpointer oldnode, gconstpointer newnode)
{
    node_data_t* _nodedata = (node_data_t*)oldnode;
    node_data_t* _userdata = (node_data_t*)newnode;

    // print_nodedata1("---------------- old node\n", _nodedata);
    // print_nodedata1("---------------- new node\n", _userdata);

    if (memcmp((void*)&_nodedata->fivetuple, (void*)&_userdata->fivetuple, sizeof(five_tuple_t)) == 0)
    {
        printf("[SAME] five tuple\n");
        return 0;
    }
    else
    {
        // printf("**************** diff five tuple\n");
        return -1;
    }
}

void print_nodedata_cb(gpointer data, gpointer userdata)
{
    node_data_t* nodedata = (node_data_t*)data;
    five_tuple_t* fivetuple = &nodedata->fivetuple;

    struct in_addr in[2];
    in[0].s_addr = fivetuple->src_ip;
    in[1].s_addr = fivetuple->dst_ip;

    printf("src ip=%s\n", inet_ntoa(in[0]));
    printf("dst ip=%s\n", inet_ntoa(in[1]));
    printf("src port=%u\n", fivetuple->src_port);
    printf("dst port=%u\n", fivetuple->dst_port);
    printf("protocol=%s\n", fivetuple->protocol);
    printf("index=%u\n", nodedata->index);
    printf("my_class=%u\n", nodedata->my_class);
    printf("node ptr=%p\n", data);
}

void show_bucket_by_index(uint32_t index)
{
    printf("\nhash_bucket[%u] length = %u\tFOREACH HASH BUCKET:\n", index, g_list_length(hash_bucket[index]));
    g_list_foreach(hash_bucket[index], print_nodedata_cb, NULL);
}

//查询哈希表中是否已存在该五元组信息
void find_hashbucket(node_data_t* flowinfo)
{
    uint32_t index = flowinfo->index;
    GList* foundnode;

    if (hash_bucket[index])
    {
        // printf("exist in hash table\n");
        //对比已存哈希表中五元组信息，是否一致（可能存在哈希碰撞）
        //如果一致，则更新老化表中等级，移至最新状态
        // show_bucket_by_index(index);
        // print_nodedata1("=================== flowinfo:\n", flowinfo);
        foundnode = g_list_find_custom(hash_bucket[index], flowinfo, compare_node);
        // printf("foundnode=%p\n", foundnode);
        if (foundnode == NULL)
        {
            //没有找到，该流为新流，存在多个流哈希碰撞
            hash_bucket[index] = g_list_append(hash_bucket[index], flowinfo);
            append_ageing_list(flowinfo);
            // printf("-------------------------\n");
        }
        else
        {
            //存在同样的流，只需更新流状态等级
            node_data_t* nodedata = foundnode->data;
            // print_nodedata1("\nnew node\n", flowinfo);
            // print_nodedata1("\nold node\n", nodedata);

            free(flowinfo);
            if (nodedata->my_class != fresh_class)
                update_ageing_list(nodedata);
        }
    }
    else
    {
        // printf("NOT exist in hash table\n");
        //将该流添加到哈希桶和老化链表中
        insert_hashtbucket_agelist(flowinfo);
    }

    return;
}

//展示哈希表信息，目前100万条流
static void* show_hashbucket(void* arg)
{
    unsigned long i;
    runing_flag = 1;
    unsigned long flow_size;
    uint8_t showtimegap = *(uint8_t*)arg;
    prctl(PR_SET_NAME, "show_hashbucket");

    while(runing_flag)
    {
        sleep(showtimegap);

        flow_size = 0;
        // printf("------------------------------------\n");
        for (i = 0; i < HASHTABLE_SIZE; i++)
        {
            if (hash_bucket[i])
            {
                flow_size += g_list_length(hash_bucket[i]);
            }
        }
        // printf("------------------------------------\n");
        printf("there are %lu flows in hash table, fresh_class = %u\n", flow_size, fresh_class);
    }

    printf("%s exit\n", __func__);

    return NULL;
}

/* 每隔 showtimegap 秒 展示一次哈希桶里存放的流 */
void start_show_hashtable(uint8_t showtimegap)
{
    int ret;
    pthread_t pid;

    if (showtimegap < 30 || showtimegap > 600)
        showtimegap = 60;

    ret = pthread_create(&pid, NULL, show_hashbucket, &showtimegap);
    assert(ret == 0);
    usleep(100);
}

void stop_show_hashtable()
{
    runing_flag = 0;
}
