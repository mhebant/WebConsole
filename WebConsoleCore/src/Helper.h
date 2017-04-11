#ifndef HELPER_H_INCLUDED
#define HELPER_H_INCLUDED

#include <string>
#include <fstream>

#include "Request.h"
#include "Response.h"
#include "FileSystem.h"

#define BUFFSIZE 1024000

class Request;
class Response;

class Helper {
private:
    static std::string chartohex(char dec);
    static char hextochar(const char* hex);

public:
    static std::string url_encode(const std::string& str);
    static std::string url_decode(const std::string& url);

    static int defaultResource(Request& request, Response& response, std::string root);
};

#endif // HELPER_H_INCLUDED
