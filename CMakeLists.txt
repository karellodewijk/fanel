cmake_minimum_required(VERSION 2.6)
INCLUDE(CPack)

project(fanel)
set (fanel_VERSION 1.0)

find_package(OpenSSL)
find_package(Threads)
find_package(Boost 1.47.0 REQUIRED COMPONENTS system regex thread)

FILE(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/fanel/*.h")
FILE(GLOB implementation "${CMAKE_CURRENT_SOURCE_DIR}/fanel/*.impl")

set(LIBRARY_DIRS ${Boost_LIBRARY_DIRS} ${OPENSSL_LIBRARY_DIRS})
set(LIBRARIES ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

foreach(arg ${LIBRARY_DIRS})
    set(LINKER_FLAGS "${LINKER_FLAGS} ${arg}")
endforeach(arg ${LIBRARY_DIRS})

foreach(arg ${LIBRARIES})
    set(LINKER_FLAGS "${LINKER_FLAGS} ${arg}")
endforeach(arg ${LIBRARIES})

link_directories ( ${LIBRARY_DIRS} )
link_libraries ( ${LIBRARIES} )
include_directories ( fanel )

INSTALL(FILES ${headers} DESTINATION include/fanel)
INSTALL(FILES ${implementation} DESTINATION include/fanel)

set(CMAKE_CXX_FLAGS "-std=c++0x")

CONFIGURE_FILE (${CMAKE_CURRENT_SOURCE_DIR}/fanel.pc.cmake ${CMAKE_CURRENT_SOURCE_DIR}/fanel.pc)
FILE(GLOB pkgconf "${CMAKE_CURRENT_SOURCE_DIR}/fanel.pc")
INSTALL(FILES ${pkgconf} DESTINATION lib/pkgconfig)

add_subdirectory(examples)



