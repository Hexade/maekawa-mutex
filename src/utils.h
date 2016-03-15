#ifndef UTILS_H
#define UTILS_H

#include "exception.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

class Utils
{
    public:
        static void inline print_error(std::string msg)
        {
            std::cerr << "error: " << msg << std::endl;
        }
    
        static int str_to_int(std::string val_str) throw (Exception);

        static std::vector<std::string> split_str(std::string str,
            std::string delim);

        static void copy_str_to_arr(std::string str, char* arr, int len);

        static void reset_copy_arr(void* dest, void* src, int len);

};

#endif
