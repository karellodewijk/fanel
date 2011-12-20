#ifndef FANEL_TCP_PORT_CONNECTOR_H
#define FANEL_TCP_PORT_CONNECTOR_H

#ifdef USE_SSL
    #undef USE_SSL
    #include <PortConnector.impl>
    #define USE_SSL
#else
    #include <PortConnector.impl>
#endif

#endif //FANEL_TCP_PORT_CONNECTOR_H
