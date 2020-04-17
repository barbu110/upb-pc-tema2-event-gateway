#include "gateway/gateway.h"

namespace gateway
{

void Gateway::on_conn(microloop::net::TcpServer::PeerConnection &conn)
{
  if (clients_.find(conn.fd()) == clients_.end())
  {
    /* Client ID is left to be completed upon incoming data */
    clients_.emplace(conn.fd(), SubscriberConnection{conn});
  }
}

void Gateway::on_tcp_data(microloop::net::TcpServer::PeerConnection &conn, const microloop::Buffer &buf)
{
  auto client_it = clients_.find(conn.fd());
  auto &[fd, client] = *client_it;

  if (buf.empty())
  {
    on_disconnect(client);
    return;
  }

  auto message = commons::subscriber_messages::from_buffer(buf);
  std::visit(
      [&](auto &&arg) {
        using T = std::decay_t<decltype(arg)>;

        if constexpr (std::is_same_v<T, commons::subscriber_messages::GreetingMessage>)
        {
          on_client_greeting(client, std::forward<T>(arg));
        }
        else if constexpr (std::is_same_v<T, commons::subscriber_messages::SubscribeRequest>)
        {
          on_subscribe(client, std::forward<T>(arg));
        }
        else if constexpr (std::is_same_v<T, commons::subscriber_messages::UnsubscribeRequest>)
        {
          on_unsubscribe(client, std::forward<T>(arg));
        }
      },
      message);
}

void Gateway::on_disconnect(SubscriberConnection &subscriber)
{
  std::cout << "Client \"" << subscriber.client_id << "\" disconnected.\n";

  auto fd = subscriber.raw_conn.fd();

  tcp_server_.close_conn(subscriber.raw_conn);
  clients_.erase(fd);
}

void Gateway::on_client_greeting(SubscriberConnection &subscriber,
    const commons::subscriber_messages::GreetingMessage &msg)
{
  using std::cout;

  auto peer_address = subscriber.raw_conn.str(false);
  cout << "Client \"" << msg.client_id << "\" connected from " << peer_address << ".\n";

  subscriber.client_id = msg.client_id;
}

void Gateway::on_subscribe(SubscriberConnection &subscriber,
    const commons::subscriber_messages::SubscribeRequest &msg)
{}

/* Callback to be invoked when a client sends an unsubscribe request. */
void Gateway::on_unsubscribe(SubscriberConnection &subscriber,
    const commons::subscriber_messages::UnsubscribeRequest &msg)
{}

}  // namespace gateway
