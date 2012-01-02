#include <fanel/Tcp_connection.h>
#include <fanel/Tcp_acceptor.h>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <set>
#include <iostream>
#include <sstream>

using namespace boost::filesystem;

filesystem::path root;

struct Get_request {
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

    void fetch(char*& page, size_t& size) {
        path p = root / uri;
        if (!exists(p)) return;
        if (is_directory(p)) {
            if (is_regular_file(p / "index.html")) {
                p /= "index.html";
            } else if (is_regular_file(p / "index.htm")) {
                p /= "index.htm";
            } else {
                return;
            }
        }
        size = file_size(p);
        page = (char *)malloc(size);
        filesystem::ifstream file(p);
        file.read(page, size);
    }
};

std::ostream& operator<<(std::ostream& out, const Get_request& request) {
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
		m_connections.insert(connection);
	}
	
	void error(Connection* connection, const system::error_code& error_code) {
	    delete connection;
		m_connections.erase(connection);
	}
	
	void error(const system::error_code& error_code) {
		std::cout << "Accept failed: " << error_code.message() << std::endl;
	}
	
	void received(Connection* connection, const char* data, int size) {
	    Get_request request;
	    request.parse(data, size);
	    std::cout << request << std::endl;
        char* page;
        size_t page_size = 0;
        request.fetch(page, page_size);
        connection->write(page, page_size);
	}

    void done_writing(Connection* connection) {
	    delete connection;
		m_connections.erase(connection);
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

    root = path(document_root); 
    if (!exists(root) || !is_directory(root))  {
        std::cout << "Path: " << document_root << ", does not exist or is not a directory." <<  std::endl;
        exit(1);
    }

    boost::asio::io_service io_service;
    Acceptor acceptor(io_service);
    acceptor.accept(port);
    io_service.run();
 
    return 0;
}
