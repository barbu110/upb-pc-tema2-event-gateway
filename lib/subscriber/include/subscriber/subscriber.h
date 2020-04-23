#pragma once

#include "absl/strings/str_split.h"
#include "commons/subscriber_messages.h"
#include "net_utils/keyboard_input.h"
#include "net_utils/tcp_client.h"

#include <cstdint>
#include <iostream>
#include <signal.h>
#include <string>
#include <type_traits>
#include <unistd.h>
#include <variant>

namespace subscriber
{

class Subscriber
{
public:
  Subscriber(std::string client_id, std::string server_ip, std::uint16_t server_port) :
      client_id_{client_id}, client_{server_ip, server_port}
  {
    using microloop::EventLoop;

    client_.on_connect(&Subscriber::on_connect, this);
    client_.on_connect_err(&Subscriber::on_connect_err, this);
    client_.on_data(&Subscriber::on_server_data, this);

    auto keyboard = new net_utils::KeyboardInput;
    keyboard->on_input(&Subscriber::on_keyboard_input, this);
    EventLoop::instance().add_event_source(keyboard);

    EventLoop::instance().register_signal_handler(SIGINT, [](std::uint32_t) {
      /*
       * This is here just to allow the application to exit smoothly, performing stack unwinding and
       * every other avaiable clean up.
       */

      return true;
    });

    client_.connect();
  }

private:
  void on_connect(net_utils::AddressWrapper &c)
  {
    conn_ = &c;

    std::cout << "Connected to " << c.str() << "\n";

    commons::subscriber_messages::GreetingMessage greeting{client_id_};
    c.send(greeting.serialize());
  }

  void on_connect_err(const std::string &err)
  {
    std::cerr << "error: " << err << "\n";

    kill(getpid(), SIGINT);
  }

  void on_server_data(const microloop::Buffer &buf)
  {
    if (buf.empty())
    {
      // TODO Print goodbye message
      kill(getpid(), SIGINT);
      return;
    }

    using namespace commons::subscriber_messages;

    fragment_ += buf;

    while (can_parse_entire_msg(fragment_))
    {
      auto [message, consumed] = from_buffer(fragment_);
      fragment_.remove_prefix(consumed);

      std::visit(
          [&](auto &&msg) {
            using T = std::decay_t<decltype(msg)>;

            if constexpr (std::is_same_v<T, ServerResponse>)
            {
              using namespace commons::server_response;

              if (msg.code == StatusCode::OK) {}
              else if (msg.code == StatusCode::SUBSCRIBE_SUCCESSFUL)
              {
                auto topic = msg.notes;
                std::cout << "response: subscribed to " << topic << "\n";
              }
              else if (msg.code == StatusCode::UNSUBSCRIBE_SUCCESSFUL)
              {
                auto topic = msg.notes;
                std::cout << "response: unsubscribed from " << topic << "\n";
              }
              else
              {
                std::cerr << "error response: " << status_str(msg.code) << "\n";
              }
            }
            else if constexpr (std::is_same_v<T, DeviceNotification>)
            {
              std::cout << msg.device_address << " - ";
              std::visit([](auto &&m) { std::cout << m.str(); }, msg.original_message);
              std::cout << "\n";
            }
          },
          message);
    }
  }

  void on_keyboard_input(const std::string &input)
  {
    if (input.empty())
    {
      return;
    }

    if (input == "exit" || input == "q")
    {
      kill(getpid(), SIGINT);
    }

    std::vector<std::string_view> parts = absl::StrSplit(input, ' ');
    auto command = parts.front();

    if (command == "subscribe")
    {
      static constexpr std::string_view usage = "subscribe topic store_forward";
      if (parts.size() != 3)
      {
        std::cerr << "usage: " << usage << "\n";
        return;
      }

      auto topic = parts[1];
      bool store_forward;

      if (parts[2] == "true" || parts[2] == "TRUE" || parts[2] == "1")
      {
        store_forward = true;
      }
      else if (parts[2] == "false" || parts[2] == "FALSE" || parts[2] == "0")
      {
        store_forward = false;
      }
      else
      {
        std::cerr << "error: invalid value for store_forward\n";
        return;
      }

      commons::subscriber_messages::SubscribeRequest request{std::string{topic}, store_forward};
      conn_->send(request.serialize());
    }
    else if (command == "unsubscribe")
    {
      static constexpr std::string_view usage = "unsubscribe topic";
      if (parts.size() != 2)
      {
        std::cerr << "usage: " << usage << "\n";
        return;
      }

      auto topic = parts[1];

      commons::subscriber_messages::UnsubscribeRequest request{std::string{topic}};
      conn_->send(request.serialize());
    }
    else
    {
      std::cerr << "unknown command: " << command << "\n";
    }
  }

private:
  std::string client_id_;
  net_utils::TcpClient client_;
  net_utils::AddressWrapper *conn_;  // Not managed by this class.
  microloop::Buffer fragment_;  // Fragment of message to be parsed on next read iteration.
};

}  // namespace subscriber
