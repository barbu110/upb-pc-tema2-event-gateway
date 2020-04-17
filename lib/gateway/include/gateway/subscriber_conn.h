#pragma once

#include "microloop/net/tcp_server.h"

namespace gateway
{

struct SubscriberConnection
{
  /* The raw TCP connection to the peer socket. */
  microloop::net::TcpServer::PeerConnection &raw_conn;

  /* The client ID as provided by the Greeting message from a client upon connection. */
  std::string client_id;
};

}  // namespace gateway
