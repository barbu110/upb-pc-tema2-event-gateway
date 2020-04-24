#pragma once

#include "microloop/buffer.h"
#include "microloop/event_source.h"
#include "microloop/kernel_exception.h"

#include <fcntl.h>
#include <string>
#include <tuple>
#include <unistd.h>

namespace net_utils
{

/**
 * \brief Mechanism for handling keyboard input events.
 *
 * The event emitted by this event source is "input".
 */
class KeyboardInput : public microloop::EventSource, protected microloop::TypeHelper<std::string>
{
public:
  KeyboardInput(bool remove_trailing_newline = true) :
      EventSource{STDIN_FILENO}, remove_trailing_newline_{remove_trailing_newline}
  {
    auto flags = fcntl(get_fd(), F_GETFL);
    if (flags == -1)
    {
      throw microloop::KernelException(errno);
    }

    flags |= O_NONBLOCK;
    if (fcntl(get_fd(), F_SETFL, flags) == -1)
    {
      throw microloop::KernelException(errno);
    }
  }

  template <class Func, class... Args>
  void on_input(Func &&func, Args &&... args)
  {
    using namespace std::placeholders;
    on_input_ = std::bind(std::forward<Func>(func), std::forward<Args>(args)..., _1);
  }

  std::uint32_t produced_events() const override
  {
    return EPOLLIN;
  }

  bool native_async() const override
  {
    /* Run this event source on the main thread, so reading the input can block. */
    return true;
  }

  void start() override
  {}

  void run_callback() override
  {
    read_input();
    std::apply(on_input_, get_return_object());
  }

private:
  void read_input()
  {
    /*
     * Keyboard inputs are generally expected to be relatively small, so reducing the amount of
     * memory to allocate is feasible.
     */
    char buf[512]{};

    std::string input;
    input.reserve(sizeof(buf));

    ssize_t nread;

    do
    {
      nread = read(STDIN_FILENO, buf, sizeof(buf));
      if (nread == -1)
      {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
          break;
        }

        throw microloop::KernelException{errno};
      }

      input.append(buf, nread);
    } while (nread != 0);

    if (remove_trailing_newline_ && input.back() == '\n')
    {
      input.pop_back();
    }

    set_return_object(input);
  }

private:
  Callback on_input_;
  bool remove_trailing_newline_;
};

}  // namespace net_utils
