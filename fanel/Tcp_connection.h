#ifndef FANEL_TCPCONNECTION_H
#define FANEL_TCPCONNECTION_H

#ifdef USE_SSL
    #undef USE_SSL
    #include "Socket_connection.impl"
    #define USE_SSL
#else
    #include "Socket_connection.impl"
#endif

typedef Socket_connection<asio::ip::tcp::socket> Tcp_connection;

#endif //FANEL_TCPCONNECTION_H
