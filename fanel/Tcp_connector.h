#ifndef FANEL_TCP_CONNECTOR_H
#define FANEL_TCP_CONNECTOR_H

#ifdef USE_SSL
    #undef USE_SSL
    #include <Connector.impl>
    #define USE_SSL
#else
    #include <Connector.impl>
#endif

#endif //FANEL_TCP_CONNECTOR_H
