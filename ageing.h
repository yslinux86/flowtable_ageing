#ifndef __AGING_H__
#define __AGING_H__

#include <glib.h>
#include "flow.h"

#define AGEING_CLASS_NUM  30


void append_ageing_list(node_data_t* nodedata);
void update_ageing_list(node_data_t* nodedata);
uint32_t get_size_ageinglist(uint8_t class);

int start_ageing_work();
void stop_ageing_work();


#endif
