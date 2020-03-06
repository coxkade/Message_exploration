/**
 * @file time-out-helper.c
 * @author Kade Cox
 * @date Created: Mar 6, 2020
 * @details
 *
 */

#include <time-out-helper.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

/***********************************************************************************/
/***************************** Defines and Macros **********************************/
/***********************************************************************************/

//Macro that gets the number of elements supported by the array
#define ARRAY_MAX_COUNT(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

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
 * @brief Function that initializes a timeout helper instance
 * @param data the instance data to initialize
 * @param timeout_ms timeout occurs after this many milliseconds
 */
void time_out_helper_init(time_out_helper_data_s *data, uint64_t timeout_ms)
{
    assert(NULL != data);
    assert(0 == gettimeofday(&data->init_time, NULL));
    data->timeout_ms = timeout_ms;
}

/**
 * @brief Function that checks if the timeout time has elapsed
 * @param data the instance data
 * @return true if the time has elapsed, false otherwise
 */
bool time_out_helper_check(time_out_helper_data_s *data)
{
    int result;
    static const uint64_t ms_per_sec = 1000;
    static const uint64_t us_per_ms = 1000;
    struct timeval current;
    struct timeval diff;
    uint64_t elapsed_ms;
    uint64_t typed_s;
    uint64_t typed_us;
    assert(NULL != data);
    //Get the Current Time
    assert(0 == gettimeofday(&current, NULL));
    result = timercmp(&current, &data->init_time, >=);
    assert(0 != result); //Check to make sure the current time is larger than the start time
    timersub(&current, &data->init_time, &diff);
    //Now convert to ms
    typed_s = diff.tv_sec;
    typed_us = diff.tv_usec;
    elapsed_ms = (ms_per_sec * typed_s) + (typed_us / us_per_ms);
    if(elapsed_ms >= data->timeout_ms)
    {
        return true;
    }
    return false;
}
