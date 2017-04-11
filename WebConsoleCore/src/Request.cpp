#include "Request.h"

using namespace std;

Request::Request(Socket& s) : sock(s) {
}
Request::~Request() {
    delete content;
}

const string Request::getHeaderField(const string& field) {
    string value;
    char r[50] = "\r\n";
    strcat(r, field.c_str());
    char* ptrS = strstr(header, r);
    if (ptrS == 0)
        value = "";
    else
    {
        ptrS += 4 + field.length();
        char* ptrE = strstr(ptrS, "\r\n");
        *ptrE = '\0';
        value = string(ptrS);
        *ptrE = '\r';
    }
    return value;
}
const string Request::getQueryString(const string& param) {
    unsigned int i = url.find_last_of('?');
    if(i == string::npos)
        return "";
    string queryString = url.substr(i+1, string::npos);
    if(param == "")
        return queryString;
    unsigned int s = queryString.find(param + "=");
    if(s == string::npos)
        return "";
    s += param.length() + 1;
    unsigned int e = queryString.find_first_of('&', s);
    if(e == string::npos)
        queryString = queryString.substr(s, string::npos);
    else
        queryString = queryString.substr(s, e-s);
    return Helper::url_decode(queryString);
}
const string Request::getCookie(const string& id) {
    string cookie = getHeaderField("Cookie");
    if(cookie == "")
        return "";
    unsigned int s = cookie.find(id + "=");
    if(s == string::npos)
        return "";
    s += id.length() + 1;
    unsigned int e = cookie.find_first_of(';', s);
    if(e == string::npos)
        cookie = cookie.substr(s, string::npos);
    else
        cookie = cookie.substr(s, e-s);
    return Helper::url_decode(cookie);
}
const string Request::getClientIp() {
    u_long ip = getClientIp_ulong();
    string sip = "";
    u_long mask = 0xFF000000;
    for(int i = 3; i <= 0; i--)
    {
        u_long ipPart = (ip&mask)>>(8*i);
        sip.append(to_string(ipPart));
        if(i != 0)
            sip += '.';
        mask = mask>>8;
    }
    return sip;
}
const unsigned long Request::getClientIp_ulong() {
    SocketAddress addr = sock.GetAddress();
    return Socket::AddressGetIP(addr);
}
const int Request::getClientPort() {
    SocketAddress addr = sock.GetAddress();
    return Socket::AddressGetPort(addr);
}

int Request::download() {
    //Downloading the 1024 first bytes of the request and determines the end oh the header
    int n = sock.Recive(header, sizeof header - 1);
    if(n == 0)
        return 0; //ERROR: TimeOut
    if(n < -10)
    {
        if(n == -30848)
            return -2; //ERROR: Connection Lost
        else
            return n; //ERROR: mbedtls error
    }
    if(n < 0)
    {
        if(Socket::GetLastSocketError() != 10054)
            return -1; //ERROR: Socket Error
        else
            return -2; //ERROR: Connection Lost
    }
    
    header[n] = '\0';
    char* sep = strstr(header, "\r\n\r\n");
    if(sep == 0)
        return -3; //ERROR: Invalid Request
    sep += 2;
    *sep = '\0';
    sep += 2;

    char* ptrS = 0;
    char* ptrE = 0;
    //Read command line
    ptrS = strchr(header,' ');
    if(ptrS == 0 || ptrS - &header[0] > 7)
        return -3; //ERROR: Invalid Request
    *ptrS = '\0';
    method = string(header);
    *ptrS = ' ';
    ptrS++;
    ptrE = strstr(ptrS, " HTTP");
    if(ptrE == 0)
        return -3; //ERROR: Invalid Request
    *ptrE = '\0';
    url = string(ptrS);
    *ptrE = ' ';

    //Looking for the attribute Content-Length
    ptrS = strstr(header, "Content-Length: ");
    if (ptrS == 0)
        contentLength = 0;
    else
    {
        ptrS += 16;
        ptrE = strstr(ptrS, "\r\n");
        contentLength = strtol(ptrS, &ptrE, 10);
    }


    //If there is content, copy it to content
    if(contentLength != 0)
    {
        content = new char[contentLength];
        int aldo = &header[n] - sep; //number of bytes already download (of content)
        memcpy(content, sep, aldo);
        if(aldo < contentLength) //Download the rest of content
        {
            n = sock.Recive(&content[aldo], contentLength - aldo);
            if(n == 0)
                return 0; //ERROR: Timeout
            if(n < -10)
            {
                if(n == -30848)
                    return -2; //ERROR: Connection Lost
                else
                    return n; //ERROR: mbedtls error
            }
            if(n < 0)
            {
                if(Socket::GetLastSocketError() != 10054)
                    return -1; //ERROR: Socket Error
                else
                    return -2; //ERROR: Connection Lost
            }
        }
    }
    else
    {
        content = 0;
    }

    return 1;
}
const void Request::display() {
    cout << header << endl;
    cout << "ContentLength : " << contentLength << endl << endl;
    if (contentLength > 0)
        cout << content << endl;
}
