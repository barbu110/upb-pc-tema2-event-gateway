#pragma once

#include "commons/subscriber_messages.h"
#include "microloop/net/tcp_server.h"

#include <functional>
#include <queue>

namespace gateway
{

struct Subscription
{
  std::string client_id;
  std::string topic;
  bool store_forward;
};

struct SubscriberConnection
{
  /* The raw TCP connection to the peer socket. */
  microloop::net::TcpServer::PeerConnection *raw_conn;

  /* The client ID as provided by the Greeting message from a client upon connection. */
  std::string client_id;

  std::queue<commons::subscriber_messages::DeviceNotification> pending_messages;

  bool active() const
  {
    return raw_conn != nullptr;
  }
};

}  // namespace gateway
