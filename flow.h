#ifndef __FLOW_H__
#define __FLOW_H__

#include <glib.h>
#include <sys/prctl.h>

#include "sm3.h"

//五元组
typedef struct five_tuple {
    unsigned int src_ip;
    unsigned int dst_ip;
    unsigned int src_port;
    unsigned int dst_port;
    char protocol[8];
} five_tuple_t;


//链表节点数据
typedef struct node_data {
    uint32_t index;            //在哈希表中的索引，来一条流时根据五元组计算出index，查看哈希表中是否存在该流
    uint8_t my_class;          //该流在老化层级中的位置（level）

    five_tuple_t fivetuple;         //五元组
} node_data_t;

void create_rand();
void destroy_rand();

void print_nodedata(const char* title, void* data);
void print_nodedata1(const char* title, void* data);
int start_generate_fivetuple();
void stop_generate_fivetuple();
uint32_t cal_sm3_data(five_tuple_t* fivetuple);


#endif
