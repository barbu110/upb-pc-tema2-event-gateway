#pragma once

#include "commons/device_messages.h"
#include "commons/subscriber_messages.h"
#include "gateway/input_endpoint.h"
#include "gateway/subscriber_conn.h"
#include "gateway/subscriber_endpoint.h"
#include "microloop/net/tcp_server.h"
#include "net_utils/udp_server.h"

#include <cstdint>
#include <iostream>
#include <map>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>

namespace gateway
{

/**
 * \brief Bridge mechanism between the input endpoint and the device endpoint.
 */
class Gateway
{
public:
  Gateway(int port) : input_endpoint_{port}, subscriber_endpoint_{port, subscribers_}
  {
    /* Pipe device data input into the subscriber endpoint */
    input_endpoint_.subscribe(&Gateway::on_device_input, this);
  }

  /* Event handler for device messages. */
  void on_device_input(const net_utils::AddressWrapper &,
      const commons::device_messages::GenericDeviceMessage &);

private:
  endpoint::InputEndpoint input_endpoint_;
  endpoint::SubscriberEndpoint subscriber_endpoint_;

  SubscribersStorage subscribers_;
};

}  // namespace gateway
