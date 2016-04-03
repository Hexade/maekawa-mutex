#include "maekawa.h"

#include "constants.h"
#include "utils.h"

#include <unistd.h>

#include <iostream>

//TODO: tune this
#define TOKEN_WAIT_SLEEP 5000 // 5 milliseconds

Maekawa Maekawa::instance_;

Maekawa& Maekawa::instance(void)
{
    return instance_;
}

// blocking function: waits for connections
void Maekawa::init(Config* c, int id)
{
    config = c;
    my_id = id;

    tokens.push_back(my_id);
    my_token_holder = my_id;

    // configure quorum
    std::vector<TcpConfig> quorum;
    quorum_peers = Utils::get_quorum_peer_nums(
                        QUORUM_CONFIG_FILE, my_id);
    for (auto& peer_id: quorum_peers) {
        quorum.push_back(config->getTcpConfig(peer_id));
    }

    // +1 -> include me
    quorum_size = quorum_peers.size() + 1;
    connections.init(c->get_all());
    connections.connect_all();
}

void Maekawa::acquire_lock(void)
{
    // add my own request to request
    // to indicate that my token is not available
    // when I am in critical section
    push_request_t_safe(my_id);
 
    // wait till my request reaches the top of the queue
    while (get_front_request_t_safe() != my_id)
        usleep(TOKEN_WAIT_SLEEP);

    request_tokens();
    
    // wait while all required tokens are acquired
    while (tokens.size() != quorum_size)
        usleep(TOKEN_WAIT_SLEEP);
}

void Maekawa::release_lock(void)
{
    // remove myself from the request queue
    if (get_front_request_t_safe() == my_id)
        pop_request_t_safe();
    else {
        //TODO: log error
    }
    
    // release peers
    release_tokens();
    
    // grant token to the next request
    // and only if not already granted
    if (request_count_t_safe() > 0 &&
            deferred.contains(get_front_request_t_safe())) {

        int id = get_front_request_t_safe();
        my_token_holder = id;
        tokens.remove(my_id);
        deferred.remove(id);

        SimpleMessage message;
        message.payload.maekawa_m.maekawa_t = REPLY;
        send(get_front_request_t_safe(), message);
        print_state();
    }
}

// broadcast to not just quorum but all
void Maekawa::broadcast_all(SIMPLE_MSG_TYPE type)
{
    const std::vector<Connection>& conns = connections.get_all();

    SimpleMessage msg;
    msg.msg_t = type;
    for (auto& conn: conns) {
        if (conn.getId() == my_id)
            continue;

        conn.send(&msg, sizeof(SimpleMessage));
        conn.receive(&msg, sizeof(SimpleMessage));
    }
}

void Maekawa::process_message(SimpleMessage* message)
{
    connections.wait_for_connections();

    MaekawaMessage* mm = &message->payload.maekawa_m;
    int id = mm->id;
    Utils::log_message(id, my_id, mm->maekawa_t, RECV);
    print_state();
    
    switch (mm->maekawa_t) {
        case REQUEST:
            process_request(mm);
            break;

        case REPLY:
            process_reply(mm);
            break;

        case RELEASE:
            process_release(mm);
            break;

        case ENQUIRE:
            process_enquire(mm);
            break;

        default:
            break;
    }
    Utils::log_message(my_id, id, mm->maekawa_t, SENT);
    print_state();
}

void Maekawa::request_tokens(void)
{
    broadcast(REQUEST);
}

void Maekawa::release_tokens(void)
{
    // release other tokens
    // keep my token until requested by others
    tokens.clear();
    tokens.push_back(my_id);

    broadcast(RELEASE);
}

void Maekawa::process_request(MaekawaMessage* mm)
{
    push_request_t_safe(mm->id);

    const int front_req = get_front_request_t_safe();
    if (front_req == mm->id) {
        // token is available
        // lease my own token to the requesting process
        tokens.remove(my_id);
        my_token_holder = mm->id;
        
        mm->maekawa_t = REPLY;
    } else if (front_req > mm->id) {
        // requesting process has a lower priority
        mm->maekawa_t = FAIL;
        deferred.push_back(mm->id);
        insert_request_t_safe(mm->id, 1);
    } else {
        // requesting process has a higher priority
        if (front_req == my_id) {
            //if (fails.size() != 0 || yields.size() != 0) {
            if (tokens.size() < quorum_size) {
                tokens.remove(my_id);
                // perform similer to when you receive a yield
                process_yield(mm);
                insert_request_t_safe(my_id, 1);
            }
            else {
                mm->maekawa_t = FAIL;
                deferred.push_back(mm->id);
                insert_request_t_safe(mm->id, 1);
            }
        }
        else {
            if (front_req == my_token_holder) {
                SimpleMessage message;
                message.payload.maekawa_m.maekawa_t = ENQUIRE;
                // introducing delay to solve REPLY - ENQUIRE FIFO problem
                // TODO: need to come up with a concrete solution
                usleep(TOKEN_WAIT_SLEEP);
                send(front_req, message);
            
                if (YIELD == message.payload.maekawa_m.maekawa_t) {
                    deferred.push_back(message.payload.maekawa_m.id);
                    process_yield(mm);
                    insert_request_t_safe(message.payload.maekawa_m.id, 1);
                }
                else {
                    mm->maekawa_t = FAIL;
                    deferred.push_back(mm->id);
                    insert_request_t_safe(mm->id, 1);
                }
            } else {
                mm->maekawa_t = FAIL;
                deferred.push_back(mm->id);
                insert_request_t_safe(mm->id, 0);
            }
            print_state();
        }
    }
    mm->id = my_id;
}

void Maekawa::process_reply(MaekawaMessage* mm)
{
    tokens.push_back(mm->id);
    yields.remove(mm->id);  // if any
    fails.remove(mm->id);   // if any
    mm->maekawa_t = NONE;
    mm->id = my_id;
}

void Maekawa::process_release(MaekawaMessage* mm)
{ 
    tokens.push_back(my_id);
    my_token_holder = my_id;

    // remove request from the request queue
    if (get_front_request_t_safe() == mm->id) {
        pop_request_t_safe();
    }
    else {
        //TODO: log error
    }

    if (request_count_t_safe() > 0) {
        const int front_request = get_front_request_t_safe();
        if (front_request != my_id &&
                deferred.contains(front_request)) {
       
            tokens.remove(my_id);
            my_token_holder = front_request;
            deferred.remove(front_request);


            SimpleMessage message;
            message.payload.maekawa_m.maekawa_t = REPLY;
            send(front_request, message);
            print_state();
        } else {
            // if my own request or no pending requests
        }
    }
    mm->maekawa_t = NONE;
    mm->id = my_id;
}

void Maekawa::process_fail(int id)
{
    fails.push_back(id);
}

void Maekawa::process_enquire(MaekawaMessage* mm)
{
    //if (fails.size() != 0 || yields.size() != 0) {
    if (tokens.contains(mm->id) 
            && tokens.size() < quorum_size) {
        mm->maekawa_t = YIELD;
        tokens.remove(mm->id);
        yields.push_back(mm->id);
    }
    else
        // cannot yield at this point
        mm->maekawa_t = NONE;
    mm->id = my_id;
}

void Maekawa::process_yield(MaekawaMessage* mm)
{
    // put the process yielding to on top
    //last_request_front_t_safe();
    insert_request_t_safe(mm->id, 0);
    
    my_token_holder = get_front_request_t_safe();
    mm->maekawa_t = REPLY;
}

// broadcasts messages to all the peers of the quorum
void Maekawa::broadcast(MAEKAWA_MSG_TYPE message_type)
{
    SimpleMessage message;
    message.msg_t = MAEKAWA;
    MaekawaMessage* mm = &message.payload.maekawa_m;

    for (const auto& peer: quorum_peers) {
        const Connection* conn = connections.get(peer);
        mm->maekawa_t = message_type;
        mm->id = my_id; 

        conn->send(&message, sizeof(SimpleMessage));
        Utils::log_message(my_id, peer, mm->maekawa_t, SENT);
        print_state();
        
        conn->receive(&message, sizeof(SimpleMessage));
        Utils::log_message(peer, my_id, mm->maekawa_t, RECV);

        // handle replies
        if (REPLY == mm->maekawa_t)
            process_reply(mm);
        else if (FAIL == mm->maekawa_t)
            process_fail(mm->id);

        print_state();
    }
}

// sends message to a specific peer of the quorum
void Maekawa::send(int id, SimpleMessage& message)
{    
    message.msg_t = MAEKAWA;
    MaekawaMessage* mm = &message.payload.maekawa_m;
    mm->id = my_id;
/*
    // create a new connection if not connected
    if (NULL == connections.get(id)) {
        connections.add(config->getTcpConfig(id));
        connections.connect(id);
    }
*/
    const Connection* conn = connections.get(id);
    conn->send(&message, sizeof(SimpleMessage));
    Utils::log_message(my_id, id, mm->maekawa_t, SENT);
    print_state();

    conn->receive(&message, sizeof(SimpleMessage));
    Utils::log_message(mm->id, my_id, mm->maekawa_t, RECV);
}

bool Maekawa::token_available(void)
{
    return my_token_holder == my_id;
}

void Maekawa::push_request_t_safe(int id)
{
    std::lock_guard<std::mutex> lock(requests_mutex);
    requests.push_back(id);
}

void Maekawa::pop_request_t_safe(void)
{
    std::lock_guard<std::mutex> lock(requests_mutex);
    requests.pop_front();
}

int Maekawa::get_front_request_t_safe(void)
{
    std::lock_guard<std::mutex> lock(requests_mutex);
    return requests.front();
}

unsigned int Maekawa::request_count_t_safe(void)
{
    std::lock_guard<std::mutex> lock(requests_mutex);
    return requests.size();
}

void Maekawa::last_request_front_t_safe(void)
{
    std::lock_guard<std::mutex> lock(requests_mutex);
    requests.push_front(requests.back());
    requests.pop_back();
}

void Maekawa::insert_request_t_safe(int id, int startIndex)
{    
    std::lock_guard<std::mutex> lock(requests_mutex);
    // remove
    for (std::deque<int>::iterator it = requests.begin();
            it != requests.end();) {
        if (*it == id) {
            it = requests.erase(it);
        } else
            ++it;
    }

    // insert at appropriate location
    for (std::deque<int>::iterator it = requests.begin()
            + startIndex; it != requests.end(); ++it) {
        if (*it < id) {
            requests.insert(it, id);
            return;
        }
    }
    requests.push_back(id);
}

void Maekawa::print_requests_t_safe(void)
{
    std::lock_guard<std::mutex> lock(requests_mutex);
    Utils::print_deque("requests", my_id, requests); 
}

void Maekawa::print_state(void)
{
    tokens.print("tokens", my_id);
    fails.print("fails", my_id);
    yields.print("yields", my_id);
    deferred.print("deferred", my_id);
    print_requests_t_safe();
}
