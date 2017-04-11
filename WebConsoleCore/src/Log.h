/*
LogError
0 : No error
-1 : Command already registered
-2 : Command not registered (not found)
*/

#ifndef LOG_H_INCLUDED
#define LOG_H_INCLUDED

#include <queue>
#include <vector>
#include <thread>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <time.h>

#include "WebConsole.h"
#include "AppModule.h"
#include "FileSystem.h"

#define LOGFILE "webconsole.log"

class Log {
private:
    typedef void (*callback_func)(std::istringstream&);
    struct Message{ std::string msg; int type; };

    struct cmd_struct { std::string cmd; callback_func func;};
    static std::vector<cmd_struct> cmdList;
    static std::queue<Message> mlist;
    static std::ofstream* flog;
    static std::thread readMsgTh;
    static bool continueReadMsg;
    static std::thread consoleTh;
    static bool continueConsole;
    static int cmdspace;

    static void readMsg();
    static void console();

public:
    static const short ERR = 1;
    static const short WARNING = 2;
    static const short INFO = 3;

    static void init();
    static void end();

    static void print(const std::string& msg, short type);
    static int registerCmd(std::string cmd, callback_func func);
    static int unregisterCmd(std::string cmd);


};

#endif // LOG_H_INCLUDED
