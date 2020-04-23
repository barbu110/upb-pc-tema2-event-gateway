#include "commons/subscriber_messages.h"

#include "commons/device_messages.h"
#include "messages_internal.h"
#include "net_utils/receive_from.h"

#include <algorithm>
#include <cstring>

namespace commons::subscriber_messages
{

bool is_valid_message_type(uint8_t value)
{
  return value < MessageType::_COUNT;
}

bool can_parse_entire_msg(const microloop::Buffer &buf)
{
  using internal::MsgHdr;

  if (buf.size() < sizeof(MsgHdr))
  {
    return false;
  }

  auto hdr = (MsgHdr *)buf.data();
  if (buf.size() - sizeof(MsgHdr) < ntohs(hdr->msg_size))
  {
    return false;
  }

  return true;
}

std::pair<SubscriberMessage, std::size_t> from_buffer(const microloop::Buffer &buf)
{
  using internal::MsgHdr;
  using internal::POD_DeviceNotification_Hdr;
  using internal::POD_GreetingMessage;
  using internal::POD_ServerResponse;
  using internal::POD_SubscribeRequest;
  using internal::POD_UnsubscribeRequest;

  auto data = static_cast<const std::uint8_t *>(buf.data());
  auto hdr = (MsgHdr *)data;
  auto msg = data + sizeof(MsgHdr);

  auto consumed = sizeof(MsgHdr) + ntohs(hdr->msg_size);

  switch (hdr->type)
  {
  case MessageType::GREETING: {
    auto pod = (const POD_GreetingMessage *)msg;

    char client_id[sizeof(pod->client_id) + 1]{};
    memcpy(client_id, pod->client_id, sizeof(pod->client_id));

    return {GreetingMessage{std::string{client_id}}, consumed};
  }
  case MessageType::SUBSCRIBE: {
    auto pod = (const POD_SubscribeRequest *)msg;
    return {SubscribeRequest{std::string{pod->topic}, pod->store_forward}, consumed};
  }
  case MessageType::UNSUBSCRIBE: {
    auto pod = (const POD_UnsubscribeRequest *)msg;
    return {UnsubscribeRequest{std::string{pod->topic}}, consumed};
  }
  case MessageType::RESPONSE: {
    using commons::server_response::StatusCode;

    auto pod = (const POD_ServerResponse *)msg;
    return {ServerResponse{static_cast<StatusCode>(pod->code), std::string{pod->notes}}, consumed};
  }
  case MessageType::DEVICE_MSG: {
    auto notif_hdr = (const POD_DeviceNotification_Hdr *)msg;
    auto notif_msg = msg + sizeof(POD_DeviceNotification_Hdr);
    auto notif_len = ntohs(hdr->msg_size) - sizeof(POD_DeviceNotification_Hdr);
    auto device_msg = device_messages::from_buffer(notif_msg, notif_len);
    return {DeviceNotification{std::string{notif_hdr->device_address}, device_msg}, consumed};
  }
  default:
    __builtin_unreachable();

    /* Undefined behavior. The caller must perform the checks. */
  }
}

microloop::Buffer GreetingMessage::serialize() const
{
  using internal::client_id_maxlen;
  using internal::MsgHdr;
  using internal::POD_GreetingMessage;

  microloop::Buffer buf{sizeof(MsgHdr) + sizeof(POD_GreetingMessage)};
  std::uint8_t *data = static_cast<std::uint8_t *>(buf.data());

  auto hdr = (MsgHdr *)data;
  auto payload = (POD_GreetingMessage *)(data + sizeof(MsgHdr));

  hdr->type = MessageType::GREETING;
  hdr->msg_size = htons(sizeof(POD_GreetingMessage));

  memcpy(payload->client_id, client_id.c_str(), std::min(client_id_maxlen(), client_id.size()));

  return buf;
}

microloop::Buffer SubscribeRequest::serialize() const
{
  using commons::internal::topic_maxlen;
  using internal::client_id_maxlen;
  using internal::MsgHdr;
  using internal::POD_SubscribeRequest;

  microloop::Buffer buf{sizeof(MsgHdr) + sizeof(POD_SubscribeRequest)};
  std::uint8_t *data = static_cast<std::uint8_t *>(buf.data());

  auto hdr = (MsgHdr *)data;
  auto payload = (POD_SubscribeRequest *)(data + sizeof(MsgHdr));

  hdr->type = MessageType::SUBSCRIBE;
  hdr->msg_size = htons(sizeof(POD_SubscribeRequest));

  memcpy(payload->topic, topic.c_str(), std::min(sizeof(payload->topic), topic.size()));
  payload->store_forward = store_forward;

  return buf;
}

microloop::Buffer UnsubscribeRequest::serialize() const
{
  using commons::internal::topic_maxlen;
  using internal::client_id_maxlen;
  using internal::MsgHdr;
  using internal::POD_UnsubscribeRequest;

  microloop::Buffer buf{sizeof(MessageType) + sizeof(POD_UnsubscribeRequest)};
  std::uint8_t *data = static_cast<std::uint8_t *>(buf.data());

  auto hdr = (MsgHdr *)data;
  auto payload = (POD_UnsubscribeRequest *)(data + sizeof(MsgHdr));

  hdr->type = MessageType::UNSUBSCRIBE;
  hdr->msg_size = htons(sizeof(POD_UnsubscribeRequest));

  memcpy(payload->topic, topic.c_str(), std::min(sizeof(payload->topic), topic.size()));

  return buf;
}

microloop::Buffer ServerResponse::serialize() const
{
  using commons::subscriber_messages::internal::notes_maxlen;
  using internal::MsgHdr;
  using internal::POD_ServerResponse;

  microloop::Buffer buf{sizeof(MsgHdr) + sizeof(POD_ServerResponse)};
  std::uint8_t *data = static_cast<std::uint8_t *>(buf.data());

  auto hdr = (MsgHdr *)data;
  auto payload = (POD_ServerResponse *)(data + sizeof(MsgHdr));

  hdr->type = MessageType::RESPONSE;
  hdr->msg_size = htons(sizeof(POD_ServerResponse));

  payload->code = code;
  std::memcpy(payload->notes, notes.c_str(), std::min(notes.size(), notes_maxlen()));

  return buf;
}

microloop::Buffer DeviceNotification::serialize() const
{
  using internal::MsgHdr;
  using internal::POD_DeviceNotification_Hdr;

  microloop::Buffer raw_dev_msg;
  std::visit([&](auto &&arg) { raw_dev_msg = arg.serialize(); }, original_message);

  microloop::Buffer buf{sizeof(MsgHdr) + sizeof(POD_DeviceNotification_Hdr) + raw_dev_msg.size()};
  std::uint8_t *data = static_cast<std::uint8_t *>(buf.data());

  auto hdr = (MsgHdr *)data;
  auto notif_hdr = (POD_DeviceNotification_Hdr *)(data + sizeof(MsgHdr));
  auto notif_payload = data + sizeof(MsgHdr) + sizeof(POD_DeviceNotification_Hdr);

  hdr->type = MessageType::DEVICE_MSG;
  hdr->msg_size = htons(sizeof(POD_DeviceNotification_Hdr) + raw_dev_msg.size());

  std::memcpy(notif_hdr->device_address, device_address.c_str(),
      std::min(net_utils::AddressWrapper::str_maxlen, device_address.size()));
  std::memcpy(notif_payload, raw_dev_msg.data(), raw_dev_msg.size());

  return buf;
}

}  // namespace commons::subscriber_messages
