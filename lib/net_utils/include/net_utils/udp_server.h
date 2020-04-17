#pragma once

#include "microloop/event_loop.h"
#include "microloop/event_sources/net/receive.h"
#include "net_utils/receive_from.h"

namespace net_utils
{

class UdpServer
{
  using DataHandler = std::function<void(const AddressWrapper &, const microloop::Buffer &)>;

public:
  UdpServer(std::uint16_t port);

  template <class Func, class... Args>
  void set_data_callback(Func &&func, Args &&... args)
  {
    using namespace std::placeholders;

    auto bound = std::bind(std::forward<Func>(func), std::forward<Args>(args)..., _1, _2);
    on_data_ = std::move(bound);
  }

private:
  /**
   * Create a passive socket listening on an unspecified address on either IPv4 or IPv6 on the
   * given port.
   * @param  port The port to listen on.
   * @return A non-negative file descriptor of the TCP passive socket.
   */
  static std::uint32_t create_passive_socket(std::uint16_t port);

  void handle_data(const AddressWrapper &source, const microloop::Buffer &buffer);

private:
  std::uint16_t port_;
  DataHandler on_data_;
};

}  // namespace net_utils
