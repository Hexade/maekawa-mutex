#include "maekawa.h"

#include "constants.h"

#include <unistd.h>

//TODO: tune this
#define TOKEN_WAIT_SLEEP 5000 // 5 milliseconds

Maekawa Maekawa::instance_;

Maekawa& Maekawa::instance(void)
{
    return instance_;
}

// blocking function: waits for connections
void Maekawa::init(std::vector<TcpConfig>& quorumConfig, int id)
{
    my_id = id;
    add_token_t_safe(my_id);
    // +1 -> include me
    quorum_size = quorumConfig.size() + 1;
    quorum_connections.init(quorumConfig);
    quorum_connections.connect_all();
}

void Maekawa::acquire_lock(void)
{
    request_tokens();

    // add my own request to request
    // to indicate that my token is not available
    // when I am in critical section
    req_queue.push(my_id);
    
    // wait while all required tokens are acquired
    // and my request gets to the top of the queue
    while (token_count_t_safe() != quorum_size ||
            req_queue.front() != my_id) {
        usleep(TOKEN_WAIT_SLEEP);
    }
}

void Maekawa::release_lock(void)
{
    release_tokens();
    
    // remove myself from the request queue
    if (req_queue.front() == my_id)
        req_queue.pop();
    else {
        //TODO: log error
    }
    
    // grant token to the next request (if any)
    if (req_queue.size() > 0) {
       send(req_queue.front(), REPLY);
    }
}

void Maekawa::process_message(SimpleMessage* message)
{
    MaekawaMessage* mm = &message->payload.maekawa_m;
    
    switch (mm->maekawa_t) {
        case REQUEST:
            process_request(mm);
            break;

        case REPLY:
            add_token_t_safe(mm->id);
            mm->maekawa_t = NONE;
            break;

        case RELEASE:
            process_release(mm);
            break;

        default:
            break;
    }
}

void Maekawa::request_tokens(void)
{
    broadcast(REQUEST);
}

void Maekawa::release_tokens(void)
{
    // release other tokens
    // keep my token until requested by others
    remove_all_tokens_except(my_id);

    broadcast(RELEASE);
}

void Maekawa::process_request(MaekawaMessage* mm)
{
    req_queue.push(mm->id);

    // wait till my gets to the top of the queue
    // TODO: No waiting when handling deadlock
    while (req_queue.front() != mm->id ||
            !token_available_t_safe()) {
        usleep(TOKEN_WAIT_SLEEP);
    }

    // lease my own token to the requesting process
    remove_token_t_safe(my_id);

    //TODO: Change when implementing deadlock handling
    mm->maekawa_t = REPLY;
}

void Maekawa::process_release(MaekawaMessage* mm)
{
    // remove request from the request queue
    if (req_queue.front() == mm->id)
        req_queue.pop();
    else {
        //TODO: log error
    }
    
    // grant token to the next request (if any)
    if (req_queue.front() != my_id && 
        req_queue.size() > 0) {
       send(req_queue.front(), REPLY);
    } else {
        // if my own request or no pending requests
        tokens.push_back(my_id);
    }
    mm->maekawa_t = NONE; 
}

// broadcasts messages to all the peers of the quorum
void Maekawa::broadcast(MAEKAWA_MSG_TYPE message_type)
{
    const std::vector<Connection>& connections = 
                quorum_connections.get_all();

    SimpleMessage message;
    message.msg_t = MAEKAWA;
    MaekawaMessage* mm = &message.payload.maekawa_m;
    mm->id = my_id; 

    for (const auto& conn: connections) {
        mm->maekawa_t = message_type;

        conn.send(&message, sizeof(SimpleMessage));
        conn.receive(&message, sizeof(SimpleMessage));

        if (REPLY == mm->maekawa_t)
            tokens.push_back(mm->id);
    }
}

// sends message to a specific peer of the quorum
void Maekawa::send(int id, MAEKAWA_MSG_TYPE message_type)
{    
    SimpleMessage message;
    message.msg_t = MAEKAWA;
    MaekawaMessage* mm = &message.payload.maekawa_m;
    mm->id = my_id;
    mm->maekawa_t = message_type;

    const Connection* conn = quorum_connections.get(id);
    conn->send(&message, sizeof(SimpleMessage));
    conn->receive(&message, sizeof(SimpleMessage));
}

bool Maekawa::token_available_t_safe(void)
{
    std::lock_guard<std::mutex> lock(token_mutex);
    for (const auto& token: tokens) {
        if (token == my_id) {
            return true;
        }
    }
    return false;
}

void Maekawa::add_token_t_safe(int id)
{
    std::lock_guard<std::mutex> lock(token_mutex);
    tokens.push_back(id);
}

void Maekawa::remove_token_t_safe(int id)
{
    std::lock_guard<std::mutex> lock(token_mutex);
    for (std::vector<int>::iterator it = tokens.begin();
            it != tokens.end();) {
        if (*it == id) {
            it = tokens.erase(it);
        } else
            ++it;
    }
}

unsigned int Maekawa::token_count_t_safe(void)
{
    std::lock_guard<std::mutex> lock(token_mutex);
    return tokens.size();
}

void Maekawa::remove_all_tokens_except(int id)
{
    std::lock_guard<std::mutex> lock(token_mutex);
    tokens.clear();
    tokens.push_back(my_id);
}

void Maekawa::push_request_t_safe(int id)
{
    std::lock_guard<std::mutex> lock(req_q_mutex);
    req_queue.push(id);
}

void Maekawa::pop_request_t_safe(void)
{
    std::lock_guard<std::mutex> lock(req_q_mutex);
    req_queue.pop();
}

int Maekawa::front_request_t_safe(void)
{
    std::lock_guard<std::mutex> lock(req_q_mutex);
    return req_queue.front();
}
