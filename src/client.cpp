#include "config.h"
#include "constants.h"
#include "tcp_socket.h"
#include "utils.h"

#include <ctime>
#include <cstdlib>
#include <iostream>

using namespace std;

void do_terminate(Config& cfg);

int main(int argc, char* argv[])
{
    // read config
    Config config(CONFIG_FILE);
    try {
        config.create();
    } catch (...) {
        Utils::print_error("unable to read config");
        exit(EXIT_FAILURE);
    }

    // init socket data
    SimpleMessage send_data;
    send_data.msg_t = DATA;
    WriteMessage* wm = &send_data.payload.write_m;
    wm->id = 1;
    wm->seq_num = 1;
    Utils::reset_copy_arr(wm->host_name,
                            argv[1], HOST_NAME_LEN);

    // seed for random number
    srand(time(NULL));

    // select server by random
    int server_num = (rand() % 3) + 1;

    // send request to server
    TcpConfig cfg = config.getTcpConfig(server_num);
    TcpSocket tcp_client(cfg.port, cfg.host);
    try {
        SimpleMessage server_reply;
        ReplyMessage* recv_data = &server_reply.payload.reply_m;
        tcp_client.connect();
        tcp_client.send(&send_data, sizeof(SimpleMessage));
        tcp_client.receive(&server_reply, sizeof(SimpleMessage));
        cout << "Server " << recv_data->server_num
            << " :: " << recv_data->message << endl;
        tcp_client.close();
    } catch (Exception ex) {    
        Utils::print_error(ex.get_message());
    }

    return 0;
}

void do_terminate(Config& config)
{
    SockData sig_term;
    sig_term.msg_t = CLIENT_TO_SERVER;
    sig_term.cmd_t = E_CMD_TERMINATE;

    for (int i = 1; i <= NUM_OF_SERVERS; ++i) {
        TcpConfig cfg = config.getTcpConfig(i);
        TcpSocket tcp_client(cfg.port, cfg.host);
        try {
            ReplyMessage recv_data;
            tcp_client.connect();
            tcp_client.send(&sig_term, sizeof(SockData));
            tcp_client.receive(&recv_data, sizeof(ReplyMessage));
            tcp_client.close();
        } catch (Exception ex) {
            // ignore errorrs;
        }
    }
}

