#include "commons/subscriber_messages.h"

#include "messages_internal.h"

#include <algorithm>
#include <cstring>

namespace commons::subscriber_messages
{

bool is_valid_message_type(uint8_t value)
{
  return value < MessageType::_COUNT;
}

SubscriberMessage from_buffer(const microloop::Buffer &buf)
{
  using internal::POD_GreetingMessage;
  using internal::POD_SubscribeRequest;
  using internal::POD_UnsubscribeRequest;

  auto payload = static_cast<const std::uint8_t *>(buf.data());

  switch (*reinterpret_cast<const MessageType *>(buf.data()))
  {
  case MessageType::GREETING: {
    const POD_GreetingMessage *pod = reinterpret_cast<const POD_GreetingMessage *>(payload + 1);

    char client_id[sizeof(pod->client_id) + 1]{};
    memcpy(client_id, pod->client_id, std::min<size_t>(buf.size() - 1, sizeof(client_id)));

    return GreetingMessage{std::string{client_id}};
  }
  case MessageType::SUBSCRIBE: {
    auto pod = reinterpret_cast<const POD_SubscribeRequest *>(payload);
    return SubscribeRequest{std::string{pod->topic}, pod->store_forward};
  }
  case MessageType::UNSUBSCRIBE: {
    auto pod = reinterpret_cast<const POD_UnsubscribeRequest *>(payload);
    return UnsubscribeRequest{std::string{pod->topic}};
  }
  default:
    __builtin_unreachable();

    /* Undefined behavior. The caller must perform the checks. */
  }
}

microloop::Buffer GreetingMessage::serialize() const
{
  using internal::client_id_maxlen;
  using internal::POD_GreetingMessage;

  microloop::Buffer buf{sizeof(MessageType) + sizeof(POD_GreetingMessage)};
  std::uint8_t *data = static_cast<std::uint8_t *>(buf.data());

  data[0] = MessageType::GREETING;
  memcpy(data + 1, client_id.c_str(), std::min(client_id_maxlen(), client_id.size()));

  return buf;
}

microloop::Buffer SubscribeRequest::serialize() const
{
  using commons::internal::topic_maxlen;
  using internal::client_id_maxlen;
  using internal::POD_SubscribeRequest;

  microloop::Buffer buf{sizeof(MessageType) + sizeof(POD_SubscribeRequest)};
  std::uint8_t *data = static_cast<std::uint8_t *>(buf.data());

  data[0] = MessageType::SUBSCRIBE;
  memcpy(data + 1, topic.c_str(), std::min(topic_maxlen(), topic.size()));
  data[buf.size() - 1] = store_forward;

  return buf;
}

microloop::Buffer UnsubscribeRequest::serialize() const
{
  using commons::internal::topic_maxlen;
  using internal::client_id_maxlen;
  using internal::POD_UnsubscribeRequest;

  microloop::Buffer buf{sizeof(MessageType) + sizeof(POD_UnsubscribeRequest)};
  std::uint8_t *data = static_cast<std::uint8_t *>(buf.data());

  data[0] = MessageType::UNSUBSCRIBE;
  memcpy(data + 1, topic.c_str(), std::min(topic_maxlen(), topic.size()));

  return buf;
}

}  // namespace commons::subscriber_messages
