/**
 * @file local-messenger-message-types.c
 * @author Kade Cox
 * @date Created: Mar 6, 2020
 * @details
 *
 */

#include <local-messenger-message-types.h>
#include <string.h>
#include <assert.h>

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
 * @brief Function that builds an action message
 * @param action
 * @return The message to send out
 */
struct local_messanger_internal_message_s local_messenger_build_action(enum local_messenger_message_internal_action_type_e action)
{
    struct local_messanger_internal_message_s rv;
    rv.type = LOCAL_MESSAGE_TYPE_INTERNAL_ACTION;
    rv.data.action = action;
    return rv;
}


/**
 * @brief Function that builds an user message
 * @param data
 * @param message_size
 * @return The message to send out
 */
struct local_messanger_internal_message_s local_messenger_build_user_msg(void *data, long message_size)
{
    struct local_messanger_internal_message_s rv;
    rv.type = LOCAL_MESSAGE_TYPE_USR;
    assert(NULL != data);
    assert(0 < message_size);
    assert(ARRAY_MAX_COUNT(rv.data.user.message_data) >= message_size);
    memcpy(rv.data.user.message_data, data, message_size);
    rv.data.user.message_size = message_size;
    return rv;
}
