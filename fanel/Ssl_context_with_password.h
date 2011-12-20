#ifndef FANEL_SSL_CONTEXT_WITH_PASSWORD_H
#define FANEL_SSL_CONTEXT_WITH_PASSWORD_H

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

using namespace boost::asio;
using boost::asio::ip::tcp;

class Ssl_context_with_password : public ssl::context {
  public:
    Ssl_context_with_password(ssl::context::method method, const std::string& password_ = "") : ssl::context(method), password(password_) {
        set_password_callback(boost::bind(&Ssl_context_with_password::get_password, this));
        //set_verify_callback(boost::bind(&Ssl_context_with_password::verify_certificate, this, _1, _2));
    }
    std::string get_password() {return password;}
    /*
    bool verify_certificate(bool preverified, boost::asio::ssl::verify_context& ctx) {
        std::cout << "Verifying certificate " << preverified << std::endl;
        return preverified;
    }
    */

  private:  
    std::string password;
};

#endif //FANEL_SSL_CONTEXT_WITH_PASSWORD_H
