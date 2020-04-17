#pragma once

#include "commons/device_messages.h"
#include "net_utils/udp_server.h"

#include <cstdint>
#include <functional>
#include <list>
#include <utility>

namespace gateway::endpoint
{

class InputEndpoint
{
private:
  using MessageCallback = std::function<void(const net_utils::AddressWrapper &,
      const commons::device_messages::GenericDeviceMessage &)>;

public:
  InputEndpoint(std::uint16_t port) : server_{port}
  {
    server_.set_data_callback(&InputEndpoint::on_data, this);
  }

  void on_data(const net_utils::AddressWrapper &source, const microloop::Buffer &buf);

  template <class Func, class... Args>
  void subscribe(Func &&func, Args &&... args)
  {
    using namespace std::placeholders;
    auto bound = std::bind(std::forward<Func>(func), std::forward<Args>(args)..., _1, _2);
    subscriber_ = std::move(bound);
  }

private:
  net_utils::UdpServer server_;
  MessageCallback subscriber_;
};

}  // namespace gateway::endpoint
