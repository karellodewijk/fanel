#ifndef FANEL_SSL_CONNECTION
#define FANEL_SSL_CONNECTION

#include "Socket_connection.impl"

typedef Socket_connection<boost::asio::ssl::stream<boost::asio::ip::tcp::socket> > Ssl_connection;

#endif //FANEL_SSL_CONNECTION
