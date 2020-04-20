#pragma once

#include "microloop/buffer.h"
#include "microloop/event_loop.h"
#include "microloop/event_sources/net/receive.h"
#include "microloop/net/tcp_server.h"
#include "net_utils/receive_from.h"

#include <cstdint>
#include <functional>
#include <memory>
#include <netdb.h>
#include <optional>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace net_utils
{

class TcpClient
{
  using ConnectHandler = std::function<void(net_utils::AddressWrapper &)>;
  using ConnectErrHandler = std::function<void(std::string)>;
  using DataHandler = std::function<void(const microloop::Buffer &)>;

public:
  TcpClient(std::string ip, std::uint16_t port) : ip_{ip}, port_{port}
  {}

  /**
   * \brief Binds the connect event to an event handler.
   */
  template <class Func, class... Args>
  void on_connect(Func &&func, Args &&... args)
  {
    using namespace std::placeholders;
    on_connect_ = std::bind(std::forward<Func>(func), std::forward<Args>(args)..., _1);
  }

  /**
   * \brief Binds the connect error event to an event handler.
   */
  template <class Func, class... Args>
  void on_connect_err(Func &&func, Args &&... args)
  {
    using namespace std::placeholders;
    on_connect_err_ = std::bind(std::forward<Func>(func), std::forward<Args>(args)..., _1);
  }

  /**
   * \brief Binds the data event to an event handler. This callback will be invoked whenever the
   * TCP server sends data to this connection.
   */
  template <class Func, class... Args>
  void on_data(Func &&func, Args &&... args)
  {
    using namespace std::placeholders;
    on_data_ = std::bind(std::forward<Func>(func), std::forward<Args>(args)..., _1);
  }

  std::int32_t connect()
  {
    addrinfo hints{};
    addrinfo *result;

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_NUMERICSERV;

    auto port_str = std::to_string(port_);

    if (auto err_code = getaddrinfo(ip_.c_str(), port_str.c_str(), &hints, &result); err_code != 0)
    {
      if (static_cast<bool>(on_connect_err_))
      {
        (*on_connect_err_)(gai_strerror(err_code));
      }

      return -1;
    }

    std::int32_t client_fd;

    addrinfo *rp;

    sockaddr_storage addr;
    socklen_t addrlen;

    for (rp = result; rp != nullptr; rp = rp->ai_next)
    {
      client_fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
      if (client_fd == -1)
      {
        continue;
      }

      if (::connect(client_fd, rp->ai_addr, rp->ai_addrlen) != -1)
      {
        memcpy(&addr, rp->ai_addr, rp->ai_addrlen);
        addrlen = rp->ai_addrlen;

        break;
      }

      ::close(client_fd);
    }

    freeaddrinfo(result);

    if (rp == nullptr)
    {
      if (static_cast<bool>(on_connect_err_))
      {
        (*on_connect_err_)("Could not find a suitable connection.");
      }


      return -1;
    }

    if (static_cast<bool>(on_connect_))
    {
      using microloop::net::TcpServer;
      server_addr_ = std::make_unique<net_utils::AddressWrapper>(client_fd, addr, addrlen);
      (*on_connect_)(*server_addr_);
    }

    using microloop::EventLoop;
    using microloop::event_sources::net::Receive;

    auto receive_event_source = new Receive<false>(client_fd);
    receive_event_source->set_on_recv(on_data_);

    EventLoop::instance().add_event_source(receive_event_source);

    return client_fd;
  }

private:
  std::string ip_;
  std::uint16_t port_;
  std::optional<ConnectHandler> on_connect_;
  std::optional<ConnectErrHandler> on_connect_err_;
  DataHandler on_data_;
  std::unique_ptr<net_utils::AddressWrapper> server_addr_;
};

}  // namespace net_utils
