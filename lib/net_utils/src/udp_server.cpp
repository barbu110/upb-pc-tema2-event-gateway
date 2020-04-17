#include "net_utils/udp_server.h"

#include "microloop/event_loop.h"
#include "microloop/event_source.h"
#include "net_utils/receive_from.h"

#include <functional>
#include <netdb.h>
#include <signal.h>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>

namespace net_utils
{

UdpServer::UdpServer(std::uint16_t port) : port_{port}
{
  using namespace std::placeholders;
  using microloop::EventLoop;

  auto server_fd = create_passive_socket(port);

  auto data_handler = std::bind(&UdpServer::handle_data, this, _1, _2);
  EventLoop::instance().add_event_source(new ReceiveFrom(server_fd, data_handler));

  EventLoop::instance().register_signal_handler(SIGINT, [](std::uint32_t) {
    /*
     * This is here just to allow the application to exit smoothly, performing stack unwinding and
     * every other avaiable clean up.
     */
    return true;
  });
}

std::uint32_t UdpServer::create_passive_socket(std::uint16_t port)
{
  auto port_str = std::to_string(port);

  addrinfo *results;
  addrinfo hints{};

  hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV;
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;

  auto err_code = getaddrinfo(nullptr, port_str.c_str(), &hints, &results);
  if (err_code != 0)
  {
    std::stringstream err;
    err << __PRETTY_FUNCTION__ << ": " << gai_strerror(err_code);

    throw std::runtime_error(err.str());
  }

  std::int32_t fd;
  auto r = results;
  for (; r != nullptr; r = r->ai_next)
  {
    fd = socket(r->ai_family, r->ai_socktype, r->ai_protocol);
    if (fd < 0)
    {
      continue;
    }

    if (bind(fd, r->ai_addr, r->ai_addrlen) == 0)
    {
      /*
       * We now have a valid socket.
       */
      break;
    }

    close(fd);
  }

  if (r == nullptr)
  {
    std::stringstream err;
    err << __PRETTY_FUNCTION__ << ": "
        << "Failed to bind to port " << port;

    throw std::runtime_error(err.str());
  }

  freeaddrinfo(results);

  return static_cast<std::uint32_t>(fd);
}

void UdpServer::handle_data(const AddressWrapper &source, const microloop::Buffer &buf)
{
  on_data_(source, buf);
}

}  // namespace net_utils
