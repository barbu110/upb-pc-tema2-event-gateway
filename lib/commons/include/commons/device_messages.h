#pragma once

#include "microloop/buffer.h"

#include <cmath>
#include <cstdint>
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

  /* The integral value stored in this message. */
  std::int64_t value;

  std::string str() const
  {
    std::stringstream ss;
    ss << topic << " - INT - " << value;
    return ss.str();
  }
};

template <>
struct DeviceMessage<PayloadType::SHORT_REAL>
{
  std::string topic;

  /* The value stored in this message multiplied by 100. */
  std::uint16_t value;

  std::string str() const
  {
    std::stringstream ss;

    ss << topic << " - SHORT_REAL - ";
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
};

template <>
struct DeviceMessage<PayloadType::FLOAT>
{
  std::string topic;
  std::uint8_t sign;
  std::uint8_t float_size;
  std::uint32_t abs_val;

  std::string str() const
  {
    std::stringstream ss;
    ss << topic << " - FLOAT - ";

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
};

template <>
struct DeviceMessage<PayloadType::STRING>
{
  std::string topic;
  std::string value;

  std::string str() const
  {
    std::stringstream ss;
    ss << topic << " - STRING - " << value;
    return ss.str();
  }
};

using GenericDeviceMessage = std::variant<DeviceMessage<PayloadType::INT>,
    DeviceMessage<PayloadType::SHORT_REAL>,
    DeviceMessage<PayloadType::FLOAT>,
    DeviceMessage<PayloadType::STRING>>;

GenericDeviceMessage from_buffer(const microloop::Buffer &buf);

}  // namespace commons::device_messages
