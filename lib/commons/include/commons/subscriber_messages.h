#pragma once

#include "microloop/buffer.h"

#include <cstdint>
#include <string>
#include <variant>

namespace commons::subscriber_messages
{

/**
 * \brief Message to be retrieved from subscriber clients upon connection initiation. This message
 * is similar to a handshake, including client identification data.
 */
struct GreetingMessage
{
  /* Client ID string. No more than 10 characters. */
  std::string client_id;
};

/**
 * \brief Message representing a subscribe request. Subscriber clients send this message when they
 * want to be subscribed to a specific topic.
 */
struct SubscribeRequest
{
  /* The topic to subscribe the client to. */
  std::string topic;

  /* Whether to enable Store and Forward mechanism for this subscription. */
  bool store_forward;
};

/**
 * \brief Message to be sent whenever a client wishes to unsubscribe from a certain topic.
 *
 * Note that sending this message will remove a client's subscription even if the Store and Forward
 * mechanism was enabled.
 */
struct UnsubscribeRequest
{
  /* The topic to unsubscribe the client from. */
  std::string topic;
};

/* Message types supported from subscriber clients. */
using SubscriberMessage = std::variant<GreetingMessage, SubscribeRequest, UnsubscribeRequest>;

/**
 * \brief Checks whether the supplied byte represents a valid message type.
 *
 * This function is to be used to perform checks for incoming network packets.
 */
bool is_valid_message_type(uint8_t value);

/**
 * \brief Constructs a strongly-typed message structure given a buffer from an incoming network
 * packet.
 *
 * Note that this function does not perform any checks regarding the message type supplied into the
 * buffer. Providing a buffer containing an invalid message type will lead to undefined behavior.
 *
 * Users are required to perform checks using the `is_valid_message_type` function.
 */
SubscriberMessage from_buffer(const microloop::Buffer &buf);

}  // namespace commons::subscriber_messages
