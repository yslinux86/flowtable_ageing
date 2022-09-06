#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include <glib.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "flow.h"
#include "sm3.h"
#include "hashtable.h"

#define MAX_PORT  65536
static bool flow_runing_flag;
extern uint8_t fresh_class;

char protocol[][8] =
{
    "tcp",
    "udp",
    "snmp",
    "http",
    "https"
};

uint32_t cal_sm3_data(five_tuple_t* fivetuple)
{
    uint32_t tmp;
    uint32_t index;
    BYTE digest[32];

    TCM_SM3_soft((BYTE*)fivetuple, sizeof(fivetuple), digest);
    memcpy(&tmp, digest + 28, 4);
    index = tmp % HASHTABLE_SIZE;

    return index;
}

//创建随机数发生器
GRand* rand1;
void create_rand()
{
    rand1 = g_rand_new_with_seed(0);
}

void destroy_rand()
{
    g_rand_free(rand1);
}

/*
对五元组信息进行处理
1、计算哈希值
2、根据哈希值得到在哈希表中位置
3、检查哈希表中是否已存在该流
3.1、已存在，但五元组不同，存在哈希碰撞，添加到哈希表节点后面链表中，并添加到当前最新层级待处理数据队列中
3.2、已存在，五元组相同，说明是之前已来过的流，查看该流的等级，不在当前最新等级时，将动作标记为移除
3.3、不存在，添加到哈希表中，并添加到当前最新的层级待处理数据队列中
4、每个待处理数据的队列有相应的线程处理，不存在多线程处理同一个队列，不存在竞争关系【不在这里处理】
*/
void process_fivetuple()
{
    BYTE digest[32];
    node_data_t* flowinfo;
    guint value[5];
    int ii;

    flowinfo = malloc(sizeof(node_data_t));
    assert(flowinfo);
    memset(flowinfo, 0, sizeof(node_data_t));

    value[0] = g_rand_int(rand1);
    value[1] = g_rand_int(rand1);
    value[2] = g_rand_int_range(rand1, 1, MAX_PORT);
    value[3] = g_rand_int_range(rand1, 1, MAX_PORT);
    value[4] = g_rand_int_range(rand1, 0, 5);

    flowinfo->fivetuple.src_ip = value[0];
    flowinfo->fivetuple.dst_ip = value[1];
    flowinfo->fivetuple.src_port = value[2];
    flowinfo->fivetuple.dst_port = value[3];
    strcpy(flowinfo->fivetuple.protocol, protocol[value[4]]);
    //计算五元组的sm3哈希值，通过哈希值的末4位计算哈希表中位置
    flowinfo->index = cal_sm3_data(&flowinfo->fivetuple);
    flowinfo->my_class = fresh_class;

    // print_nodedata1("create new node\n", flowinfo);
    find_hashbucket(flowinfo);
}
#if 1
void print_nodedata(const char* title, void* data)
{

}

void print_nodedata1(const char* title, void* data)
{
    node_data_t* nodedata = (node_data_t*)data;
    five_tuple_t* fivetuple = &nodedata->fivetuple;

    struct in_addr in[2];
    in[0].s_addr = fivetuple->src_ip;
    in[1].s_addr = fivetuple->dst_ip;

    printf("%s", title);
    printf("src ip=%s\n", inet_ntoa(in[0]));
    printf("dst ip=%s\n", inet_ntoa(in[1]));
    printf("src port=%u\n", fivetuple->src_port);
    printf("dst port=%u\n", fivetuple->dst_port);
    printf("protocol=%s\n", fivetuple->protocol);
    printf("index=%u\n", nodedata->index);
    printf("my_class=%u\n", nodedata->my_class);
    printf("node ptr=%p\n", data);
}
#else
void print_nodedata(const char* title, void* data)
{
    node_data_t* nodedata = (node_data_t*)data;
    five_tuple_t* fivetuple = &nodedata->fivetuple;

    struct in_addr in[2];
    in[0].s_addr = fivetuple->src_ip;
    in[1].s_addr = fivetuple->dst_ip;

    printf("\n%s\n", title);
    printf("src ip=%s\n", inet_ntoa(in[0]));
    printf("dst ip=%s\n", inet_ntoa(in[1]));
    printf("src port=%u\n", fivetuple->src_port);
    printf("dst port=%u\n", fivetuple->dst_port);
    printf("protocol=%s\n", fivetuple->protocol);
    printf("index=%u\n", nodedata->index);
    printf("my_class=%u\n", nodedata->my_class);
    printf("node ptr=%p\n", data);
}
#endif
//生成伪五元组，模拟真实网络五元组数据，可由多个线程并发
void* generate_pseudo_five_tuple(void* arg)
{
    prctl(PR_SET_NAME, "generate_pseudo_five_tuple");

    while(flow_runing_flag)
    {
        // usleep(1);
        // usleep(10);
        // usleep(100000);
        process_fivetuple();
    }

    printf("%s exit\n", __func__);
    return NULL;
}

int start_generate_fivetuple()
{
    flow_runing_flag = 1;
    int ret;
    pthread_t pid[10];
    int i;

    for (i = 0; i < 10; i++)
    {
        ret = pthread_create(&pid[i], NULL, generate_pseudo_five_tuple, NULL);
        assert(ret == 0);
    }

    return 0;
}

void stop_generate_fivetuple()
{
    flow_runing_flag = 0;
}

