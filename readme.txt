Micro Event Gateway

Client-server application based on the micro framework [https://github.com/barbu110/micro].  The
application offers the following components:
    
    1. Server - exposes TCP and UDP endpoints
    2. Subscriber Application - can subscribe to event sources available on the server [topics] and
       log received events


Architecture

The system uses the Bazel build system in order to integrate with the micro framework.  C++17 is 
used and the application is meant to only run on Linux.

