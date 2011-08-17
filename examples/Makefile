all: client server client_and_server client_and_server_multithread
	
server: server_test.cpp ../Tcp_connection.h ../Tcp_connection.cpp ../Connection_manager.h ../Connection_manager.cpp ../Tcp_acceptor.h
	g++ -o server -std=c++0x -g server_test.cpp ../Tcp_connection.cpp ../Connection_manager.cpp -I/usr/include -I../ -L/usr/lib -lboost_system
	
client: client_test.cpp ../Tcp_connection.h ../Tcp_connection.cpp ../Connection_manager.h ../Connection_manager.cpp ../Tcp_connector.h
	g++ -o client -std=c++0x -g client_test.cpp ../Tcp_connection.cpp ../Connection_manager.cpp -I/usr/include -I../ -L/usr/lib -lboost_system
	
client_and_server: client_and_server.cpp ../Tcp_connection.h ../Tcp_connection.cpp ../Connection_manager.h ../Connection_manager.cpp ../Tcp_connector.h ../Tcp_acceptor.h ../Tcp_connector_and_acceptor.h
	g++ -o client_and_server -std=c++0x -g client_and_server.cpp ../Tcp_connection.cpp ../Connection_manager.cpp -I/usr/include -I../ -L/usr/lib -lboost_system
	
client_and_server_multithread: client_and_server_multithread.cpp ../Tcp_connection.h ../Tcp_connection.cpp ../Connection_manager.h ../Connection_manager.cpp ../Tcp_connector.h ../Tcp_acceptor.h ../Tcp_connector_and_acceptor.h
	g++ -o client_and_server_multithread -DTHREADSAFE -std=c++0x -g client_and_server_multithread.cpp ../Tcp_connection.cpp ../Connection_manager.cpp -I/usr/include -I../ -L/usr/lib -lboost_system -lboost_thread
	
client_big_message: client_test_big_message.cpp ../Tcp_connection.h ../Tcp_connection.cpp ../Connection_manager.h ../Connection_manager.cpp ../Tcp_connector.h
	g++ -o client_big_message -std=c++0x -g client_test_big_message.cpp ../Tcp_connection.cpp ../Connection_manager.cpp -I/usr/include -I../ -L/usr/lib -lboost_system


delimiter: delimiter_server_example delimiter_client_example delimiter_client_and_server

delimiter_server_example: server_test.cpp ../Tcp_connection.h ../Tcp_connection.cpp ../Connection_manager.h ../Connection_manager.cpp ../Tcp_acceptor.h
	g++ -o delimiter_server_example server_test.cpp -DDELIMITER=\"::\" -std=c++0x -g ../Tcp_connection.cpp ../Connection_manager.cpp -I/usr/include -I../ -L/usr/lib -lboost_system
	
delimiter_client_example: client_test.cpp ../Tcp_connection.h ../Tcp_connection.cpp ../Connection_manager.h ../Connection_manager.cpp ../Tcp_connector.h
	g++ -o delimiter_client_example -std=c++0x -g client_test.cpp -DDELIMITER=\"::\" ../Tcp_connection.cpp ../Connection_manager.cpp -I/usr/include -I../ -L/usr/lib -lboost_system
	
delimiter_client_and_server: client_and_server.cpp ../Tcp_connection.h ../Tcp_connection.cpp ../Connection_manager.h ../Connection_manager.cpp ../Tcp_connector.h ../Tcp_acceptor.h ../Tcp_connector_and_acceptor.h
	g++ -o delimiter_client_and_server -DDELIMITER=\"::\" -std=c++0x -g client_and_server.cpp ../Tcp_connection.cpp ../Connection_manager.cpp -I/usr/include -I../ -L/usr/lib -lboost_system


netstring: netstring_server_example netstring_client_example netstring_client_and_server

netstring_server_example: server_test.cpp ../Tcp_connection.h ../Tcp_connection.cpp ../Connection_manager.h ../Connection_manager.cpp ../Tcp_acceptor.h
	g++ -o netstring_server_example server_test.cpp -DNETSTRING -std=c++0x -g ../Tcp_connection.cpp ../Connection_manager.cpp -I/usr/include -I../ -L/usr/lib -lboost_system
	
netstring_client_example: client_test.cpp ../Tcp_connection.h ../Tcp_connection.cpp ../Connection_manager.h ../Connection_manager.cpp ../Tcp_connector.h
	g++ -o netstring_client_example -std=c++0x -g client_test.cpp -DNETSTRING ../Tcp_connection.cpp ../Connection_manager.cpp -I/usr/include -I../ -L/usr/lib -lboost_system
	
netstring_client_and_server: client_and_server.cpp ../Tcp_connection.h ../Tcp_connection.cpp ../Connection_manager.h ../Connection_manager.cpp ../Tcp_connector.h ../Tcp_acceptor.h ../Tcp_connector_and_acceptor.h
	g++ -O0 -o netstring_client_and_server -DNETSTRING -std=c++0x -g client_and_server.cpp ../Tcp_connection.cpp ../Connection_manager.cpp -I/usr/include -I../ -L/usr/lib -lboost_system

