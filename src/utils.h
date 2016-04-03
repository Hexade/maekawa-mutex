#ifndef UTILS_H
#define UTILS_H

#include "exception.h"

#include <errno.h>
#include <string.h>

#include <cstdlib>
#include <deque>
#include <iostream>
#include <fstream>
#include <mutex>
#include <string>
#include <vector>

class Utils
{
    public:
        static void print_error(std::string msg, bool use_errno = false);

        static void inline print_info(std::string msg)
        {
            std::cout << "INFO: " << msg << std::endl;
        }

        static int str_to_int(std::string val_str) throw (Exception);

        static std::vector<std::string> split_str(std::string str,
            std::string delim);

        static void copy_str_to_arr(std::string str, char* arr, int len);

        static void reset_copy_arr(void* dest, void* src, int len);

        static std::vector<int> get_quorum_peer_nums(std::string filename, int client_num);

        static void log_message(int from, int to, int type, const std::string& dir);

        template <typename T>
        static void print_vector(const std::string& name, int from, const std::vector<T>& v ) {
            char fn[100];
            sprintf(fn, "client%d.log", from);
            std::lock_guard<std::mutex> lock(file_mutex);
            std::ofstream ofs;
            ofs.open(fn, std::ofstream::out | std::ofstream::app);
            ofs << name << ": ";
            for (const auto& item: v) {
                ofs << item << " ";
            }
            ofs << std::endl;
            ofs.close();
        }

        template <typename T>
        static void print_deque(const std::string& name, int from, const std::deque<T>& dq ) {
            char fn[100];
            sprintf(fn, "client%d.log", from);
            std::lock_guard<std::mutex> lock(file_mutex);
            std::ofstream ofs;
            ofs.open(fn, std::ofstream::out | std::ofstream::app);
            ofs << name << ": ";
            for (const auto& item: dq) {
                ofs << item << " ";
            }
            ofs << std::endl;
            ofs.close();
        }

        private:
            static std::mutex file_mutex;
        
};

#endif
