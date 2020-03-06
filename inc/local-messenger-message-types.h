/**
 * @file local-messenger-message-types.h
 * @author Kade Cox
 * @date Created: Mar 6, 2020
 * @details
 * Module that helps manage the message types used by the messenger
 */

#ifndef INC_LOCAL_MESSENGER_MESSAGE_TYPES_H_
#define INC_LOCAL_MESSENGER_MESSAGE_TYPES_H_

#ifndef LOCAL_MESSENGER_MAX_MESSAGE_SIZE
#define LOCAL_MESSENGER_MAX_MESSAGE_SIZE 100
#endif  //LOCAL_MESSENGER_MAX_MESSAGE_SIZE

enum local_messenger_message_type_e
{
    LOCAL_MESSAGE_TYPE_INTERNAL_ACTION,
    LOCAL_MESSAGE_TYPE_USR
}; //!< Enum for local message type

enum local_messenger_message_internal_action_type_e
{
    LOCAL_MESSENGER_ACTION_NONE //!< There is no action to perform
}; //!< Enum for internal message actions

struct local_messenger_user_message_s
{
    long message_size;
    char message_data[LOCAL_MESSENGER_MAX_MESSAGE_SIZE];
}; //!< Structure for user messages

struct local_messanger_internal_message_s
{
    enum local_messenger_message_type_e type; //!< The current message type
    union
    {
        struct local_messenger_user_message_s user; //!< The User data
        enum local_messenger_message_internal_action_type_e action; //!< Internal Action
    } data;
}; //!< Structure used internally for the message manager

/**
 * @brief Function that builds an action message
 * @param action
 * @return The message to send out
 */
struct local_messanger_internal_message_s local_messenger_build_action(enum local_messenger_message_internal_action_type_e action);

/**
 * @brief Function that builds an user message
 * @param data
 * @param message_size
 * @return The message to send out
 */
struct local_messanger_internal_message_s local_messenger_build_user_msg(void *data, long message_size);

#endif /* INC_LOCAL_MESSENGER_MESSAGE_TYPES_H_ */
