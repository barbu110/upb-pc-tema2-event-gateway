#include "gateway/gatewway.h"
#include "microloop/event_loop.h"

#include <iostream>

int main()
{
  gateway::Gateway gateway{8900};

  while (MICROLOOP_TICK())
  {}
}
