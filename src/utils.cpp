#include "utils.h"

#include "constants.h"

#include <fstream>
#include <iterator>
#include <mutex>
#include <sstream>
#include <string.h>

std::mutex Utils::file_mutex;

void Utils::print_error(std::string msg, bool use_errno)
{
    if (use_errno)
        msg.append(strerror(errno));
    std::cerr << "error: " << msg << std::endl;
}

int Utils::str_to_int(std::string val_str)
    throw (Exception)
{
    char first_char = val_str.at(0);
    // first character must be a digit or '-' or '+'
    if (val_str.empty() ||
        (!isdigit(first_char) &&
        first_char != '-' &&
        first_char != '+'))
        throw Exception("Not a number");
    
    char* str_end;
    int val = strtol(val_str.c_str(), &str_end, 10);
    // make sure the string is not of the type '3awd' or '324m'
    if (*str_end != 0)
        throw Exception("Not a number");
        
    return val;
}

std::vector<std::string> Utils::split_str(std::string str,
    std::string delim)
{
    std::istringstream iss(str);
    std::vector<std::string> tokens;
    copy(std::istream_iterator<std::string>(iss),
        std::istream_iterator<std::string>(),
        back_inserter(tokens));
    return tokens;
}

void Utils::copy_str_to_arr(std::string str, char* arr,
    int len)
{
    if (!arr)
        throw Exception("Utils::copy_str_to_arr: undefined array");

    memset(arr, 0, len);

    if (str.size() < (unsigned int)len)
        len = str.size();
    memcpy(arr, str.c_str(), len);    
}

void Utils::reset_copy_arr(void* dest, void* src, int len)
{    
    if (!src || !dest)
        throw Exception("Utils::copy_str_to_arr: undefined array");

    memset(dest, 0, len);
    memcpy(dest, src, len);
}

std::vector<int> Utils::get_quorum_peer_nums(std::string filename, int client_num)
{
    std::ifstream ifs(filename.c_str());
    if (!ifs.good())
        throw Exception("Utils::get_quorum: file not found");

    std::vector<int> quorum_peers;
    std::string line;
    int line_num = 1;
    // skip n-1 lines
    while (line_num < client_num) {
        std::getline(ifs, line);
        line_num++;
    }
    
    // read the required line
    std::getline(ifs, line);
    std::istringstream iss(line);
    int peer_num;
    while (iss >> peer_num) {
        quorum_peers.push_back(peer_num);
    }
    return quorum_peers;
}

void Utils::log_message(int from, int to, int type, const std::string& dir)
{
    char fn[100];
    int client_num = (SENT == dir) ? from : to;
    sprintf(fn, "client%d.log", client_num);
    std::lock_guard<std::mutex> lock(file_mutex);
    std::ofstream ofs;
    ofs.open(fn, std::ofstream::out | std::ofstream::app);
    ofs << dir << " -- " << from << " -> " << MAEKAWA_MESSAGES[type] << " -> " << to << std::endl; 
    ofs.close();
}

