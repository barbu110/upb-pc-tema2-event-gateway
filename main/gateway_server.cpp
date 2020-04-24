#include "gateway/gateway.h"
#include "microloop/event_loop.h"
#include "net_utils/keyboard_input.h"

#include <iostream>
#include <signal.h>

int main(int argc, char **argv)
{
  if (argc < 2)
  {
    std::cerr << "usage: " << argv[0] << " port\n";
    return -1;
  }

  int port = atoi(argv[1]);
  if (port == 0)
  {
    std::cerr << "error: invalid port number\n";
    return -1;
  }

  signal(SIGWINCH, SIG_IGN);

  gateway::Gateway gateway{port};

  auto keyboard_input = new net_utils::KeyboardInput;
  keyboard_input->on_input([](const std::string &data) {
    if (data == "exit")
    {
      kill(getpid(), SIGINT);
    }
  });

  microloop::EventLoop::instance().add_event_source(keyboard_input);

  while (MICROLOOP_TICK())
  {}
}
