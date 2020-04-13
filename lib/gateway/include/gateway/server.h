#pragma once

#include "microloop/net/tcp_server.h"
#include "subscriber/messages.h"

#include <cstdint>
#include <iostream>
#include <map>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>

namespace gateway
{

struct Client
{
  microloop::net::TcpServer::PeerConnection &raw_conn;
  std::string client_id;
};

class Server
{
public:
  Server(int port) : server_{port}
  {
    server_.set_connection_callback(&Server::on_conn, this);
    server_.set_data_callback(&Server::on_data, this);
  }

private:
  void on_conn(microloop::net::TcpServer::PeerConnection &conn)
  {
    std::cout << "Client connected from (" << conn.str() << ")\n";

    if (clients_.find(conn.fd) == clients_.end())
    {
      /* Client ID is left to be completed upon incoming data */
      clients_.emplace(conn.fd, Client{conn});
    }
  }

  void on_data(microloop::net::TcpServer::PeerConnection &conn, const microloop::Buffer &buf)
  {
    if (buf.empty())
    {
      std::cout << "Closing connection (" << conn.str() << ")\n";

      clients_.erase(conn.fd);
      conn.close();

      return;
    }

    std::cout << "Message from connection (" << conn.str() << ")\n";

    auto client_it = clients_.find(conn.fd);
    if (client_it == clients_.end())
    {
      return;
    }

    auto &[fd, client] = *client_it;

    auto message = subscriber::messages::from_buffer(buf);
    std::visit(
        [&, &client](auto &&arg) {
          using T = std::decay_t<decltype(arg)>;

          if constexpr (std::is_same_v<T, subscriber::messages::GreetingMessage>)
          {
            on_client_greeting(client, std::forward<T>(arg));
          }
          else if constexpr (std::is_same_v<T, subscriber::messages::SubscribeRequest>)
          {
            on_subscribe(client, std::forward<T>(arg));
          }
          else if constexpr (std::is_same_v<T, subscriber::messages::UnsubscribeRequest>)
          {
            on_unsubscribe(client, std::forward<T>(arg));
          }
        },
        message);
  }

  void on_client_greeting(Client &client, const subscriber::messages::GreetingMessage &msg)
  {
    std::cout << "Greeting message from client (" << client.raw_conn.str() << ")\n";
    std::cout << "Client (" << client.raw_conn.str() << " is identified by " << msg.client_id
              << "\n";
  }

  void on_subscribe(Client &client, const subscriber::messages::SubscribeRequest &msg)
  {}

  void on_unsubscribe(Client &client, const subscriber::messages::UnsubscribeRequest &msg)
  {}

private:
  microloop::net::TcpServer server_;
  std::map<std::int32_t, Client> clients_;
};

}  // namespace gateway
