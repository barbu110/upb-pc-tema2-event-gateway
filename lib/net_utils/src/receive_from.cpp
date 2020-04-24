#include "net_utils/receive_from.h"

#include <cstdio>
#include <netdb.h>
#include <netinet/in.h>
#include <sstream>
#include <sys/socket.h>

namespace net_utils
{

bool AddressWrapper::send(const microloop::Buffer &buf) const
{
  std::size_t total_sent = 0;

  while (total_sent != buf.size())
  {
    const std::uint8_t *data = static_cast<const std::uint8_t *>(buf.data());
    ssize_t nsent = ::sendto(
        server_sock_, data + total_sent, buf.size() - total_sent, 0, (sockaddr *)&addr_, addrlen_);
    if (nsent == -1)
    {
      return false;
    }

    total_sent += nsent;
  }

  return true;
}

std::string AddressWrapper::str() const
{
  static constexpr std::size_t port_strlen = 8;

  char hostbuf[INET6_ADDRSTRLEN]{};
  char portbuf[port_strlen]{};

  auto err_code = getnameinfo(reinterpret_cast<const sockaddr *>(&addr_), addrlen_, hostbuf,
      sizeof(hostbuf), portbuf, sizeof(portbuf), NI_NUMERICHOST | NI_NUMERICSERV);
  if (err_code != 0)
  {
    std::stringstream err;
    err << __PRETTY_FUNCTION__ << ": " << gai_strerror(err_code);

    throw std::runtime_error(err.str());
  }

  char buf[sizeof(hostbuf) + sizeof(portbuf) + 1]{};
  /* No buffer overflow can occur here, due to both buffer being checked prior this call. */
  sprintf(buf, "%s:%s", hostbuf, portbuf);

  return buf;
}

}  // namespace net_utils
