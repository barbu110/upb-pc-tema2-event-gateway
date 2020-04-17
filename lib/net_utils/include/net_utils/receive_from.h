#pragma once

#include "microloop/buffer.h"
#include "microloop/event_source.h"
#include "microloop/kernel_exception.h"

#include <atomic>
#include <string>
#include <sys/socket.h>
#include <tuple>

namespace net_utils
{

class AddressWrapper
{
public:
  AddressWrapper() = default;

  AddressWrapper(std::uint32_t server_sock, sockaddr_storage addr, socklen_t addrlen) :
      server_sock_{server_sock}, addr_{addr}, addrlen_{addrlen}
  {}

  /**
   * Send a buffer to this client from the UDP server.
   *
   * This member function is a thin wrapper around the `sendto` syscall.
   *
   * \param buf The buffer to be sent to the peer socket.
   * \return Whether the operation succeeded or not. If only a fraction of the entire buffer is
   * sent, then the function will report it as a failure.
   */
  bool send(const microloop::Buffer &buf) const;

  /**
   * \brief Get a string representation of this peer connection. The representation will contain
   * a pretty representation of the socket address.
   * \return The string representation of an address wrapper with the following format:
   *     <ip>:<port>
   */
  std::string str() const;

  auto addr() const
  {
    return std::make_pair(addr_, addrlen_);
  }

private:
  sockaddr_storage addr_;
  socklen_t addrlen_;
  std::uint32_t server_sock_;
};

class ReceiveFrom :
    public microloop::EventSource,
    public microloop::TypeHelper<AddressWrapper, microloop::Buffer>
{
public:
  static constexpr std::size_t DEFAULT_MAX_READ_SIZE = 2048;

  ReceiveFrom(std::uint32_t sock,
      Callback &&callback,
      std::size_t max_read_size = DEFAULT_MAX_READ_SIZE) :
      EventSource{sock}, on_recv_{std::move(callback)}, max_read_size_{max_read_size}
  {}

  std::uint32_t produced_events() const override
  {
    return EPOLLIN;
  }

  bool native_async() const override
  {
    return false;
  }

  void start() override
  {}

  void run_callback() override
  {
    run_recv();
    std::apply(on_recv_, get_return_object());
  }

private:
  void run_recv()
  {
    microloop::Buffer buf{max_read_size_};
    sockaddr_storage addr{};
    socklen_t addrlen = sizeof(addr);

    ssize_t nrecv = recvfrom(get_fd(), buf.data(), buf.size(), 0, (sockaddr *)&addr, &addrlen);
    if (nrecv == -1)
    {
      if (errno == EAGAIN || errno == EWOULDBLOCK)
      {
        return;
      }

      throw microloop::KernelException(errno);
    }

    buf.resize(nrecv);

    set_return_object(std::make_tuple(AddressWrapper{get_fd(), addr, addrlen}, buf));
  }

private:
  std::size_t max_read_size_;
  Callback on_recv_;
};

}  // namespace net_utils
