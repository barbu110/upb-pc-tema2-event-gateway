#pragma once

#include <cstdint>

namespace commons::server_response
{

enum StatusCode : std::uint8_t
{
  OK = 0,
  INVALID_MSG_TYPE,
  EXPECTED_GREETING,
  UNEXPECTED_GREETING,
  DUPLICATE_CLIENT_ID,
};

}
