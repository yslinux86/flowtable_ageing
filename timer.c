#include <pthread.h>
#include <unistd.h>
#include <malloc.h>
#include <stdbool.h>
#include <assert.h>
 
#include "timer.h"
#include "flow.h"
#include "ageing.h"

uint8_t fresh_class;
static bool timer_runing_flag;

void* update_ageing_class(void* arg)
{
    uint8_t timegap = *(uint8_t*)arg;
    prctl(PR_SET_NAME, "update_ageing_class");
    // printf("timegap=%u\n", timegap);

    while(timer_runing_flag)
    {
        sleep(timegap);

        printf("last class=[%u], last class ageing list length = [%u]\n", 
            fresh_class, get_size_ageinglist(fresh_class));

        fresh_class++;
        if (fresh_class >= AGEING_CLASS_NUM)
            fresh_class = 0;
    }

    printf("%s, exit\n", __func__);
    return NULL;
}

void start_timer(uint8_t timegap)
{
    timer_runing_flag = 1;
    pthread_t pid;
    int ret;

    if (timegap < 5 || timegap > 120)
        timegap = 30;

    ret = pthread_create(&pid, NULL, update_ageing_class, &timegap);
    assert(ret == 0);
    usleep(100);
}

void stop_timer()
{
    timer_runing_flag = 0;
}
