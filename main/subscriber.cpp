#include "subscriber/subscriber.h"
#include "microloop/event_loop.h"

#include <iostream>
#include <cstring>

int main(int argc, char **argv)
{
  if (argc < 4)
  {
    std::cerr << "usage: " << argv[0] << " client_id server_ip server_port\n";
    return -1;
  }

  if (argv[1][0] == '\0')
  {
    std::cerr << "error: Client ID cannot be empty.\n";
    return -1;
  }

  if (strchr(argv[1], ' '))
  {
    std::cerr << "error: Client ID cannot contain whitespace.\n";
    return -1;
  }

  std::uint16_t port = atoi(argv[3]);

  subscriber::Subscriber sub{argv[1], argv[2], port};

  while (MICROLOOP_TICK())
  {}

  return 0;
}
