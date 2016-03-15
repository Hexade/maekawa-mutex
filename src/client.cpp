#include "config.h"
#include "constants.h"
#include "tcp_socket.h"
#include "utils.h"

#include <ctime>
#include <cstdlib>
#include <iostream>
#include <unistd.h>

using namespace std;

static TcpConfig my_conf;
static vector<TcpConfig> other_clients;

void do_terminate(Config& cfg);

int main(int argc, char* argv[])
{
    if (argc < 2) {
        Utils::print_error("usage: ./client <client_number>[1-7]");
        exit(EXIT_FAILURE);
    }

    // read server config
    Config server_config(SERVER_CONFIG_FILE);
    try {
        server_config.create();
    } catch (...) {
        Utils::print_error("unable to read server config");
        exit(EXIT_FAILURE);
    }
           
    // read client config
    Config client_config(CLIENT_CONFIG_FILE);
    try {
        client_config.create();

        int client_num = Utils::str_to_int(argv[1]);
        if (client_num > NUM_OF_CLIENTS)
            throw Exception("Undefined client number");
        for (int i = 1; i <= NUM_OF_CLIENTS; ++i) {
            TcpConfig cfg = client_config.getTcpConfig(i);
            if (i == client_num){
                my_conf = cfg;
                continue;
            }
            other_clients.push_back(cfg);
        }
    } catch (...) {
        Utils::print_error("unable to read client config");
        exit(EXIT_FAILURE);
    }

    // init socket data
    SimpleMessage send_data;
    send_data.msg_t = DATA;
    WriteMessage* wm = &send_data.payload.write_m;
    wm->id = my_conf.number;
    Utils::copy_str_to_arr(my_conf.host, wm->host_name, HOST_NAME_LEN);

    // seed for random number
    srand(time(NULL));

    int seq_num = 0;
    while (true) {
        // sleep for random time
        int sleep_mil = (rand() % 40) + 10;
        // sleep microseconds
        usleep(sleep_mil * 1000);
    
        //set sequence number
        wm->seq_num = seq_num;
        
        // select server by random
        int server_num = (rand() % 3) + 1;
        // send request to server
        TcpConfig cfg = server_config.getTcpConfig(server_num);
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
        seq_num++;
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

