#include <fanel/Tcp_connection.h>
#include <fanel/Tcp_acceptor.h>
#include <set>
#include <iostream>
#include <sstream>

struct Request {
    std::map<std::string, std::string> headers;
    std::string command;
    std::string uri;
    std::string protocol;

    bool parse_field(std::string& dst, const char*& current, const char delim, int& remaining) {
        size_t pos = 0;
        while(current[pos] != delim && remaining > 0) {
           pos++;        
           remaining--;
        }
        dst = std::move(std::string(current, pos));
        current += pos + 1;
        remaining--;
        return true;
    }
    
    void parse(const char* message, int remaining) {
        //parse request function, performance shouldn't be
        //too shabby, no parse_field will read more than
        //remaining characters and it will update remaining
        //so should be safe as well
        parse_field(command, message, ' ', remaining);
        parse_field(uri, message, ' ', remaining);
        parse_field(protocol, message, '\r', remaining);
        message++; //drop the newline
        remaining--;
        while (remaining > 0) {
            std::pair<std::string, std::string> header;
            parse_field(header.first, message, ':', remaining);
            parse_field(header.second, message, '\r', remaining);
            headers.insert(header);
            message++; //drop the newline
            remaining--;
        }
    }
};

std::ostream& operator<<(std::ostream& out, const Request& request) {
    out << "Command: " << request.command << std::endl;
    out << "Uri: " << request.uri << std::endl;
    out << "Protocol: " << request.protocol << std::endl;
    out << "Headers: " << std::endl;
    for (auto it = request.headers.begin(); it != request.headers.end(); ++it) {
        std::cout<< "\t" << it->first << ": " << it->second << std::endl;
    }
    return out;
}

class Acceptor : public Tcp_acceptor<> {
  public:
	Acceptor(boost::asio::io_service& io_service) : Tcp_acceptor<>(io_service) {}

	~Acceptor() {
	    for (auto it = m_connections.begin(); it != m_connections.end(); ++it) {
	        delete *it;
	    }
	}
	
	void accepted(Connection* connection) {
	    std::string server_hello("server says hello");
		m_connections.insert(connection);
		connection->write(server_hello.data(), server_hello.size());
	}
	
	void error(Connection* connection, const system::error_code& error_code) {
	    delete connection;
		m_connections.erase(connection);
	}
	
	void error(const system::error_code& error_code) {
		std::cout << "Accept failed: " << error_code.message() << std::endl;
	}
	
	void received(Connection* connection, const char* data, int size) {
		std::cout << std::string(data, size) << std::endl;
	    Request request;
	    request.parse(data, size);
	    std::cout << request << std::endl;
	}
	
    std::set<Connection*> m_connections;
};

int main(int argc, const char* argv[]) {
    if (argc < 3) {
        std::cout << "usage: ./http_server port document_root"<< std::endl;
        exit(1);
    }
    
    std::string port_str = argv[1];
    std::string document_root; 
    for (int i = 2; i < argc; ++i) 
        document_root += argv[i];

    std::istringstream istr(port_str);
    int port;
    istr >> port;

    std::cout << "Serving on port: " << port << std::endl;
    std::cout << "Document root: " << document_root << std::endl;     
    
    boost::asio::io_service io_service;
    Acceptor acceptor(io_service);
    acceptor.accept(port);
    io_service.run();
 
    return 0;
}
