#pragma once

#include "commons/subscriber_messages.h"
#include "gateway/subscriber_conn.h"
#include "microloop/net/tcp_server.h"

#include <cstdint>
#include <iostream>
#include <map>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>

namespace gateway
{

class Gateway
{
public:
  Gateway(int port) : tcp_server_{port}, udp_server_{port}
  {
    tcp_server_.set_connection_callback(&Server::on_conn, this);
    tcp_server_.set_data_callback(&Server::on_data, this);
  }

private:
  /* Callback to be invoked when a new client connects. */
  void on_conn(microloop::net::TcpServer::PeerConnection &conn);

  /* Callback to be invoked when new data arrives on the TCP endpoint. */
  void on_tcp_data(microloop::net::TcpServer::PeerConnection &conn, const microloop::Buffer &buf);

  /* Callback to be invoked when a client disconnects. */
  void on_disconnect(SubscriberConnection &client);

  /* Callback to be invoked when a client sends a Greeting message. */
  void on_client_greeting(SubscriberConnection &subscriber,
      const commons::subscriber_messages::GreetingMessage &msg);

  /* Callback to be invoked when a client sends a subscribe request. */
  void on_subscribe(SubscriberConnection &subscriber,
      const commons::subscriber_messages::SubscribeRequest &msg);

  /* Callback to be invoked when a client sends an unsubscribe request. */
  void on_unsubscribe(SubscriberConnection &subscriber,
      const commons::subscriber_messages::UnsubscribeRequest &msg);

private:
  microloop::net::TcpServer tcp_server_;
  net_utils::UdpServer udp_server_;
  std::map<std::int32_t, SubscriberConnection> clients_;
};

}  // namespace gateway
