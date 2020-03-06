/**
 * @file local-messenger.c
 * @author Kade Cox
 * @date Created: Mar 6, 2020
 * @details
 *
 */

#include <time-out-helper.h>
#include <local-messenger.h>
#include <local-messenger-message-types.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <stdbool.h>
#include <pthread.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

/***********************************************************************************/
/***************************** Defines and Macros **********************************/
/***********************************************************************************/

//Macro that gets the number of elements supported by the array
#define ARRAY_MAX_COUNT(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

#ifdef DEBUG_MESSENGER
#define PRINT_MSG(...) printf(__VA_ARGS__)
#else
#define PRINT_MSG(...)
#endif //DEBUG_MESSENGER

#ifndef TIME_OUT_MS
#define TIME_OUT_MS 1000
#endif //TIME_OUT_MS

#define MODULE_MESSAGE_TYPE 1

/***********************************************************************************/
/***************************** Type Defs *******************************************/
/***********************************************************************************/

struct messenger_module_data_s
{
    bool initialized; //!< Tells if the module has been initialized
    int queue_id; //!< The msg queue id returned on creation
    pthread_t master_thread; //!< The master thread id
    bool kill_master_thread; //!< Flag used to kill the master thread
    pthread_mutex_t init_mutex; //!< The mutex to protect the module initialization
    messenger_on_messaage_rcv cb; //!< The callback to call when a user message is received
};

struct module_message_transaction_data_s
{
    long mtype; //The message data
    char mdata[sizeof(struct local_messanger_internal_message_s)]; //The message data
}; //!< Structure used in the message transactions

/***********************************************************************************/
/***************************** Function Declarations *******************************/
/***********************************************************************************/

/**
 * @brief internal function for receiving a message
 * @param msg pointer to the object to place the message into
 * @return true if a message was received
 */
static bool internal_message_receive(struct local_messanger_internal_message_s *msg);


/**
 * Internal function for sending a message
 * @param msg
 */
static void internal_message_send(struct local_messanger_internal_message_s *msg);

/***********************************************************************************/
/***************************** Static Variables ************************************/
/***********************************************************************************/

static struct messenger_module_data_s messenger_module_data =
{
    .initialized = false,
    .init_mutex = PTHREAD_MUTEX_INITIALIZER
};

/***********************************************************************************/
/***************************** Function Definitions ********************************/
/***********************************************************************************/

/**
 * @brief The Central messenger task
 * @param args
 */
static void *central_messenger(void *args)
{
    messenger_on_messaage_rcv callback;
    struct local_messanger_internal_message_s c_message;
    messenger_module_data.kill_master_thread = false;
    while(false == messenger_module_data.kill_master_thread)
    {
        PRINT_MSG("%s waiting on message\r\n", __FUNCTION__);
        if(true == internal_message_receive(&c_message))
        {
            callback = messenger_module_data.cb;
            PRINT_MSG("%s received message\r\n", __FUNCTION__);
            switch(c_message.type)
            {
                case LOCAL_MESSAGE_TYPE_INTERNAL_ACTION:
                    PRINT_MSG("%s received message of type LOCAL_MESSAGE_TYPE_INTERNAL_ACTION\r\n", __FUNCTION__);
                    break;
                case LOCAL_MESSAGE_TYPE_USR:
                    PRINT_MSG("%s received message of type LOCAL_MESSAGE_TYPE_USR\r\n", __FUNCTION__);
                    if(NULL != callback)
                    {
                        callback(c_message.data.user.message_data, c_message.data.user.message_size);
                    }
                    break;
                default:
                    assert(false);
                    break;
            }
        }
    }
    return NULL;
}

/**
 * Initialize the ipc message queue
 */
static inline void init_msg_queue(void)
{
    int error_number;
    int result;
    result = msgget(IPC_PRIVATE, IPC_CREAT | IPC_EXCL | 0666);
    if(0  > result)
    {
        error_number = errno;
        switch(error_number)
        {
            case EACCES:
                PRINT_MSG("-----msgget returned EACCES\r\n");
                break;
            case EEXIST:
                PRINT_MSG("-----msgget returned EEXIST\r\n");
                break;
            case ENOENT:
                PRINT_MSG("-----msgget returned ENOENT\r\n");
                break;
            case ENOMEM:
                PRINT_MSG("-----msgget returned ENOMEM\r\n");
                break;
            case ENOSPC:
                PRINT_MSG("-----msgget returned ENOSPC\r\n");
                break;
            default:
                PRINT_MSG("Error Unknown error\r\n");
                assert(false);
        }
    }
    messenger_module_data.queue_id = result;
}

/**
 * @brief Function that initializes the messaging module if needed
 */
static void init_if_needed(void)
{
    if(false == messenger_module_data.initialized)
    {
        assert(0 == pthread_mutex_lock(&messenger_module_data.init_mutex));
        if(false == messenger_module_data.initialized)
        {
            PRINT_MSG("%s initializing module\r\n", __FUNCTION__);
            init_msg_queue();
            messenger_module_data.kill_master_thread = true;
            assert(0 == pthread_create(&messenger_module_data.master_thread, NULL, central_messenger, NULL));
            while(true == messenger_module_data.kill_master_thread) {}
            assert(false == messenger_module_data.kill_master_thread);
            messenger_module_data.cb = NULL;
            messenger_module_data.initialized = true;
        }
        pthread_mutex_unlock(&messenger_module_data.init_mutex);
    }
}


/**
 * @brief internal function for receiving a message
 * @param msg pointer to the object to place the message into
 * @return true if a message was received
 */
static bool internal_message_receive(struct local_messanger_internal_message_s *msg)
{
    int result = -1;
    time_out_helper_data_s time_data;
    struct module_message_transaction_data_s data;
    assert(NULL != msg);
    time_out_helper_init(&time_data, TIME_OUT_MS);
    while(false == time_out_helper_check(&time_data) && result < 0)
    {
        result = msgrcv(messenger_module_data.queue_id, &data, sizeof(struct local_messanger_internal_message_s), MODULE_MESSAGE_TYPE, IPC_NOWAIT);
    }
    if(0 > result)
    {
        return false;
    }
    assert(result == sizeof(struct local_messanger_internal_message_s));
    memcpy(msg, data.mdata, sizeof(struct local_messanger_internal_message_s));
    return true;
}

/**
 * @brief Function that prints an internal message number
 * @param function
 * @param errornum
 */
static inline void internal_print_error_number(const char *function, int errornum)
{
    PRINT_MSG("**** %s had error %i: %s\r\n", function, errornum, strerror(errornum));
}

/**
 * Internal function for sending a message
 * @param msg
 */
static void internal_message_send(struct local_messanger_internal_message_s *msg)
{
    int result = -1;
    time_out_helper_data_s time_data;
    struct module_message_transaction_data_s data;
    data.mtype = MODULE_MESSAGE_TYPE;
    memcpy(data.mdata, msg, sizeof(struct local_messanger_internal_message_s));
    time_out_helper_init(&time_data, TIME_OUT_MS);
    while(false == time_out_helper_check(&time_data) && result < 0)
    {
        result = msgsnd(messenger_module_data.queue_id, &data, sizeof(struct local_messanger_internal_message_s), IPC_NOWAIT);
        if(0 > result)
        {
            internal_print_error_number("msgsnd", errno);
        }
    }
    PRINT_MSG("%s result: %i\r\n", __FUNCTION__, result);
    assert(0 <= result);
}

/**
 * @brief register a callback to call when a message is received
 * @param cb The callback in question
 */
void messenger_register_callback(messenger_on_messaage_rcv cb)
{
    init_if_needed();
    assert(NULL != cb);
    assert(NULL == messenger_module_data.cb); //We do not support overwriting the callback
    messenger_module_data.cb = cb;
}

/**
 * Kill the messenger task and reset the module
 */
void messenger_kill(void)
{
    struct local_messanger_internal_message_s msg;
    init_if_needed();
    //Add the Kill Logic here
    messenger_module_data.kill_master_thread = true;
    msg = local_messenger_build_action(LOCAL_MESSENGER_ACTION_NONE);
    internal_message_send(&msg);
    pthread_join(messenger_module_data.master_thread, NULL);
    messenger_module_data.initialized = false;
}

/**
 * @brief send a message over the sender
 * @param message ptr to the message data to send. It will be copied
 * @param message_size The size of the message to send.
 */
void messenger_send(void *message, long message_size)
{
    struct local_messanger_internal_message_s msg;
    assert(NULL != message);
    assert(0 < message_size);
    init_if_needed();
    msg = local_messenger_build_user_msg(message, message_size);
    internal_message_send(&msg);
}

