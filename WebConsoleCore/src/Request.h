/* RequestError:
0 : TimeOut
-1 : SocketError
-2 : Connection Aborted
-3 : Invalid Request 
<-10 : mbedtls error */

#ifndef REQUEST_H_INCLUDED
#define REQUEST_H_INCLUDED

#include <iostream> //A sup?
#include <string.h>

#include "Socket.h"
#include "Helper.h"

class Request
{
private:
    char header[1025];
    char* content = 0;
    int contentLength;

    Socket& sock;

public:
    std::string method;
    std::string url;
    const std::string getHeaderField(const std::string& field); // Return "" if field isn't defined.
    const std::string getQueryString(const std::string& param = ""); // Return the query string parameter url decoded. If no param specified return the entire query string. Return "" if not param isn't defined.
    const std::string getCookie(const std::string& id); // Return "" if id isn't declared
    const std::string getClientIp();
    const unsigned long getClientIp_ulong();
    const int getClientPort();

    Request(Socket& sock);
    ~Request();

    int download();
    const void display();
};

#endif // REQUEST_H_INCLUDED
