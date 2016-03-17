#include "exception.h"

#include <errno.h>
#include <string.h>

Exception::Exception(std::string msg, bool useErrno) 
    throw() : message(msg)
{
    if (useErrno) {
        message.append(": ");
        message.append(strerror(errno));
    }
}

const char* Exception::what(void) const throw()
{
    return this->message.c_str();
}
