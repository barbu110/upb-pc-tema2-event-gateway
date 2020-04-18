#pragma once

#include "commons/device_messages.h"
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

  /**
   * Messages waiting to be sent upon client connection. Only messages in topics for which the
   * client has the Store&Forward feature enabled will be stored in this list. This queue is
   * essentially nothing but a VIEW in the main queue of pending messages.
   */
  std::queue<std::reference_wrapper<commons::device_messages::GenericDeviceMessage>>
      pending_messages;

  /**
   * Subscriptions for this client. This list a view in the main subscrption manager.
   */
  std::vector<std::reference_wrapper<Subscription>> subscriptions;
};

}  // namespace gateway
