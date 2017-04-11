#include "WebConsole.h"

using namespace std;

vector<WebConsole::ListenedPort> WebConsole::listPort;

int WebConsole::init() {
    Socket::init();
    Log::init();
    Log::print("WebConsole Dev (build 014)", Log::INFO);
    Client::init();
    if(FileSystem::init() != 0)
        Log::print("Impossible to load mime.type file !", Log::ERR);
    AppModule::init();

    return 0;
}
int WebConsole::server(int port, onRequest_func onRequest, Socket::SSLConfig* sslconfig) {
    Socket serv = Socket(Socket::TCP, 0, port);
    if(!serv.IsValid() || serv.Listen(5) < 0)
    {
        Log::print("Impossible to start server on port" + to_string(port) + ". SocketError : " + to_string(Socket::GetLastSocketError()), Log::ERR);
        return -1;
    }
    serv.SetTimeout(1000); //1sec

    listPort.push_back(ListenedPort());
    vector<ListenedPort>::iterator thisPort = listPort.end() - 1;
    thisPort->port = port;
    thisPort->acceptClient = true;

    Log::print("Server listening on port " + to_string(port), Log::INFO);
    while (thisPort->acceptClient)
    {
        Socket clt = serv.Accept();
        if (clt.IsValid())
        {
            if (Client::newClient(clt, onRequest, sslconfig) < 0)
            {
                clt.Close();
                Log::print("Overload : Client rejected !", Log::WARNING);
            }
        }
    }
    serv.Close();
    listPort.erase(thisPort);

    Log::print("Stop listening on port " + to_string(port), Log::INFO);

    return 0;
}
int WebConsole::end() {
    AppModule::end();
    FileSystem::end();
    Client::end();
    Log::print("WebConsole ended", Log::INFO);
    Log::end();

    return 0;
}

void WebConsole::stop(int port) {
    if (port == 0)
    {
        for(int i = 0; i < listPort.size(); i++)
            listPort[i].acceptClient = false;
    }
    else
    {
        for(int i = 0; i < listPort.size(); i++)
            if(listPort[i].port == port)
            {
                listPort[i].acceptClient = false;
                break;
            }

    }
}
