#pragma once

#include <cstdint>
#include <string>

namespace commons::server_response
{

enum StatusCode : std::uint8_t
{
  OK = 0,
  INVALID_MSG_TYPE,
  EXPECTED_GREETING,
  UNEXPECTED_GREETING,
  DUPLICATE_CLIENT_ID,
  SUBSCRIBE_SUCCESSFUL,
  UNSUBSCRIBE_SUCCESSFUL,
  DUPLICATE_SUBSCRIPTION,
};

static std::string status_str(StatusCode c)
{
  switch (c)
  {
  case OK:
    return "OK.";
  case INVALID_MSG_TYPE:
    return "Invalid message type.";
  case EXPECTED_GREETING:
    return "Expected greeting.";
  case UNEXPECTED_GREETING:
    return "Unexpected greeting.";
  case DUPLICATE_CLIENT_ID:
    return "Duplicate client ID.";
  case SUBSCRIBE_SUCCESSFUL:
    return "Successfully subscribed.";
  case UNSUBSCRIBE_SUCCESSFUL:
    return "Successfully unsubscribed.";
  case DUPLICATE_SUBSCRIPTION:
    return "Already subscribed.";
  default:
    __builtin_unreachable();
  }
}

}  // namespace commons::server_response
