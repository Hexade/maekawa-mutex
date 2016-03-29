#include "config.h"
#include "connection_manager.h"
#include "constants.h"
#include "maekawa.h"
#include "callback_bridge.h"
#include "tcp_server.h"
#include "tcp_socket.h"
#include "utils.h"

#include <unistd.h>

#include <ctime>
#include <cstdlib>
#include <iostream>
#include <thread>

#define NUM_OF_WRITES 40

using namespace std;

static TcpConfig my_conf;
static vector<TcpConfig> quorum_peers;

bool configure_quorum(int client_num);
void run_server(int port);
void do_terminate(Config& cfg);
void on_client_data(void* data, TcpSocket* c_sock);
void on_client_data_async(void* data, TcpSocket* c_sock);

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
 
    // configure quorum
    if (!configure_quorum(Utils::str_to_int(argv[1]))) 
        exit(EXIT_FAILURE);

    // spawn server thread 
    thread server_thread(run_server, my_conf.port);

    // setup connections with quorum peers
    Maekawa::instance().init(quorum_peers, my_conf.number);
    cout << "INFO: quorum initialized" << endl;

    // init socket data
    SimpleMessage send_data;
    send_data.msg_t = DATA;
    WriteMessage* wm = &send_data.payload.write_m;
    wm->id = my_conf.number;
    Utils::copy_str_to_arr(my_conf.host, wm->host_name, HOST_NAME_LEN);

    // create server connections
    ConnectionManager server_connections(server_config.get_all());
    server_connections.connect_all();
    cout << "INFO: Connection established with all the servers" << endl;

    // seed for random number
    srand(time(NULL));

    int seq_num = 0;
    while (seq_num < NUM_OF_WRITES) {
        // sleep for random time
        int sleep_mil = (rand() % 40) + 10;
        // sleep microseconds
        usleep(sleep_mil * 1000);
    
        //set sequence number
        wm->seq_num = seq_num;
        
        // select server by random
        int server_num = (rand() % 3) + 1;

        // send request to server
        const Connection* conn = server_connections.get(server_num);
        try {
            SimpleMessage server_reply;
            ReplyMessage* recv_data = &server_reply.payload.reply_m;
            conn->send(&send_data, sizeof(SimpleMessage));
            conn->receive(&server_reply, sizeof(SimpleMessage));
            cout << "Server " << recv_data->server_num
                << " :: " << recv_data->message << endl;
        } catch (Exception ex) {    
            Utils::print_error(ex.what());
        }
        seq_num++;
    }

    server_connections.close_all();
    server_thread.join();

    return 0;
}

bool configure_quorum(int client_num)
{
    bool success = true;
    Config client_config(CLIENT_CONFIG_FILE);
    try {
        client_config.create();

        if (client_num > NUM_OF_CLIENTS)
            throw Exception("Undefined client number");        
        my_conf = client_config.getTcpConfig(client_num);

        vector<int> quorum_peer_nums = Utils::get_quorum_peer_nums(
                                QUORUM_CONFIG_FILE, client_num);
        for (auto& peer_id: quorum_peer_nums) {
            quorum_peers.push_back(client_config.getTcpConfig(peer_id));
        }
    } catch (...) {
        Utils::print_error("unable to configure quorum");
        success = false;
    }
   
    return success;
}

void run_server(int port)
{
    // init callbacks
    SimpleMessage client_data;
    CallbackBridge client_sock_cb;
    client_sock_cb.subscribe(&on_client_data, &client_data);

    // start TCP server
    TcpServer server(port, &client_sock_cb, sizeof(SimpleMessage));
    try {
        cout << "Started listening on port " << port << endl;
        cout.flush();
        server.start();
    } catch (Exception ex) {
        Utils::print_error("server_main: "
            + string(ex.what()));
        exit(EXIT_FAILURE);
    }    
}

void on_client_data(void* data, TcpSocket* c_sock)
{
    thread t_proc_msg(on_client_data_async, data, c_sock);
}

void on_client_data_async(void* data, TcpSocket* c_sock)
{
    SimpleMessage* s_message = (SimpleMessage*)data;
    if (MAEKAWA == s_message->msg_t)
        Maekawa::instance().process_message(s_message);
    c_sock->send(data, sizeof(SimpleMessage));
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

