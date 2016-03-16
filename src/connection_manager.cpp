#include "connection_manager.h"

ConnectionManager::ConnectionManager(std::vector<TcpConfig>& cfgs)
            : configs(cfgs)
{
    for (auto& cfg: cfgs) {
       connections.emplace_back(cfg.number, cfg.port, cfg.host);
    }
}

void ConnectionManager::connect_all(void)
{
    for (auto& conn: connections) {
        conn.connect();
    }
}

bool ConnectionManager::connect(int id)
{
    for (auto& conn: connections) {
        if (conn.getId() == id) {
            conn.connect();
            return true;
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
