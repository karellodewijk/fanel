#ifndef FANEL_TCP_PORT_ACCEPTOR_H
#define FANEL_TCP_PORT_ACCEPTOR_H

#ifdef USE_SSL
    #undef USE_SSL
    #include <PortAcceptor.impl>
    #define USE_SSL
#else
    #include <PortAcceptor.impl>
#endif

#endif //FANEL_TCP_PORT_ACCEPTOR_H
