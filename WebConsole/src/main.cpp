#include "WebConsole.h"

using namespace std;

void onRequest80(Request& request, Response& response) {
    if(Helper::defaultResource(request, response, ".") != 0)
        response.send();
}

int main() {
    WebConsole::init();
    Socket::SSLConfig sslconfig;
    sslconfig.LoadCrtFile("./serv2.crt");
    sslconfig.Config(Socket::SERVER, Socket::TCP, "./privatekey.key");
    WebConsole::server(443, onRequest80, &sslconfig);
    WebConsole::end();

    return 0;
}