#include "Response.h"

using namespace std;

Response::Response(Socket& s) : sock(s) {
}
Response::~Response() {
    if(contentHosted)
        delete content;
}

void Response::addHeader(const char* field, const char* value) {
    strcat(header, field);
    strcat(header, ": ");
    strcat(header, value);
    strcat(header, "\r\n");
}
void Response::setContent(char* ptrContent, int length) {
    if(contentHosted)
        delete content;
    content = ptrContent;
    contentLength = length;
    contentHosted = false;
}
char* Response::createContent(int length) {
    if(contentHosted)
        delete content;
    content = new char[length];
    contentLength = length;
    contentHosted = true;
    return content;
}

int Response::sendHeader() {
    if(!headerSended)
    {
        addHeader("Connection", "keep-alive");
        addHeader("Keep-Alive", "timeout=5");
        time_t rawtime;
        struct tm * timeinfo;
        char date [40];
        time (&rawtime);
        timeinfo = gmtime(&rawtime);
        strftime (date,40,"%a, %d %b %Y %H:%M:%S GMT",timeinfo);
        addHeader("Date", date);
        addHeader("Content-Length", to_string(contentLength).c_str());

        strcat(header, "\r\n");

        char* data = new char[strlen(header)+40];
        *data = '\0';
        strcat(data, "HTTP/1.1 ");
        strcat(data, to_string(code).c_str());
        switch(code)
        {
        case 200:
            strcat(data, " OK\r\n");
            break;
        case 304:
            strcat(data, " Not Modified\r\n");
            break;
        case 404:
            strcat(data, " Not Found\r\n");
            break;
        case 500:
            strcat(data, " Internal Server Error\r\n");
            break;

        case 100:
            strcat(data, " Continue\r\n");
            break;
        case 102:
            strcat(data, " Processing\r\n");
            break;
        case 204:
            strcat(data, " No Content\r\n");
            break;
        case 300:
            strcat(data, " Multiple Choices\r\n");
            break;
        case 301:
            strcat(data, " Moved Permanently\r\n");
            break;
        case 302:
            strcat(data, " Moved Temporarily\r\n");
            break;
        case 307:
            strcat(data, " Temporary Redirect\r\n");
            break;
        case 308:
            strcat(data, " Permanent Redirect\r\n");
            break;
        case 310:
            strcat(data, " Too many Redirects\r\n");
            break;
        case 400:
            strcat(data, " Bad Request\r\n");
            break;
        case 403:
            strcat(data, " Forbidden\r\n");
            break;
        case 429:
            strcat(data, " Too Many Requests\r\n");
            break;
        case 503:
            strcat(data, " Service Unavailable\r\n");
            break;
        case 508:
            strcat(data, " Loop detected\r\n");
            break;
        }

        strcat(data, header);
        int e = sock.Send(data, strlen(data));
        delete data;
        if (e < 0)
            return -1; // SocketError
        else {
            headerSended = true;
            return 1;
        }
    }
    return 0; // Headers already sent
}
int Response::send() {
    if (sendHeader() < 0)
        return -1;
    if(content != 0)
        if(sock.Send(content, contentLength) < 0)
            return -1;
    return 1;
}
