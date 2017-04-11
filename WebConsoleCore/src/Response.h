/*
ResponseError
0 : Header already sent
-1 : SocketError
*/

#ifndef RESPONSE_H_INCLUDED
#define RESPONSE_H_INCLUDED

#include <string>
#include <cstring>
#include <time.h>

#include "Socket.h"

class Response
{
private:
    char header[2048] = "";
    bool headerSended = false;
    char* content = 0;
    int contentLength = 0;
    bool contentHosted = false; // A sup ?

    Socket& sock;

public:
    int code = 200;

    Response(Socket& sock);
    ~Response();

    void addHeader(const char* field, const char* value);
    void setContent(char* ptrContent, int length);
    char* createContent(int length); // A sup ?

    int sendHeader();
    int send();
};

#endif // RESPONSE_H_INCLUDED
