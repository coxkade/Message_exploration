/**
 * @file time-out-helper.h
 * @author Kade Cox
 * @date Created: Mar 6, 2020
 * @details
 *
 */

#ifndef INC_TIME_OUT_HELPER_H_
#define INC_TIME_OUT_HELPER_H_

#include <sys/time.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct time_out_helper_data_s
{
    struct timeval init_time;
    uint64_t timeout_ms;
} time_out_helper_data_s;

/**
 * @brief Function that initializes a timeout helper instance
 * @param data the instance data to initialize
 * @param timeout_ms timeout occurs after this many milliseconds
 */
void time_out_helper_init(time_out_helper_data_s *data, uint64_t timeout_ms);

/**
 * @brief Function that checks if the timeout time has elapsed
 * @param data the instance data
 * @return true if the time has elapsed, false otherwise
 */
bool time_out_helper_check(time_out_helper_data_s *data);

#endif /* INC_TIME_OUT_HELPER_H_ */
