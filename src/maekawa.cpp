#include "maekawa.h"

Maekawa Maekawa::instance_;

Maekawa& Maekawa::instance(void)
{
    return instance_;
}

// bloking function: waits for connections
void Maekawa::init(std::vector<TcpConfig>& quorumConfig)
{
    quorum_connections.init(quorumConfig);
    quorum_connections.connect_all();
}
