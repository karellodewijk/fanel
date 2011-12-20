include_directories(../ fanel)
link_directories(${LIBRARY_DIRS})

add_executable(client client.cpp)
target_link_libraries(client ${LIBRARIES})
add_executable(server server.cpp)
target_link_libraries(server ${LIBRARIES})
add_executable(client_and_server client_and_server.cpp)
target_link_libraries(client_and_server ${LIBRARIES})

add_executable(delimiter_client client.cpp)
target_link_libraries(delimiter_client ${LIBRARIES})
add_executable(delimiter_server server.cpp)
target_link_libraries(delimiter_server ${LIBRARIES})
add_executable(delimiter_client_and_server client_and_server.cpp)
target_link_libraries(delimiter_client_and_server ${LIBRARIES})

set_target_properties(delimiter_server delimiter_client delimiter_client_and_server 
                      PROPERTIES 
                              COMPILE_FLAGS ""-DDELIMITER="::" ""
                     )

add_executable(netstring_client client.cpp)
target_link_libraries(netstring_client ${LIBRARIES})
add_executable(netstring_server server.cpp)
target_link_libraries(netstring_server ${LIBRARIES})
add_executable(netstring_client_and_server client_and_server.cpp)
target_link_libraries(netstring_client_and_server ${LIBRARIES})

set_target_properties(netstring_server netstring_client netstring_client_and_server 
                      PROPERTIES 
                              COMPILE_FLAGS "-DNETSTRING"
                     )

add_executable(streaming_client client.cpp)
target_link_libraries(client ${LIBRARIES})
add_executable(streaming_server server.cpp)
target_link_libraries(server ${LIBRARIES})
add_executable(streaming_client_and_server client_and_server.cpp)
target_link_libraries(client_and_server ${LIBRARIES})

set_target_properties(streaming_client streaming_server streaming_client_and_server 
                      PROPERTIES 
                              COMPILE_FLAGS "-DSTREAMING"
                     )

add_executable(ssl_client ssl_client_test.cpp)
target_link_libraries(ssl_client ${LIBRARIES})
add_executable(ssl_server ssl_server_test.cpp)
target_link_libraries(ssl_server ${LIBRARIES})

set_target_properties(ssl_server ssl_client 
                      PROPERTIES
                              COMPILE_FLAGS "-DUSE_SSL"
                     )



