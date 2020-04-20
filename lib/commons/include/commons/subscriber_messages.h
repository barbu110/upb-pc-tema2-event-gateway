#pragma once

#include "commons/server_response.h"
#include "device_messages.h"
#include "microloop/buffer.h"

#include <cstdint>
#include <string>
#include <variant>

namespace commons::subscriber_messages
{

enum MessageType : std::uint8_t
{
  GREETING,
  SUBSCRIBE,
  UNSUBSCRIBE,
  RESPONSE,
  DEVICE_MSG,
  _COUNT,  // End of valid messages from client.
};

/**
 * \brief Message to be retrieved from subscriber clients upon connection initiation. This message
 * is similar to a handshake, including client identification data.
 */
struct GreetingMessage
{
  /* Client ID string. No more than 10 characters. */
  std::string client_id;

  /* Create a buffer from this message to be sent over the network. */
  microloop::Buffer serialize() const;
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

  /* Create a buffer from this message to be sent over the network. */
  microloop::Buffer serialize() const;
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

  /* Create a buffer from this message to be sent over the network. */
  microloop::Buffer serialize() const;
};

/**
 * \brief Response to be sent to subscribers regarding their requests.
 */
struct ServerResponse
{
  /* The status code of this response. */
  server_response::StatusCode code;

  /* Optional notes of this response. Will be truncated/extended to exactly 64 bytes. */
  std::string notes;

  /* Serialize this response into a buffer ready to be sent over the network. */
  microloop::Buffer serialize() const;
};

/**
 * \brief Message containing a serialized version of another message coming from the UDP endpoint
 * (i.e. Device Endpoint).
 */
struct DeviceNotification
{
  /* The device address as obtained from AddressWrapper::str() */
  std::string device_address;

  /* A copy of the original message sent by the device. */
  device_messages::GenericDeviceMessage original_message;

  microloop::Buffer serialize() const;
};

/* Message types supported from subscriber clients. */
using SubscriberMessage = std::variant<GreetingMessage,
    SubscribeRequest,
    UnsubscribeRequest,
    ServerResponse,
    DeviceNotification>;

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
