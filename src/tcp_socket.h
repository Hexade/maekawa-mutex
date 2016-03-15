#ifndef TCP_SOCKET
#define TCP_SOCKET

#include "constants.h"
#include "exception.h"

#include <string>
#include <netinet/in.h>

#define MAX_FILE_NAME_LEN 128
#define MAX_WRITE_LEN 512
#define MAX_DATA_LEN 1024
#define HOST_NAME_LEN 50

// command input can be a string or integer
union CommandInput {
    char data[MAX_WRITE_LEN];
    int number;
};

// data struct carried by the socket buffer
struct SockData {
    MSG_TYPE msg_t;
    CMD_TYPE cmd_t;
    char filename[MAX_FILE_NAME_LEN];
    CommandInput input;
};

struct ReplyMessage {
    int server_num;
    bool result;
    char message[MAX_WRITE_LEN];
};

struct WriteMessage {
    int id;
    int seq_num;
    char host_name[HOST_NAME_LEN];

    std::string to_string();
};

union SimpleMessagePayload {
    MAEKAWA_MSG_TYPE maekawa_t;
    WriteMessage write_m;
    ReplyMessage reply_m;
};

struct SimpleMessage {
    SIMPLE_MSG_TYPE msg_t;
    SimpleMessagePayload payload;
};

class TcpSocket
{
    public:
        TcpSocket() {}  //default constructor
        TcpSocket(int port, std::string host = "localhost");
        ~TcpSocket() throw (Exception);
        
        // socket API wrappers
        void connect(void) throw (Exception);
        void bind(void) throw (Exception);
        void listen(int req_q_len) throw (Exception);
        void accept(TcpSocket& client_socket) throw (Exception);
        void send(void* data, int size) throw (Exception);
        void send(std::string) throw (Exception);
        int receive(void* data, int size) throw (Exception);
        void close(void) throw (Exception);

        inline int get_sock_fd() { return sock_fd; }

    private:
        std::string host;
        int port;
        int sock_fd;
        struct hostent* host_entry;
        struct sockaddr_in host_addr;

        void init(bool ifIpAddrAny = false) throw (Exception);
};

#endif
