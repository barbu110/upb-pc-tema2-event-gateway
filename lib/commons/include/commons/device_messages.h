#pragma once

#include "microloop/buffer.h"

#include <cmath>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <limits>
#include <sstream>
#include <string>
#include <variant>

namespace commons::device_messages
{

enum PayloadType
{
  INT,
  SHORT_REAL,
  FLOAT,
  STRING,
};


template <PayloadType ValueType>
struct DeviceMessage;

template <>
struct DeviceMessage<PayloadType::INT>
{
  std::string topic;

  /* Sign byte for the value. 0 is positive, 1 is negative. */
  std::uint8_t sign;

  /* The absolute integral value stored in this message. */
  std::uint32_t value;

  std::string value_repr() const
  {
    static constexpr std::size_t max_digits = std::numeric_limits<std::uint32_t>::digits10;
    char buf[max_digits + 1 + 1]{};

    std::snprintf(buf, sizeof(buf), "%s%ld", sign ? "-" : "", value);
    return buf;
  }

  std::string str() const
  {
    std::stringstream ss;
    ss << topic << " - INT - " << value_repr();
    return ss.str();
  }

  microloop::Buffer serialize() const;
};

template <>
struct DeviceMessage<PayloadType::SHORT_REAL>
{
  std::string topic;

  /* The value stored in this message multiplied by 100. */
  std::uint16_t value;

  std::string value_repr() const
  {
    std::stringstream ss;

    ss << (value / 100);

    if (auto frac = value % 100; frac != 0)
    {
      ss << ".";

      auto f = ss.fill('0');
      auto w = ss.width(2);
      ss << frac;

      ss.fill(f);
      ss.width(w);
    }

    return ss.str();
  }

  std::string str() const
  {
    std::stringstream ss;
    ss << topic << " - SHORT_REAL - " << value_repr();

    return ss.str();
  }

  microloop::Buffer serialize() const;
};

template <>
struct DeviceMessage<PayloadType::FLOAT>
{
  std::string topic;
  std::uint8_t sign;
  std::uint8_t float_size;
  std::uint32_t abs_val;

  std::string value_repr() const
  {
    std::stringstream ss;

    /* Take care of the sign */
    ss << (sign ? "-" : "");

    /* Write the integral part */
    std::uint32_t digits10 = std::pow(10, float_size);
    ss << (abs_val / digits10);

    if (auto frac = abs_val % digits10; frac != 0)
    {
      ss << ".";
      ss.fill('0');
      ss.width(float_size);
      ss << frac;
    }

    return ss.str();
  }

  std::string str() const
  {
    std::stringstream ss;
    ss << topic << " - FLOAT - " << value_repr();

    return ss.str();
  }

  microloop::Buffer serialize() const;
};

template <>
struct DeviceMessage<PayloadType::STRING>
{
  std::string topic;
  std::string value;

  std::string value_repr() const
  {
    return value;
  }

  std::string str() const
  {
    std::stringstream ss;
    ss << topic << " - STRING - " << value_repr();
    return ss.str();
  }

  microloop::Buffer serialize() const;
};

using GenericDeviceMessage = std::variant<DeviceMessage<PayloadType::INT>,
    DeviceMessage<PayloadType::SHORT_REAL>,
    DeviceMessage<PayloadType::FLOAT>,
    DeviceMessage<PayloadType::STRING>>;

GenericDeviceMessage from_buffer(const microloop::Buffer &buf);
GenericDeviceMessage from_buffer(const void *data, std::size_t n);

}  // namespace commons::device_messages
