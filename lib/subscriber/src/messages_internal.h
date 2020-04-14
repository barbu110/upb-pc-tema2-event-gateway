#pragma once

namespace subscriber::messages::internal
{

static constexpr std::size_t client_id_maxlen()
{
  return 10;
}

static constexpr std::size_t topic_maxlen()
{
  return 50;
}

static constexpr std::size_t msg_payload_size()
{
  return 1500;
}

enum MessageType : std::uint8_t
{
  GREETING,
  SUBSCRIBE,
  UNSUBSCRIBE,
  _COUNT,  // Must always be the last enumerator
};

struct POD_GreetingMessage
{
  char client_id[client_id_maxlen()];
};

struct POD_SubscribeRequest
{
  char topic[topic_maxlen()];
  bool store_forward : 8;
} __attribute__((__packed__));

struct POD_UnsubscribeRequest
{
  char topic[topic_maxlen()];
};

}  // namespace subscriber::messages::internal
