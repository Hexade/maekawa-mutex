#include "tcp_socket.h"

#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <sstream>

std::string WriteMessage::to_string()
{
    std::stringstream ss;

    ss << "< " << id << ", " << seq_num << ", " << host_name << " >";
    
    return ss.str();;
}

TcpSocket::TcpSocket(int port, std::string host)
: host(host), port(port)
{ }

// destructor: close all active socket descriptors
TcpSocket::~TcpSocket() throw (Exception)
{
    if (sock_fd > -1)
        ::close(sock_fd);
}

// initialize socket descriptor and host address
void TcpSocket::init(bool isIpAddrAny) throw (Exception)
{
    // create socket
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0)
        throw Exception("unable to create socket", true);

    // build host's internet address
    memset(&host_addr, 0, sizeof(host_addr));
    if (isIpAddrAny) {
        host_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    } else {
        // get host's DNS entry
        host_entry = gethostbyname(host.c_str());
        if (!host_entry)
            throw Exception("no such host " + host, true);
        
        memcpy((char *)&host_addr.sin_addr.s_addr, host_entry->h_addr,
            host_entry->h_length);
    }

    host_addr.sin_port = htons(port);
    host_addr.sin_family = AF_INET;
}

// connect to a host. 
// used by client modules.
void TcpSocket::connect(void) throw (Exception)
{
    // initialize socket
    init();

    int success = ::connect(sock_fd, (struct sockaddr *)&host_addr,
        sizeof(host_addr));
    if (success < 0)
        throw Exception("unable to connect to host", true);
}

// bind the socket to an ip address
// used by server modules
void TcpSocket::bind(void) throw (Exception)
{
    // initialize socket with INADDR_ANY
    init(true);

    int success = ::bind(sock_fd, (struct sockaddr *)&host_addr,
        sizeof(host_addr));
    if (success < 0)
        throw Exception("unable to bind to host address", true);
}

// listen for requests on the specified ports
void TcpSocket::listen(void) throw (Exception)
{
    int success = ::listen(sock_fd, REQ_QUEUE_SZ);
    if (success < 0)
        throw Exception("could not initiate listen", true);
}

// accept requests from client and create client socket
void TcpSocket::accept(TcpSocket& client_socket, int& client_socket_len)
    throw (Exception)
{
    socklen_t client_len = sizeof(client_socket.host_addr);
    client_socket.sock_fd = ::accept(sock_fd, 
        (struct sockaddr *)&client_socket.host_addr,
        &client_len);
    if (client_socket.sock_fd < 0)
        throw Exception("could not accept connection request", true);
    client_socket_len = (int) client_len;
}

// send/write string data to socket
void TcpSocket::send(std::string data) throw (Exception)
{
    // extra space for '\0'
    const int len = data.size() + 1;
    char data_a[len];
    memcpy(data_a, data.c_str(), data.size());
    // null termination
    data_a[len-1] = 0;
    send(data_a, len);
}

// send/write data to socket
void TcpSocket::send(void* data, int size) throw (Exception)
{
    int bytes_written = write(sock_fd, data, size);
    if (bytes_written < 0)
        throw Exception("cannot send data", true);
}

// receive/read data from socket
void TcpSocket::receive(void* data, int size) throw (Exception)
{
    int bytes_read = read(sock_fd, data, size);
    if (bytes_read < 0)
        throw Exception("cannot receive data", true);    
}

// close socket descriptor
void TcpSocket::close(void) throw (Exception)
{
    ::close(sock_fd);
}
