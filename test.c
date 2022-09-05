#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <glib.h>

#include "timer.h"
#include "hashtable.h"
#include "ageing.h"
#include "flow.h"

static void signal_cb(int signo)
{
    printf("%s, recv signal %d\n", __func__, signo);

    switch(signo)
    {
    case SIGINT:
    case SIGTERM:
    case SIGQUIT:
    case SIGTSTP:
    	stop_timer();
        stop_show_hashtable();
        stop_generate_fivetuple();
        stop_ageing_work();
        destroy_rand();
        break;
    }

    exit(0);
}

void signal_init(void)
{
    signal(SIGINT,  signal_cb);
    signal(SIGTERM, signal_cb);
    signal(SIGTSTP, signal_cb);
    signal(SIGQUIT, signal_cb);
}

void test_random_num()
{
    GRand* rand;

    rand = g_rand_new_with_seed(0);
    while(1)
    {
        sleep(0.1);
        printf("%u\n", g_rand_int(rand));
    }
}

int main(int argc, char** argv)
{
    signal_init();
    create_rand();
    // test_random_num();

    start_timer(60);
    start_show_hashtable(60);
    
    start_ageing_work();
    start_generate_fivetuple();

	while(1)
		pause();

	return 0;
}
