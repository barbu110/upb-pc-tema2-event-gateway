#include "gateway/subscriber_endpoint.h"

#include "commons/subscriber_messages.h"

#include <iostream>
#include <utility>

namespace gateway::endpoint
{

void SubscriberEndpoint::on_tcp_conn(microloop::net::TcpServer::PeerConnection &conn)
{
  std::cout << "Naked connection received from " << conn.str() << "\n";

  subscribers_.register_unnamed_client(conn);
}

void SubscriberEndpoint::on_tcp_data(microloop::net::TcpServer::PeerConnection &conn,
    const microloop::Buffer &buf)
{
  using namespace commons::subscriber_messages;
  using namespace commons::server_response;

  auto is_pending_conn = subscribers_.is_pending(conn.fd());

  if (buf.empty())
  {
    if (!is_pending_conn)
    {
      on_disconnect(*subscribers_.with_fd(conn.fd()));
    }

    server_.close_conn(conn);
    subscribers_.disconnect(conn);
    return;
  }

  std::uint8_t msg_type = static_cast<const std::uint8_t *>(buf.data())[0];

  if (!is_valid_message_type(msg_type))
  {
    // TODO Send error response to client.
    return;
  }

  auto message = from_buffer(buf);

  if (is_pending_conn)
  {
    if (msg_type != MessageType::GREETING)
    {
      // TODO Send error response to client.
      return;
    }

    auto greeting = std::get<GreetingMessage>(message);  // TODO Can this be an lvalue ref?

    auto subscriber_conn = subscribers_.attach_client_id(conn, greeting.client_id);
    if (!subscriber_conn)
    {
      ServerResponse error_response{StatusCode::DUPLICATE_CLIENT_ID};
      conn.send(error_response.serialize());

      server_.close_conn(conn);
      subscribers_.disconnect(conn);

      return;
    }

    on_client_greeting(*subscriber_conn);

    return;
  }

  if (msg_type == MessageType::GREETING)
  {
    // TODO Send error response to client.
    return;
  }

  std::visit(
      [&](auto &&arg) {
        using T = std::decay_t<decltype(arg)>;

        auto &subscriber = *subscribers_.with_fd(conn.fd());

        if constexpr (std::is_same_v<T, SubscribeRequest>)
        {
          on_subscribe(subscriber, arg);
        }
        else if constexpr (std::is_same_v<T, UnsubscribeRequest>)
        {
          on_unsubscribe(subscriber, arg);
        }
      },
      message);
}

void SubscriberEndpoint::on_disconnect(SubscriberConnection &subscriber)
{
  std::cout << "Client \"" << subscriber.client_id << "\" disconnected.\n";
}

void SubscriberEndpoint::on_client_greeting(SubscriberConnection &subscriber)
{
  std::cout << "New client \"" << subscriber.client_id << "\" connected from "
            << subscriber.raw_conn->str() << ".\n";
}

void SubscriberEndpoint::on_subscribe(SubscriberConnection &subscriber,
    const commons::subscriber_messages::SubscribeRequest &msg)
{
  std::cout << "Client " << subscriber.client_id << " requested subscription to " << msg.topic
            << "\n";
}

/* Callback to be invoked when a client sends an unsubscribe request. */
void SubscriberEndpoint::on_unsubscribe(SubscriberConnection &subscriber,
    const commons::subscriber_messages::UnsubscribeRequest &msg)
{
  std::cout << "Client " << subscriber.client_id << " requested to unsubscribe from " << msg.topic
            << "\n";
}

}  // namespace gateway::endpoint
