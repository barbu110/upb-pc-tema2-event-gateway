#pragma once

#include "microloop/buffer.h"

#include <cstdint>
#include <string>
#include <variant>

namespace subscriber::messages
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

struct SubscribeRequest
{
  /* The topic to subscribe the client to. */
  std::string topic;

  /* Whether to enable Store and Forward mechanism for this subscription. */
  bool store_forward;
};

struct UnsubscribeRequest
{
  /* The topic to unsubscribe the client from. */
  std::string topic;
};

using SubscriberMessage = std::variant<GreetingMessage, SubscribeRequest, UnsubscribeRequest>;

bool is_valid_message_type(uint8_t value);

SubscriberMessage from_buffer(const microloop::Buffer &buf);

}  // namespace subscriber::messages
