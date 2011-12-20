prefix=${CMAKE_INSTALL_PREFIX}/include
exec_prefix=${CMAKE_INSTALL_PREFIX}/bin
libdir=${CMAKE_INSTALL_PREFIX}/lib
includedir=${CMAKE_INSTALL_PREFIX}/include/fanel

Name: fanel
Description: Simple to use asynchronous network library build upon boost asio
Version: ${fanel_VERSION}
Libs: ${LINKER_FLAGS}
Cflags: ${CMAKE_CXX_FLAGS}
