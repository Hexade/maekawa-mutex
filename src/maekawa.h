#ifndef MAEKAWA_H
#define MAEKAWA_H

#include "callback_bridge.h"
#include "config.h"
#include "connection_manager.h"

class Maekawa
{
    public:
        static Maekawa& instance(void);
        
        void init(std::vector<TcpConfig>& quorumConfig);

        inline void subscribe_callbacks(CallbackBridge* cb)
        {
            cb_bridge = cb;
        }

    private:
        Maekawa() { }
        ~Maekawa() { }

        static Maekawa instance_;

        CallbackBridge* cb_bridge;        

        ConnectionManager quorum_connections;
};

#endif
