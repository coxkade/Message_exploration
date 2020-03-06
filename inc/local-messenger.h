/**
 * @file local-messenger.h
 * @author Kade Cox
 * @date Created: Mar 6, 2020
 * @details
 *
 */

#ifndef INC_LOCAL_MESSENGER_H_
#define INC_LOCAL_MESSENGER_H_

#include <stdint.h>

typedef void (*messenger_on_messaage_rcv)(void *msg, long message_size);  //!< Typedef for callback function to call when a message is received

/**
 * @brief register a callback to call when a message is received
 * @param cb The callback in question
 */
void messenger_register_callback(messenger_on_messaage_rcv cb);

/**
 * @brief Kill the messenger task and reset the module
 */
void messenger_kill(void);

/**
 * @brief send a message over the sender
 * @param message ptr to the message data to send. It will be copied
 * @param message_size The size of the message to send.
 */
void messenger_send(void *message, long message_size);


#endif /* INC_LOCAL_MESSENGER_H_ */
