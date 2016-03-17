#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <exception>
#include <string>

class Exception: public std::exception
{
    public:
        Exception(std::string message, bool useErrno = false) throw();
        ~Exception() throw() { }
        const char* what(void) const throw();

    private:
        std::string& message;
};

#endif
