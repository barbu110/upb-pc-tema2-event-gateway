#include "commons/device_messages.h"

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
    int64_t value = sign ? -1 * static_cast<int64_t>(abs_val) : abs_val;
    return DeviceMessage<INT>{topic, value};
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

}  // namespace commons::device_messages
