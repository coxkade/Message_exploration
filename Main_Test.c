/**
 * @file Main_Test.c
 * @author Kade Cox
 * @date Created: Mar 6, 2020
 * @details
 *
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <stdbool.h>

/***********************************************************************************/
/***************************** Defines and Macros **********************************/
/***********************************************************************************/

//Macro that gets the number of elements supported by the array
#define ARRAY_MAX_COUNT(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

#ifndef ST_NS_PER_MS
#define ST_NS_PER_MS (1000)
#endif //ST_NS_PER_MS

/***********************************************************************************/
/***************************** Type Defs *******************************************/
/***********************************************************************************/

/***********************************************************************************/
/***************************** Function Declarations *******************************/
/***********************************************************************************/

/***********************************************************************************/
/***************************** Static Variables ************************************/
/***********************************************************************************/

/***********************************************************************************/
/***************************** Function Definitions ********************************/
/***********************************************************************************/

/**
 * Internal function for sleeping ns
 * @param ns
 */
static inline void test_sleep_ns(unsigned int ns)
{
    struct timespec time_data =
    {
        .tv_sec = 0,
        .tv_nsec = 0
    };
    time_data.tv_nsec = ns;
    while(0 != nanosleep(&time_data, &time_data)) {}
}

/**
 * Internal function for sleeping ms
 * @param ms
 */
static void test_sleep_ms(unsigned int ms)
{
    for(unsigned int i = 0; i < ms; i++)
    {
        test_sleep_ns(ST_NS_PER_MS);
    }
}


/**
 * @brief Function to use do catch segfaults.
 * It spins so that I can attach to the process. remove before merge
 * @param signo
 */
static void segfault_catch(int signo)
{
    printf("Error Segfault\r\n");
    while(1) {}
    exit(-1);
}

#ifndef DISABLE_TIME_OUT
/**
 * @brief Timeout task that allows the unit tests to exit in the event of a lockup
 * @param arg
 */
void *timeout_worker(void *arg)
{
    unsigned int *typed;
    struct timespec time_data =
    {
        .tv_sec = 0,
        .tv_nsec = 0
    };
    typed = arg;
    assert(NULL != typed);
    time_data.tv_sec = typed[0];
    while(0 != nanosleep(&time_data, &time_data)) {}
    time_data.tv_nsec = 1000;
    time_data.tv_sec = 0;
    //If we got this far then we timed out.
    printf("Error Tests have timed out\r\n");
    while(0 != nanosleep(&time_data, &time_data)) {}
    assert(false);
    return NULL;
}
#endif //DISABLE_TIME_OUT

/*********************************************************************
 *************** Dummy Test ******************************************
 ********************************************************************/
static void just_pass(void **state)
{

}

/**
 * @brief the main function
 * @return
 */
int main(void)
{
    int rv;
#ifndef DISABLE_TIME_OUT
    pthread_t thread;
    unsigned int timeout_seconds = 30;
    if(ST_NS_PER_MS < 100000)
    {
        timeout_seconds = 5;
    }
#endif //DISABLE_TIME_OUT
    const struct CMUnitTest tests[] =
    {
        cmocka_unit_test(just_pass),

    };
    signal(SIGSEGV, segfault_catch);
#ifndef DISABLE_TIME_OUT
    assert(0 == pthread_create(&thread, NULL, timeout_worker, &timeout_seconds));
#endif //DISABLE_TIME_OUT
    rv = cmocka_run_group_tests(tests, NULL, NULL);
    assert(rv <= ARRAY_MAX_COUNT(tests));
    return rv;
}
