#ifndef CLIENT_H_INCLUDED
#define CLIENT_H_INCLUDED

#include <thread>
#include <iostream>
#include <stack>

#include "Log.h"
#include "Socket.h"
#include "Request.h"
#include "Response.h"

#define MAXCLIENT 10

class Client {
private:
    typedef void (*onRequest_func)(Request& requ, Response& resp);

    static Client* clients[MAXCLIENT];
    static std::stack<int> freeSlot;
    static std::stack<int> toFree;
    static std::thread freeMemoryTh;
    static bool continueFreeMemory;
    static void freeMemory();
public:
    static void init();
    static void end();
    static void freeClient(Client* client);
    static int newClient(Socket clt, onRequest_func onRequest, Socket::SSLConfig* conf);
    static int getNumClient();

private:
    Socket client;
    onRequest_func onRequest;
    Socket::SSLConfig* sslconfig;
    std::thread th;

public:
    Client(Socket client, onRequest_func, Socket::SSLConfig* conf);
    ~Client();

    void takeOver();
};

#endif // CLIENT_H_INCLUDED
