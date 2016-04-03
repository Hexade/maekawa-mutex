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

void run_server(int port);
void on_client_data(void* data, TcpSocket* c_sock);
void on_client_data_async(SimpleMessage s_message, TcpSocket* c_sock);
static TcpServer* server_ptr;
static bool do_terminate = false;

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
    TcpConfig my_conf;
    Config client_config(CLIENT_CONFIG_FILE);
    int client_num = Utils::str_to_int(argv[1]);
    try {
        client_config.create();

        if (client_num > NUM_OF_CLIENTS)
            throw Exception("Undefined client number");        
        my_conf = client_config.getTcpConfig(client_num);

    } catch (...) {
        Utils::print_error("unable to configure quorum");
        exit(EXIT_FAILURE);
    }

    // spawn server thread 
    thread server_thread(run_server, my_conf.port);

    // setup connections with quorum peers
    Maekawa::instance().init(&client_config, my_conf.number);
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

            Maekawa::instance().acquire_lock();
            conn->send(&send_data, sizeof(SimpleMessage));
            conn->receive(&server_reply, sizeof(SimpleMessage));
            Maekawa::instance().release_lock();

            cout << "Server " << recv_data->server_num
                << " :: " << recv_data->message << endl;
        } catch (Exception ex) {    
            Utils::print_error(ex.what());
        }
        seq_num++;
    }

    cout << "Preparing to terminate..." << endl;
 
    // send completion status
    const Connection* conn = server_connections.get(1);
    send_data.msg_t = TERMINATE;
    try {
        Maekawa::instance().acquire_lock();
        conn->send(&send_data, sizeof(SimpleMessage));
        conn->receive(&send_data, sizeof(SimpleMessage));
        Maekawa::instance().release_lock();
    } catch (Exception ex) {    
        // igonre exception; Utils::print_error(ex.what());
    }

    while (!do_terminate) {
        usleep(10000);  // 10 milliseconds
    }
    
    server_connections.close_all();
    server_thread.join();

    cout << "Bye :)" << endl;
    return 0;
}

void run_server(int port)
{
    // init callbacks
    SimpleMessage client_data;
    CallbackBridge client_sock_cb;
    client_sock_cb.subscribe(&on_client_data, &client_data);

    // start TCP server
    TcpServer server(port, &client_sock_cb, sizeof(SimpleMessage));
    server_ptr = &server;
    try {
        cout << "Started listening on port " << port << endl;
        cout.flush();
        server.run();
    } catch (Exception ex) {
        Utils::print_error("server_main: "
            + string(ex.what()));
        exit(EXIT_FAILURE);
    }
}

void on_client_data(void* data, TcpSocket* c_sock)
{
    SimpleMessage s_message = *((SimpleMessage*)data);
    //thread (on_client_data_async, message, c_sock).detach();

    switch (s_message.msg_t) {
        case MAEKAWA:
            Maekawa::instance().process_message(&s_message);
            break;

        case TERMINATE:
            Maekawa::instance().broadcast_all(PREP_TERM);
            Maekawa::instance().broadcast_all(TERM);
            Maekawa::instance().close();
            do_terminate = true;
            server_ptr->terminate();
            break;

        case PREP_TERM:
            Maekawa::instance().close();
            break;

        case TERM:
            do_terminate = true;
            server_ptr->terminate();
            break;

        default:
            break;
    }
   
    c_sock->send(&s_message, sizeof(SimpleMessage));
}

// deprecated: asynchronous poses problems while using deadlock avoidance
void on_client_data_async(SimpleMessage s_message, TcpSocket* c_sock)
{
    if (MAEKAWA == s_message.msg_t)
        Maekawa::instance().process_message(&s_message);
    c_sock->send(&s_message, sizeof(SimpleMessage));
}

