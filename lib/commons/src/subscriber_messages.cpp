#include "commons/subscriber_messages.h"

#include "messages_internal.h"

#include <cstring>
#include <algorithm>

namespace commons::subscriber_messages
{

bool is_valid_message_type(uint8_t value)
{
  using internal::MessageType;
  return value < MessageType::_COUNT;
}

SubscriberMessage from_buffer(const microloop::Buffer &buf)
{
  using internal::MessageType;
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

}  // namespace subscriber::messages
