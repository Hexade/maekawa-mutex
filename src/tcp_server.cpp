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
    main_socket.listen();

    // accept
    int client_len;
    while (true) {
        TcpSocket client_socket;
        main_socket.accept(client_socket, client_len);
    
        client_socket.receive(client_data_cb->sock_data, sizeof(SockData));
        // callback to indicate client data is available
        client_data_cb->on_data_read(&client_socket);
    }
}

