#include "gateway/server.h"
#include "microloop/event_loop.h"

#include <iostream>

int main()
{
  gateway::Server srv{8900};

  while (MICROLOOP_TICK())
  {
    std::cout << "Tick...\n";
  }
}
