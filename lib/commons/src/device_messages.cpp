#include "commons/device_messages.h"

#include "microloop/buffer.h"
#include "messages_internal.h"

#include <algorithm>
#include <arpa/inet.h>
#include <iterator>

namespace commons::device_messages
{

GenericDeviceMessage from_buffer(const microloop::Buffer &buf)
{
  auto data = static_cast<const char *>(buf.data());
  auto data_it = data;

  using commons::internal::topic_maxlen;
  std::string topic{data_it, data_it + topic_maxlen()};
  data_it += topic_maxlen();

  PayloadType type = static_cast<PayloadType>(*data_it++);

  switch (type)
  {
  case INT: {
    std::uint8_t sign = *data_it++;
    std::uint32_t abs_val = ntohl(*((std::uint32_t *)data_it));
    return DeviceMessage<INT>{topic, sign, abs_val};
  }
  case SHORT_REAL: {
    std::uint16_t value = ntohs(*((std::uint16_t *)data_it));
    return DeviceMessage<SHORT_REAL>{topic, value};
  }
  case FLOAT: {
    std::uint8_t sign = *data_it++;
    std::uint32_t abs_val = ntohl(*((std::uint32_t *)data_it));
    data_it += sizeof(abs_val);
    std::uint8_t float_size = *data_it;

    return DeviceMessage<FLOAT>{topic, sign, float_size, abs_val};
  }
  case STRING: {
    return DeviceMessage<STRING>{topic, std::string{data_it, data + buf.size()}};
  }
  default:
    __builtin_unreachable();
  }
}

microloop::Buffer DeviceMessage<PayloadType::INT>::serialize() const
{
  using commons::device_messages::internal::POD_DeviceMessage_Header;
  using commons::device_messages::internal::POD_DeviceMessage_Int;
  using commons::internal::topic_maxlen;

  microloop::Buffer buf{sizeof(POD_DeviceMessage_Header) + sizeof(POD_DeviceMessage_Int)};
  auto data = static_cast<std::uint8_t *>(buf.data());

  POD_DeviceMessage_Header *hdr = reinterpret_cast<POD_DeviceMessage_Header *>(data);
  std::memcpy(hdr->topic, topic.c_str(), std::min(topic.size(), topic_maxlen()));
  hdr->payload_type = PayloadType::INT;

  auto payload = reinterpret_cast<POD_DeviceMessage_Int *>(data + sizeof(*hdr));
  payload->sign = sign;
  payload->value = htonl(value);

  return buf;
}

microloop::Buffer DeviceMessage<PayloadType::SHORT_REAL>::serialize() const
{
  using commons::device_messages::internal::POD_DeviceMessage_Header;
  using commons::device_messages::internal::POD_DeviceMessage_ShortReal;
  using commons::internal::topic_maxlen;

  microloop::Buffer buf{sizeof(POD_DeviceMessage_Header) + sizeof(POD_DeviceMessage_ShortReal)};
  auto data = static_cast<std::uint8_t *>(buf.data());

  POD_DeviceMessage_Header *hdr = reinterpret_cast<POD_DeviceMessage_Header *>(data);
  std::memcpy(hdr->topic, topic.c_str(), std::min(topic.size(), topic_maxlen()));
  hdr->payload_type = PayloadType::SHORT_REAL;

  auto payload = reinterpret_cast<POD_DeviceMessage_ShortReal *>(data + sizeof(*hdr));
  payload->value = htons(value);

  return buf;
}

microloop::Buffer DeviceMessage<PayloadType::FLOAT>::serialize() const
{
  using commons::device_messages::internal::POD_DeviceMessage_Header;
  using commons::device_messages::internal::POD_DeviceMessage_Float;
  using commons::internal::topic_maxlen;

  microloop::Buffer buf{sizeof(POD_DeviceMessage_Header) + sizeof(POD_DeviceMessage_Float)};
  auto data = static_cast<std::uint8_t *>(buf.data());

  POD_DeviceMessage_Header *hdr = reinterpret_cast<POD_DeviceMessage_Header *>(data);
  std::memcpy(hdr->topic, topic.c_str(), std::min(topic.size(), topic_maxlen()));
  hdr->payload_type = PayloadType::FLOAT;

  auto payload = reinterpret_cast<POD_DeviceMessage_Float *>(data + sizeof(*hdr));
  payload->sign = sign;
  payload->abs_val = htonl(abs_val);
  payload->float_size = float_size;

  return buf;
}

microloop::Buffer DeviceMessage<PayloadType::STRING>::serialize() const
{
  using commons::device_messages::internal::POD_DeviceMessage_Header;
  using commons::internal::topic_maxlen;

  auto payload_size = std::min<std::size_t>(value.size(), 1500);
  microloop::Buffer buf{sizeof(POD_DeviceMessage_Header) + payload_size};
  auto data = static_cast<std::uint8_t *>(buf.data());

  POD_DeviceMessage_Header *hdr = reinterpret_cast<POD_DeviceMessage_Header *>(data);
  std::memcpy(hdr->topic, topic.c_str(), std::min(topic.size(), topic_maxlen()));
  hdr->payload_type = PayloadType::STRING;

  std::memcpy(data + sizeof(*hdr), value.c_str(), payload_size);

  return buf;
}

}  // namespace commons::device_messages
