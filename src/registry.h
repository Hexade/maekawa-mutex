#ifndef REGISTRY_H
#define REGISTRY_H

#include <map>
#include <string>

class Registry
{
    public:
        ~Registry()
        {}

        static Registry& instance(void);

        //file ops
        std::string add_file(std::string filename);
        std::string seek_file(std::string filename, int index);
        std::string read_file(std::string filename, int length);
        std::string write_file(std::string filename, std::string data);
        std::string remove_file(std::string filename);
        std::string open_append_close(std::string filename, std::string data);
        
        bool test_add_file(std::string filename, std::string& res);
        bool test_seek_file(std::string filename, int index,
            std::string& res);
        bool test_read_file(std::string filename, int length,
            std::string& res);
        bool test_write_file(std::string filename, std::string& res);
        bool test_remove_file(std::string filename, std::string& res);
        bool test_open_append_close(std::string filename, std::string& res);

        inline void clear_seek_positions(void)
        {
            seek_positions.clear();
        }

    private:
        Registry()
        { }

        static Registry* instance_;

        std::map<std::string, int> seek_positions;

};

#endif
