#pragma once

#include "commons/subscriber_messages.h"
#include "gateway/subscriber_conn.h"
#include "gateway/subscribers_storage.h"
#include "microloop/net/tcp_server.h"

namespace gateway::endpoint
{

class SubscriberEndpoint
{
public:
  SubscriberEndpoint(std::uint16_t port, SubscribersStorage &ss) : server_{port}, subscribers_{ss}
  {
    server_.set_connection_callback(&SubscriberEndpoint::on_tcp_conn, this);
    server_.set_data_callback(&SubscriberEndpoint::on_tcp_data, this);
  }

private:
  /* Callback to be invoked when a new client connects. */
  void on_tcp_conn(microloop::net::TcpServer::PeerConnection &conn);

  /* Callback to be invoked when new data arrives on the TCP endpoint. */
  void on_tcp_data(microloop::net::TcpServer::PeerConnection &conn, const microloop::Buffer &buf);

  /* Callback to be invoked when a client disconnects. */
  void on_disconnect(SubscriberConnection &client);

  /* Callback to be invoked when a client sends a Greeting message. */
  void on_client_greeting(SubscriberConnection &subscriber);

  /* Callback to be invoked when a client sends a subscribe request. */
  void on_subscribe(SubscriberConnection &subscriber,
      const commons::subscriber_messages::SubscribeRequest &msg);

  /* Callback to be invoked when a client sends an unsubscribe request. */
  void on_unsubscribe(SubscriberConnection &subscriber,
      const commons::subscriber_messages::UnsubscribeRequest &msg);

private:
  SubscribersStorage &subscribers_;
  microloop::net::TcpServer server_;
};

}  // namespace gateway::endpoint
