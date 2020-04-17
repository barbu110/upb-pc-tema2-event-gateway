#include "gateway/gateway.h"
#include "microloop/event_loop.h"
#include "net_utils/keyboard_input.h"

#include <iostream>
#include <signal.h>

int main()
{
  gateway::Gateway gateway{8900};

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
