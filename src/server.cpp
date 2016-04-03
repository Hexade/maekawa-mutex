#include "config.h"
#include "connection_manager.h"
#include "constants.h"
#include "registry.h"
#include "tcp_server.h"
#include "utils.h"

#include <cstdlib>
#include <iostream>

using namespace std;

void on_client_data(void* data, TcpSocket* c_sock);
void commit(SimpleMessage* c_data, ReplyMessage* r_msg);
bool test_commit(SimpleMessage* c_data, ReplyMessage* r_msg);
bool test_commit_peers(SimpleMessage* c_data, SimpleMessage& reply_msg);
void commit_peers(SimpleMessage* c_data, SimpleMessage& reply_msg, SIMPLE_MSG_TYPE msg_t);
void do_terminate(void);

static TcpConfig my_conf;
static vector<TcpConfig> other_servers;
static string server_file;
ConnectionManager server_connections;
static int sig_term_count = 0;
static TcpServer* server_ptr;

int main(int argc, char* argv[])
{
    if (argc < 2) {
        Utils::print_error("usage: ./server <server_number>[1-3]");
        exit(EXIT_FAILURE);
    }

    // set server file name
    server_file = "server";
    server_file.append(argv[1]);
    server_file.append(".out");
    Registry::instance().add_file(server_file);

    // read config
    Config config(SERVER_CONFIG_FILE);
    try {
        config.create();

        int server_num = Utils::str_to_int(argv[1]);
        if (server_num > NUM_OF_SERVERS)
            throw Exception("Undefined server number");
        for (int i = 1; i <= NUM_OF_SERVERS; ++i) {
            TcpConfig cfg = config.getTcpConfig(i);
            if (i == server_num){
                my_conf = cfg;
                continue;
            }
            other_servers.push_back(cfg);
        }

    } catch (...) {
        Utils::print_error("unable to read config");
        exit(EXIT_FAILURE);
    }
    server_connections.init(other_servers);

    // init callbacks
    SimpleMessage client_data;
    CallbackBridge client_sock_cb;
    client_sock_cb.subscribe(&on_client_data, &client_data);

    // start TCP server
    TcpServer server(my_conf.port, &client_sock_cb, sizeof(SimpleMessage));
    server_ptr = &server;
    try {
        cout << "Started listening on " << my_conf.host
            << ":" << my_conf.port << endl;
        cout.flush();
        server.run();
    } catch (Exception ex) {
        Utils::print_error("server_main: "
            + string(ex.what()));
        exit(EXIT_FAILURE);
    }
 
    cout << "Bye :)" << endl; 
    return 0;
}

void on_client_data(void* data, TcpSocket* c_sock)
{
    SimpleMessage* c_data = (SimpleMessage*)data;

    // set up server reply
    SimpleMessage reply_msg;
    reply_msg.msg_t = RESULT;
    ReplyMessage* r_msg = &reply_msg.payload.reply_m;

    switch (c_data->msg_t) {

        case DATA:
            if (test_commit(c_data, r_msg)
                && test_commit_peers(c_data, reply_msg)) {
                // all good, commit peers and myself
                commit_peers(c_data, reply_msg,
                    COMMIT);
                commit(c_data, r_msg);
            }
            break;

        case TEST_COMMIT:
            test_commit(c_data, r_msg);
            break;

        case COMMIT:
            commit(c_data, r_msg);
            break;

        case TERMINATE:
            sig_term_count++;
            if (NUM_OF_CLIENTS == sig_term_count) {
                cout << "Preparing to terminate..." << endl;
                do_terminate();
            }
            break;

        case PREP_TERM:
            cout << "Preparing to terminate..." << endl;
            server_connections.close_all();
            break;

        case TERM:
            server_ptr->terminate();
            break;

        default:
            break;
    }  

    r_msg->server_num = my_conf.number;
    c_sock->send(&reply_msg, sizeof(SimpleMessage));
}

void commit(SimpleMessage* c_data, ReplyMessage* r_msg) 
{
    Registry& reg = Registry::instance();

    string result = reg.open_append_close(server_file,
                        c_data->payload.write_m.to_string());

    r_msg->result = true;
    Utils::copy_str_to_arr(result, r_msg->message, MAX_WRITE_LEN);
}

bool test_commit(SimpleMessage* c_data, ReplyMessage* r_msg) 
{
    Registry& reg = Registry::instance();

    string result;
    r_msg->result = reg.test_open_append_close(server_file, result); 
    
    Utils::copy_str_to_arr(result, r_msg->message, MAX_WRITE_LEN);
    return r_msg->result;
}

bool test_commit_peers(SimpleMessage* c_data, SimpleMessage& reply_msg)
{
    // assume success
    ReplyMessage* r_msg = &reply_msg.payload.reply_m;
    r_msg->result = true;

    commit_peers(c_data, reply_msg, TEST_COMMIT);

    return r_msg->result;
}

void commit_peers(SimpleMessage* c_data, SimpleMessage& reply_msg,
    SIMPLE_MSG_TYPE msg_t)
{
    c_data->msg_t = msg_t;
    
    const Connection* conn;
    for (auto& cfg: other_servers) {
        try {
            if (PREP_TERM == msg_t || TERM == msg_t)
                c_data->msg_t = msg_t;
            
            conn = server_connections.get(cfg.number);
            if (!conn)
                continue;
            if (!conn->is_active()) {
                if (!server_connections.connect(cfg.number))
                    throw Exception("connection failed", true);
            }

            conn->send(c_data, sizeof(SimpleMessage));
            conn->receive(&reply_msg, sizeof(SimpleMessage));
        } catch (Exception ex) {
            ReplyMessage* r_msg = &reply_msg.payload.reply_m;
            r_msg->result = false;
            Utils::copy_str_to_arr(FAIL_PEER_COMM, 
                r_msg->message, MAX_WRITE_LEN);
            Utils::print_error(ex.what());
            break;
        }
    }
}

void do_terminate(void)
{
    SimpleMessage msg;
    msg.msg_t = TERMINATE;
    Config client_config(CLIENT_CONFIG_FILE);
    try {
        client_config.create();
        TcpConfig tc = client_config.getTcpConfig(1);
        TcpSocket conn(tc.port, tc.host);
        conn.connect();
        conn.send(&msg, sizeof(SimpleMessage));
        conn.receive(&msg, sizeof(SimpleMessage));
        
        conn.close();
    } catch (Exception ex) {
        Utils::print_error("Error sending terminate signal: " 
            + string(ex.what()));
    }

    // let the peer servers close their client connections
    commit_peers(&msg, msg, PREP_TERM);

    // let peers finish termination
    commit_peers(&msg, msg, TERM);

    // terminate myself
    server_connections.close_all();
    server_ptr->terminate();
}
