#ifndef __HASHTABLE_H__
#define __HASHTABLE_H__

#include <stdbool.h>
#include "flow.h"


//用于存放五元组信息的哈希表大小
#define HASHTABLE_SIZE    10000000

void destroy_bucket_node(gpointer data, uint32_t index);
void find_hashbucket(node_data_t* flowinfo);
void start_show_hashtable(uint8_t showtimegap);
void stop_show_hashtable();

#endif
