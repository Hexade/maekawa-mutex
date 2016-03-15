#include "exception.h"
#include "sock_data_cb.h"

void SockDataCb::subscribe(on_sock_data cb, void* data)
{
    cb_func = cb;
    sock_data = data;
}

void SockDataCb::on_data_read(TcpSocket* sock) throw (Exception)
{
    if (!cb_func)
        throw Exception("callback function not defined");
    cb_func(sock_data, sock);
}
