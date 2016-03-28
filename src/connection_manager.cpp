#include "connection_manager.h"

#define CONN_WAIT_TIMEOUT 10000 // 10 milliseconds

#include <unistd.h>

ConnectionManager::ConnectionManager(std::vector<TcpConfig>& cfgs)
{
    for (auto& cfg: cfgs) {
       connections.emplace_back(cfg.number, cfg.port, cfg.host);
    }
}

void ConnectionManager::init(std::vector<TcpConfig>& cfgs)
{
    for (auto& cfg: cfgs) {
       connections.emplace_back(cfg.number, cfg.port, cfg.host);
    }
}

//blocking function; waits till all the connections are successfull
void ConnectionManager::connect_all(void)
{
    for (auto& conn: connections) {
        while (!conn.connect()) {
            usleep(CONN_WAIT_TIMEOUT);
        }
    }
}

bool ConnectionManager::connect(int id)
{
    for (auto& conn: connections) {
        if (conn.getId() == id) {
            return conn.connect();
        }
    }
    return false;
}

void ConnectionManager::close_all(void)
{
    for (auto& conn: connections) {
        conn.close();
    }
}

const Connection* ConnectionManager::get(int id)
{
    for (auto& conn: connections) {
        if (conn.getId() == id)
            return &conn;
    }
    return NULL;
}
