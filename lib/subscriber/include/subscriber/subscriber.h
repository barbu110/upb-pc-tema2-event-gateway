#pragma once

#include "commons/subscriber_messages.h"
#include "net_utils/keyboard_input.h"
#include "net_utils/tcp_client.h"

#include <cstdint>
#include <iostream>
#include <signal.h>
#include <string>
#include <unistd.h>

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
  {}

  void on_keyboard_input(const std::string &input)
  {
    if (input == "exit" || input == "q")
    {
      kill(getpid(), SIGINT);
    }
  }

private:
  std::string client_id_;
  net_utils::TcpClient client_;
  net_utils::AddressWrapper *conn_;
};

}  // namespace subscriber
