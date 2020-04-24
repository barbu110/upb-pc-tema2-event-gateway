#pragma once

#include "commons/subscriber_messages.h"
#include "microloop/net/tcp_server.h"

#include <functional>
#include <queue>

namespace gateway
{

struct Subscription
{
  /* The client identifier. */
  std::string client_id;

  /* The topic identifier. */
  std::string topic;

  /* Whether the Store&Forward feature is enabled for this subscription. */
  bool store_forward;
};

struct SubscriberConnection
{
  /* The raw TCP connection to the peer socket. */
  microloop::net::TcpServer::PeerConnection *raw_conn;

  /* The client ID as provided by the Greeting message from a client upon connection. */
  std::string client_id;

  /* Messages to be sent upon susbcriber re-connection. */
  std::queue<commons::subscriber_messages::DeviceNotification> pending_messages;

  bool active() const
  {
    return raw_conn != nullptr;
  }
};

}  // namespace gateway
