EventGateway

Client-server application based on the "micro" framework [https://github.com/barbu110/micro].  The
application offers the following components:

    1. the Gateway - exposes TCP and UDP endpoints and creates the bridge between them, and
    2. the Subscriber - can subscribe to event topics available on the server and log received
       events


System Requirements

System requirements for the EventGateway system are imposed by the "micro" framework.  Any
application powered by "micro" framework must pe executed on a system with a minimum of two threads.
A C++17 compliant compiler must be used alongside the Bazel [https://bazel.build] Build System.  The
versions used in the development environment are the ones listed in the table below:

    +---------+----------------+
    | Product | Version        |
    +---------+----------------+
    | Clang   | 9.0.0          |
    | Bazel   | 1.2.0          |
    | Kernel  | 5.3.13-arch1-1 |
    +---------+----------------+

Note: a Docker image is provided to easily build the application.  Also, fully static linking is
not supported due to the use of the "pthreads" library.


Architecture

The system uses the Bazel build system in order to integrate with the "micro" framework.  C++17 is
used and the application is meant to only run on Linux [due to system specific dependencies, like
"epoll"].

As described above, the system is made of two main components: the Subscriber CLI app, and the
Gateway.  Both applications handle keyboard input while being able to process network data, by
employing the "KeyboardInput" event source.

Entry point in the Subscriber [//main:subscriber] application is defined by instantiating the
"subscriber::Subscriber" class that is built on top of the following relevant components:

    1. KeyboardInput:  emits an "input" event whenever there is data to be read on standard input.
       Some abstractions are made in order to reduce the effort when processing human data input
       (i.e. based on lines of ASCII data).  This is used to handle the keyboard commands included
       in the specification: "subscribe", "unsubscribe", and "exit".
    2. TcpClient:  allows the user to connect to a TCP endpoint using an (IP, port) pair.  Emits
       three events: "connect", "connect_err", and "data". These events are all handled in the
       Subscriber class in order to provide relevant feedback to users.

Commands are validated both on the client and on the server to protect against problematic input,
such as empty client identifiers, invalid values for the Store&Forward mechanism enable flag etc.

The communication between Subscribers and Gateway is handled using a custom-built request-response
protocol described in the "Data Communication" section.

Similarly, the Gateway [//main:gateway_server] has its entry point in the "gateway::Gateway" class.
This class is built on top of the following components:

    1. KeyboardInput:  again, with the purpose of handling the only supported keyboard command:
       "exit". This is treated using a "SIGINT" signal that will gracefully end the process.
    2. SubscriberEndpoint: the TCP endpoint meant to be used by Subscriber applications.
    3. InputEndpoint: the UDP endpoint meant to be used by clients to submit data into the system.

The tree of components employed by every one described above goes very deep, so not very much detail
is going to be given for all of them.  However, in the following parts, an overview of the
communication protocol and type-safety measures will be given.


Data Communication [on the Input Endpoint]

Type-safety is the core value of the "micro" framework and, implicitly of this application. So,
whenever a message is received, it goes through various level of abstraction in order to provide
full type safety to the end user:

    1. It is passed through a Buffer abstraction, microloop::Buffer to ensure no memory leak occurs,
       and to improve user experience with regard to operations such as concatenation, trimming
       bytes from buffers etc.
    2. It then goes through a deserialization function that translates the messages from the raw
       buffer into a plain-old-data (POD) structure, then into a C++ structure offering high-level
       types, such as "std::string".  The message is then weapped in a C++ variant type to improve
       the type safety even more.
    3. Processing of every message is done by "visiting" [see the reference to "std::visit" --
       https://en.cppreference.com/w/cpp/utility/variant/visit] the "GenericDeviceMessage".

After the message has gone through all the levels described above, the "data" event handler for the
InputEndpoint is invoked with the parsed data.


Data Communication [on the Subscriber Endpoint]

The communication protocol involved when passing data around between Subscriber instances and the
Gateway (i.e. the SubscriberEndpoint component) is a little more complex than the one described in
the previous section.  It is based on a request-response model.

Every message going on through the network starts with a message header:

   +----------+------+
   | Field    | Size |
   +----------+------+
   | type     | 1    |
   | msg_size | 2    |
   +----------+------+
   |     3 bytes     |
   +-----------------+

This helps the applications identify what type of message is being delivered, and the total size
of the message (without the header).

There are five types of messages the protocol is currently able to handle:

   1. GREETING:  the message sent by a Subscriber immediately after connection to identify
      themselves with a client identifier.
   2. SUBSCRIBE:  the message sent by a Subscriber to create a subscription to the given topic, with
      or without the Store&Forward mechanism.
   3. UNSUBSCRIBE:  the message sent by a subscriber to inform the Gateway they are no lonver
      interested in a particular topic.
   4. RESPONSE:  the message sent by the Gateway to a Subscriber in response to an action they have
      submitted.  This is used to inform clients of success or failure of their actions.
   5. DEVICE NOTIFICATION:  the message sent by the Gateway to a Subscriber wrapping the message
      sent by a device on the Input Endpoint.

In order for a message to be transmitted between the Gateway and Subscriber instances, it must go
through a serialization sequence, that is meant to transform the high-level, type-safe message
structure into data ready to be sent over the network.

Let's go into a little bit more detail with the server RESPONSE. The structure of such a response is
described by the following table:

   +-------+------+
   | Field | Size |
   +-------+------+
   | code  | 1    |
   | notes | 64   |
   +-------+------+
   |   65 bytes   |
   +--------------+

The Subscriber application renders feedback for actions submitted by the user based on resopnses
from the server.  For example, a successful subscription action would look like this:

   > subscribe some_topic true
   response: subscribed to some_topic


Further Possible Improvements

Currently, subscriptions with the Store&Forward feature enabled have the Gateway queue messages to
be sent. These messages are stored in a linked list structure (i.e. "std::list") in order to prevent
errors when allocating very large blocks of contiguous memory (as "std::vector" would require).
However, storing extremely large amounts of messages can become a problem.  A solution to this would
be to use a memory-mapped file [mmap].  This would even preserve messages accross numerous
server "deaths".
