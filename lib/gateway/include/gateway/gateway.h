#pragma once

#include "commons/device_messages.h"
#include "commons/subscriber_messages.h"
#include "gateway/input_endpoint.h"
#include "gateway/subscriber_conn.h"
#include "microloop/net/tcp_server.h"
#include "net_utils/udp_server.h"

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
  Gateway(int port) : input_endpoint_{port}, tcp_server_{port}
  {
    // tcp_server_.set_connection_callback(&Gateway::on_conn, this);
    // tcp_server_.set_data_callback(&Gateway::on_data, this);

    input_endpoint_.subscribe(&Gateway::publish, this);
  }

private:
  void publish(const net_utils::AddressWrapper &source,
      const commons::device_messages::GenericDeviceMessage &msg)
  {
    std::visit([&](auto &&arg) { std::cout << source.str() << " - " << arg.str() << "\n"; }, msg);
  }

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
  endpoint::InputEndpoint input_endpoint_;
  microloop::net::TcpServer tcp_server_;
  std::map<std::int32_t, SubscriberConnection> clients_;
};

}  // namespace gateway
