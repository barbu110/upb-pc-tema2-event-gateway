#pragma once

#include "commons/device_messages.h"
#include "gateway/subscriber_conn.h"
#include "microloop/net/tcp_server.h"

#include <cstdint>
#include <functional>
#include <map>
#include <set>
#include <string>

namespace gateway
{

class SubscribersStorage
{
public:
  /**
   * \brief Add a new incoming connection without a client identifier. Such information is meant to
   * be transient. An unnamed connection shall disappear as soon as a client identifier is attached
   * to it.
   */
  void register_unnamed_client(microloop::net::TcpServer::PeerConnection &conn)
  {
    pending_conns_.emplace(conn.fd());
  }

  /**
   * \brief Retrieve a client with the identifier given by \p id.
   * \returns An optional subscriber connection object. Note that if there is a connection with
   * identifier \p id, but that connection does not have an active TCP tunnel, then it will not be
   * returned.
   */
  SubscriberConnection *named(std::string id)
  {
    for (auto &c : connections_)
    {
      if (c.raw_conn && c.client_id == id)
      {
        return &c;
      }
    }

    return nullptr;
  }

  /**
   * \brief Retrieve a named Subscriber Connection with the file descriptor \p fd.
   * \returns An optional subscriber connection object. Note that if the connection with the given
   * file descriptor is still pending, it will not be returned by this member function. Use
   * \ref is_pending instead.
   */
  SubscriberConnection *with_fd(std::uint32_t fd)
  {
    for (auto &c : connections_)
    {
      if (c.raw_conn && c.raw_conn->fd() == fd)
      {
        return &c;
      }
    }

    return nullptr;
  }

  /**
   * \brief Check if the gievn file descriptor represents a pending connection or not.
   */
  bool is_pending(std::uint32_t fd) const
  {
    return pending_conns_.find(fd) != pending_conns_.end();
  }

  /**
   * \brief Attach a client ID to a TCP connection.
   *
   * If a Subscriber Connection object with the identifier \p id is already registered, than the
   * TCP tunnel described by \p conn will simply be associated to that existing connection. However,
   * if no such Subscriber Connection exists, then a new one will be created, described by the given
   * TCP tunnel and identifier.
   *
   * \returns Address of the bound Subscriber Connection. If attempting to bind a TCP tunnel to a
   * named client that already has a TCP tunne asscoaited, the function will return an empty
   * optional object.
   */
  SubscriberConnection *attach_client_id(microloop::net::TcpServer::PeerConnection &conn,
      std::string id)
  {
    pending_conns_.erase(conn.fd());

    for (auto &c : connections_)
    {
      if (c.client_id == id)
      {
        if (c.raw_conn)
        {
          return nullptr;
        }

        c.raw_conn = &conn;
        return &c;
      }
    }

    /* Reaching up to this point means we do not have a connection with this ID. */

    return &connections_.emplace_back(SubscriberConnection{&conn, id});
  }

  /**
   * \brief Remove a client connection from the storage system if possible.
   *
   * This member function is to be invoked when a TCP connection is closed, or lost. If the
   * Subscriber Connection object identified by the \p conn TCP connection had any subscriptions
   * with Store&Forward feature enabled, the underlying object will be kept without any attached
   * TCP connections. If no such subscriptions are identified, the object will be completely removed
   * from the udnerlying data structures.
   */
  void disconnect(microloop::net::TcpServer::PeerConnection &conn)
  {
    if (auto it = pending_conns_.find(conn.fd()); it != pending_conns_.end())
    {
      pending_conns_.erase(it);
      return;
    }

    auto conn_it = std::find_if(connections_.begin(), connections_.end(),
        [&conn](auto &&c) { return c.raw_conn->fd() == conn.fd(); });

    if (conn_it == connections_.end())
    {
      return;
    }

    conn_it->raw_conn = nullptr;

    auto [first, last] = subscriptions_.equal_range(conn_it->client_id);
    auto sf_enabled_subs_count = std::count_if(first, last, [](auto &&p) {
      auto &[k, v] = p;
      return v.store_forward;
    });
    if (sf_enabled_subs_count != 0)
    {
      return;
    }

    /* Remove the subscriptions */
    subscriptions_.erase(first, last);

    /* Remove the connection itself */
    connections_.erase(conn_it);
  }

private:
  std::vector<SubscriberConnection> connections_;
  std::multimap<std::string, Subscription> subscriptions_;
  std::set<std::uint32_t> pending_conns_;
};

}  // namespace gateway
