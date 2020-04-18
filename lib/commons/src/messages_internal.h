#pragma once

namespace commons::internal
{

static constexpr std::size_t topic_maxlen()
{
  return 50;
}

}  // namespace commons::internal

namespace commons::subscriber_messages::internal
{

using namespace commons::internal;

static constexpr std::size_t client_id_maxlen()
{
  return 10;
}

static constexpr std::size_t msg_payload_size()
{
  return 1500;
}

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

}  // namespace commons::subscriber_messages::internal
