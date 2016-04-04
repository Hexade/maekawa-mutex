#ifndef MAEKAWA_H
#define MAEKAWA_H

#include "callback_bridge.h"
#include "config.h"
#include "connection_manager.h"
#include "safe_vector.h"

#include <atomic>
#include <mutex>
#include <deque>

class Maekawa
{
    public:
        static Maekawa& instance(void);
        
        void init(Config* c, int id);
        inline void close()
        {
            connections.close_all();
        }

        void acquire_lock(void);
        void release_lock(void);

        void broadcast_all(SIMPLE_MSG_TYPE type);
        void process_message(SimpleMessage* message);

        inline void subscribe_callbacks(CallbackBridge* cb)
        {
            cb_bridge = cb;
        }

        inline int* get_session_sends(void)
        {
            return session_sends;
        }

        inline int* get_session_recvs(void)
        {
            return session_recvs;
        }

        inline int get_sent_message_count(void)
        { return sent_messages; }

        inline int get_recv_message_count(void)
        { return recv_messages; }

    private:
        Maekawa() { }
        ~Maekawa() { }

        void request_tokens(void);
        void release_tokens(void);
        
        void process_request(MaekawaMessage* mm);
        void process_reply(MaekawaMessage* mm);
        void process_release(MaekawaMessage* mm);
        void process_fail(int id);
        void process_enquire(MaekawaMessage* mm);
        void process_yield(MaekawaMessage* mm);
        
        void broadcast(MAEKAWA_MSG_TYPE message_type);
        void send(int id, SimpleMessage& message_type);
        
        bool token_available(void);

        // thread_safe queue operations
        void push_request_t_safe(int id);
        void pop_request_t_safe(void);
        int get_front_request_t_safe(void);
        unsigned int request_count_t_safe(void);
        void last_request_front_t_safe(void);
        bool second_req_front_t_safe(void);
        void insert_request_t_safe(int id, int startIndex);
        void print_requests_t_safe(void);

        void print_state(void);

        static Maekawa instance_;

        Config* config;
        int my_id;
        int my_token_holder;

        // available tokens
        SafeVector<int> tokens;
        // request queue
        std::deque<int> requests;
        // fail messages received; yet to receive a reply
        SafeVector<int> fails;
        // yields sent; yet to receive a reply
        SafeVector<int> yields;
        // fails sent or yields received;
        // yet to send a reply
        SafeVector<int> deferred;

        // mutex
        std::mutex requests_mutex;

        CallbackBridge* cb_bridge; 

        ConnectionManager connections;
        unsigned int quorum_size;

        std::vector<int> quorum_peers;

        // stats
        int sent_messages;
        int recv_messages;
        int session_sends[NONE] = {0};
        int session_recvs[NONE] = {0};
};

#endif
