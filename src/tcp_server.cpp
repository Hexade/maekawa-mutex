#include "tcp_server.h"

TcpServer::~TcpServer() throw (Exception)
{
    main_socket.close();
}

void TcpServer::start(void) throw (Exception)
{
    // bind
    main_socket.bind();
    
    // listen
    main_socket.listen(REQ_QUEUE_SZ);

    // init active sockets with main socket to
    // accept new connections
    FD_ZERO(&active_fd_set);
    FD_SET(main_socket.get_sock_fd(), &active_fd_set);
    max_fd = main_socket.get_sock_fd();

    while (true) {
        read_fd_set = active_fd_set;

        // wait for an input in one of the active sockets
        if (select(max_fd + 1, &read_fd_set, NULL, NULL, NULL) < 0) {
            throw Exception("unable to select on active sockets", true);
        }

        // check for data on existing connections
        for (std::vector<TcpSocket>::iterator it = client_sockets.begin();
                it != client_sockets.end();) {
            TcpSocket* socket = &(*it);

            // if client socket fd is not set, data is not there on that socket
            if (!FD_ISSET(socket->get_sock_fd(), &read_fd_set))
                continue;
        
            int bytes_read = socket->receive(client_data_cb->sock_data, client_data_len);
            if (bytes_read == 0) {
                // connection closed by the client. Free the socket
                socket->close();
                FD_CLR(socket->get_sock_fd(), &active_fd_set);
                // erase returns a new iterotor to the next element
                it = client_sockets.erase(it);
            } else {
                // callback to indicate client data is available
                client_data_cb->on_data_read(socket);
                ++it;
            }
        }

        // check for new client connections on the main socket
        if (FD_ISSET(main_socket.get_sock_fd(), &read_fd_set)) {
            // create a new client connection
            TcpSocket new_socket;
            main_socket.accept(new_socket);
            int fd = new_socket.get_sock_fd();
            if (fd > max_fd)
                max_fd = fd;
            
            FD_SET(fd, &active_fd_set);
            client_sockets.push_back(new_socket);
        }
    }
}

