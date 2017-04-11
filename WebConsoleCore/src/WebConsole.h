#ifndef WEBCONSOLE_H_INCLUDED
#define WEBCONSOLE_H_INCLUDED

#include <thread>
#include <vector>

#include "Client.h"
#include "Request.h"
#include "Response.h"
#include "AppModule.h"

class WebConsole {
private:
    typedef void (*onRequest_func)(Request& requ, Response& resp);

    struct ListenedPort { int port; bool acceptClient; };
    static std::vector<ListenedPort> listPort;

public:
    static int init();
    static int server(int port, onRequest_func onRequest, Socket::SSLConfig* sslconfig = 0);
    static int end();

    static void stop(int port = 0);
};

#endif // WEBCONSOLE_H_INCLUDED
