#ifndef FANEL_TCP_ACCEPTOR_H
#define FANEL_TCP_ACCEPTOR_H

#ifdef USE_SSL
    #undef USE_SSL
    #include <Acceptor.impl>
    #define USE_SSL
#else
    #include <Acceptor.impl>
#endif

#endif //FANEL_TCP_ACCEPTOR_H
