# AOS project 2
Maekawa's Distributed Mutual Exclusion Algorithm

Platform:
 - OS: Unix
 - Compiler: g++ 4.8.5

To Build:
1. unzip maekawa-mutex.zip
2. cd to maekawa-mutex/
3. run make

To Run:
To run ServerX
    -- log into dc1X.utdallas.edu
    -- cd to maekawa-mutex/bin/
    -- run ./server X

To run ClientX
    -- log into dc3X.utdallas.edu
    -- cd to maekawa-mutex/bin/
    -- run ./client X

Output and logs:
 - serverX.out -- server X's output
 - statsX.log -- client X's message stats and elapsed time for aquiring critical section access
 - clientX.log -- client X's communication history

NOTE:
1. The hosts mentioned in the run instructions are for those specified in server.config and client.config.
2. The host configuration for servers and clients can be changed by modifying 'server.config' and 'client.config' respectively.

Source files:
    callback_bridge.h
    config.h
    connection_manager.h
    constants.h
    exception.h
    maekawa.h
    registry.h
    safe_vector.h
    tcp_server.h
    tcp_socket.h
    utils.h
    callback_bridge.cpp
    client.cpp
    config.cpp
    connection_manager.cpp
    exception.cpp
    maekawa.cpp
    registry.cpp
    server.cpp
    tcp_server.cpp
    tcp_socket.cpp
    utils.cpp
    safe_vector.tpp


