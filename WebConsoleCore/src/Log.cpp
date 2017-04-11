#include "Log.h"

using namespace std;

vector<Log::cmd_struct> Log::cmdList;
queue<Log::Message> Log::mlist;
ofstream* Log::flog;
thread Log::readMsgTh;
bool Log::continueReadMsg;
thread Log::consoleTh;
bool Log::continueConsole;
int Log::cmdspace = -1;

void Log::init() {
    flog = new ofstream();
    flog->open(LOGFILE, ios::app );
    continueReadMsg = true;
    readMsgTh = thread(readMsg);
    continueConsole = true;
    consoleTh = thread(console);
}

void Log::end() {
    continueReadMsg = false;
    readMsgTh.join();
    continueConsole = false;
    consoleTh.join();
    flog->close();
    delete(flog);
    flog = 0;
}

void Log::print(const string&  msg, short type) {
    Message m;
    m.msg = msg;
    m.type = type;
    mlist.push(m);
}

int Log::registerCmd(string cmd, callback_func func) {
    for(unsigned int i = 0; i<cmdList.size(); i++)
        if(cmdList[i].cmd == cmd)
            return -1; // ERROR: Command already registered

    cmd_struct newCmd;
    newCmd.cmd = cmd;
    newCmd.func = func;
    cmdList.push_back(newCmd);
    return 0;
}
int Log::unregisterCmd(string cmd) {
    for(unsigned int i = 0; i<cmdList.size(); i++)
        if(cmdList[i].cmd == cmd)
        {
            cmdList.erase(cmdList.begin()+i);
            if(i == cmdspace)
                cmdspace = -1;
            else if(cmdspace > i)
                cmdspace--;
            return 0;
        }
    return -2; // ERROR: Command not registered
}

void Log::readMsg() {
    while(continueReadMsg)
    {
        this_thread::sleep_for(chrono::microseconds(250000));

        while(!mlist.empty())
        {
            Message& m = mlist.front();

            time_t rawtime;
            struct tm * timeinfo;
            char date [80];
            time (&rawtime);
            timeinfo = localtime (&rawtime);
            strftime (date,80,"%Y-%m-%d %H:%M:%S ",timeinfo);

            switch (m.type)
            {
                case ERR:
                    cout << "ERR: " << m.msg << endl;
                    *flog << date << "ERR: " << m.msg << endl;
                    break;
                case WARNING:
                    cout << "/!\\: " << m.msg << endl;
                    *flog << date << "/!\\: " << m.msg << endl;
                    break;
                case INFO:
                    cout << m.msg << endl;
                    *flog << date << m.msg << endl;
                    break;
            }
            mlist.pop();
        }
    }
}

void Log::console() {
    while (continueConsole)
    {
        string line;
        istringstream cmdLine;
        if(cmdspace >= 0)
        {
            cout << cmdList[cmdspace].cmd << ">";
            getline(cin, line);
            cmdLine = istringstream(line);
            if(line == "exit")
                cmdspace = -1;
            else
                cmdList[cmdspace].func(cmdLine);
            continue;
        }

        getline(cin, line);
        cmdLine = istringstream(line);
        string cmd;
        cmdLine >> cmd;

        if(cmd == "stop") {
            if(cmdLine.eof()) {
                WebConsole::stop();
                continueConsole = false;
            } else cout << "Invalid parameter : stop" << endl;

        } else if (cmd == "load") {
            string param;
            if(cmdLine >> param) {
                int e = AppModule::load(param);
                if(e == 0)
                    Log::print("AppModule " + param + " loaded", Log::INFO);
                else
                    cout << "AppModuleError : " << e << endl;
            } else cout << "Missing parameter : load [name]" << endl;

        } else if (cmd == "unload") {
            string param;
            if(cmdLine >> param) {
                int e = AppModule::unload(param);
                if(e == 0)
                    Log::print("AppModule " + param + " unloaded", Log::INFO);
                else
                    cout << "AppModuleError : " << e << endl;
            } else cout << "Missing parameter : unload [name]" << endl;

        } else if (cmd == "reloadmime") {
            if(cmdLine.eof()) {
                if(FileSystem::loadMimeType() == 0)
                    Log::print("mime.type file reloaded", Log::INFO);
                else
                    Log::print("Impossible to reload mime.type file !", Log::ERR);
            } else cout << "Invalid parameter : reloadmime" << endl;

        } else {
            for(unsigned int i = 0; i < cmdList.size(); i++)
                if(cmdList[i].cmd == cmd)
                {
                    if(cmdLine.eof())
                        cmdspace = i;
                    else
                        cmdList[i].func(cmdLine);
                    goto continuelb;
                }
            cout << "Unknown command" << endl;
continuelb:;
        }
    }
}
