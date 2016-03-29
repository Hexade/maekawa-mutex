#ifndef MAEKAWA_H
#define MAEKAWA_H

#include "callback_bridge.h"
#include "config.h"
#include "connection_manager.h"

#include <atomic>
#include <mutex>
#include <queue>

class Maekawa
{
    public:
        static Maekawa& instance(void);
        
        void init(std::vector<TcpConfig>& quorumConfig, int id);

        void acquire_lock(void);
        void release_lock(void);

        void process_message(SimpleMessage* message);

        inline void subscribe_callbacks(CallbackBridge* cb)
        {
            cb_bridge = cb;
        }

    private:
        Maekawa() { }
        ~Maekawa() { }

        void request_tokens(void);
        void release_tokens(void);
        void process_request(MaekawaMessage* mm);
        void process_release(MaekawaMessage* mm);
        void broadcast(MAEKAWA_MSG_TYPE message_type);
        void send(int id, MAEKAWA_MSG_TYPE message_type);
        bool token_available(void);

        // thread safe container wrappers
        bool token_available_t_safe(void);
        void add_token_t_safe(int id);
        void remove_token_t_safe(int id);
        unsigned int token_count_t_safe(void);
        void remove_all_tokens_except(int id);

        void push_request_t_safe(int id);
        void pop_request_t_safe(void);
        int front_request_t_safe(void);

        static Maekawa instance_;

        int my_id;
        std::vector<int> tokens;

        // request queue
        std::queue<int> req_queue;

        // mutex
        std::mutex token_mutex;
        std::mutex req_q_mutex;

        CallbackBridge* cb_bridge; 

        ConnectionManager quorum_connections;
        unsigned int quorum_size;
        
};

#endif
