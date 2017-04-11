#include "Client.h"

using namespace std;

Client* Client::clients[MAXCLIENT];
stack<int> Client::freeSlot;
stack<int> Client::toFree;
thread Client::freeMemoryTh;
bool Client::continueFreeMemory;

void Client::init() {
    for(int i = 0; i < MAXCLIENT; i++)
    {
        clients[i] = 0;
        freeSlot.push(i);
    }
    continueFreeMemory = true;
    freeMemoryTh = thread(freeMemory); //Delete if you dont care about memory
}
void Client::end() {
    for(int i = 0; i < MAXCLIENT; i++)
        delete clients[i];

    continueFreeMemory = false;
    freeMemoryTh.join(); //Delete if you dont care about memory
}
void Client::freeClient(Client* client) {
    int i;

    for(i = 0; i < MAXCLIENT; i++)
        if(clients[i] == client)
            break;
    if(i < MAXCLIENT)
        toFree.push(i);
    else
        Log::print("ClientManager failed (free inexistent client) (this error may not appears !!)", Log::ERR);
}
int Client::newClient(Socket clt, onRequest_func func, Socket::SSLConfig* conf) {
    if(!toFree.empty())
    {
        int i = toFree.top();
        toFree.pop();
        delete clients[i];
        clients[i] = new Client(clt, func, conf);
        return 0;
    }
    else if(!freeSlot.empty())
    {
        clients[freeSlot.top()] = new Client(clt, func, conf);
        freeSlot.pop();
        return 0;
    }
    else
        return -1;
}
void Client::freeMemory() {
    while(continueFreeMemory)
    {
        while(!toFree.empty())
        {
            int i = toFree.top();
            toFree.pop();
            delete clients[i];
            clients[i] = 0;
            freeSlot.push(i);
        }
        this_thread::sleep_for(chrono::microseconds(1000000));
    }
}
int Client::getNumClient() {
    return MAXCLIENT - (freeSlot.size() + toFree.size());
}


Client::Client(Socket clt, onRequest_func func, Socket::SSLConfig* conf) : client(clt), onRequest(func), sslconfig(conf)  {
    th = thread(&Client::takeOver, this);
}
Client::~Client() {
    if(th.joinable())
        th.join();
}

void Client::takeOver() {
    client.SetTimeout(5000); // 5sec
    
    int e;
    if (sslconfig == 0 || (e=client.SSL(*sslconfig)) == 0)
        while(true)
        {
            Request request(client);
            Response response(client);

            e = request.download();
            if (e > 0)
            {
                onRequest(request, response);
                if(sslconfig == 0)
                    Log::print("HTTP request for " + request.url + " Status: " + to_string(response.code), Log::INFO);
                else
                    Log::print("HTTPS request for " + request.url + " Status: " + to_string(response.code), Log::INFO);
                
                if(request.getHeaderField("Connection") == "keep-alive")
                    continue;
                else
                    break;
            }
            if(e == 0) // TimeOut
                break;
            if(e == -1) // Socket Error
            {
                Log::print("Failed downloading request. SocketError : " + to_string(Socket::GetLastSocketError()) , Log::ERR);
                break;
            }
            if(e == -2) // Connection aborted
                break;
            if(e == -3) // Request Invalid
            {
                Log::print("Invalid request received", Log::WARNING);
                response.code = 400;
                response.send();
                continue;
            }
            if(e < -10)
            {
                Log::print("Failed downloading request. mbedtls : " + to_string(e) , Log::ERR);
                break;
            }
        }
    else
        Log::print("Failed opening SSL connection. mbedtls error: " + to_string(e), Log::WARNING);

    client.Close();

    Client::freeClient(this);
}
