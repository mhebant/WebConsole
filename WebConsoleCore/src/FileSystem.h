/* ----File ver0.1(28/07/2015)----
Win 32 / Linux

Represent
Define WIN32 if compile for Windows 32 bit or LINUX if compile for Linux
*/

#ifndef FILE_H_INCLUDED
#define FILE_H_INCLUDED

/*#ifdef WIN32
    #include <windows.h>
#elif defined LINUX
    #include
#else
#endif*/

#include <string>
#include <vector>
#include <fstream>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>

class FileSystem {
private:
    struct Mime { std::string ext; std::string type; };
    static std::vector<Mime> mimeTypes;

public:
    static int init();
    static void end();
    static int loadMimeType();

    static bool exist(const std::string& path);
    static bool isDirectory(const std::string& path);
    static size_t sizeFile(const std::string& path);
    static time_t mTimeFile(const std::string& path);
    static std::string getMimeType(const std::string& path);
};

#endif // FILE_H_INCLUDED
