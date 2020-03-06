/**
 * @file Main_Test.c
 * @author Kade Cox
 * @date Created: Mar 6, 2020
 * @details
 *
 */

#include <local-messenger.h>
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

/*********************************************************************
 *************** General Utility Functions ***************************
 ********************************************************************/

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

/*********************************************************************
 *************** Basic Test ******************************************
 ********************************************************************/
#define BASIC_TEST_BUFFER_SIZE 50
#define BASIC_TEST_FINISH_VAL 0xFE
static char basic_test_send_buffer[BASIC_TEST_BUFFER_SIZE]; //Bubber used with the basic test

static void basic_test_message_callback(void *msg, long message_size)
{
    char set_val;
    char *typed;
    assert(NULL != msg);
    assert(ARRAY_MAX_COUNT(basic_test_send_buffer) == message_size);
    typed = msg;
    set_val = typed[0];
    set_val++;
    if(set_val <= BASIC_TEST_FINISH_VAL)
    {
        memset(basic_test_send_buffer, set_val, ARRAY_MAX_COUNT(basic_test_send_buffer));
        messenger_send(basic_test_send_buffer, ARRAY_MAX_COUNT(basic_test_send_buffer));
    }
}

static void basic_test(void **state)
{
    static char finish_message[BASIC_TEST_BUFFER_SIZE];
    memset(finish_message, BASIC_TEST_FINISH_VAL, ARRAY_MAX_COUNT(finish_message));
    memset(basic_test_send_buffer, 0x00, ARRAY_MAX_COUNT(basic_test_send_buffer));
    messenger_register_callback(basic_test_message_callback);
    messenger_send(basic_test_send_buffer, ARRAY_MAX_COUNT(basic_test_send_buffer));
    while(0 != memcmp(basic_test_send_buffer, finish_message, ARRAY_MAX_COUNT(finish_message))) {}
    messenger_kill();
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
        cmocka_unit_test(basic_test),
    };
    signal(SIGSEGV, segfault_catch);
#ifndef DISABLE_TIME_OUT
    assert(0 == pthread_create(&thread, NULL, timeout_worker, &timeout_seconds));
#endif //DISABLE_TIME_OUT
    rv = cmocka_run_group_tests(tests, NULL, NULL);
    assert(rv <= ARRAY_MAX_COUNT(tests));
    return rv;
}
