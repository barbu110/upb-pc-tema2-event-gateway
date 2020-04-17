#include "gateway/input_endpoint.h"

namespace gateway::endpoint
{

void InputEndpoint::on_data(const net_utils::AddressWrapper &source, const microloop::Buffer &buf)
{
  auto msg = commons::device_messages::from_buffer(buf);

  subscriber_(source, msg);
}

}  // namespace gateway::endpoint
