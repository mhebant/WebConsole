/* AppModuleError:
0 : No error
-1 : Unable to load the library
-2 : Unable to extract function
-3 : Too many AppModule loaded (more than MAXAPPMODULE)
-4 : There is no loaded AppModule named this way (AppModule not loaded)
-5 : Initialization of the module failed
*/

#ifndef APP_H_INCLUDED
#define APP_H_INCLUDED

#ifdef WIN32 /* Windows */
    #include <windows.h>
    #define EXTENSION ".dll"
#elif defined LINUX /* Linux */
    #include <dlfcn.h>
    //#include <sys/types.h>
    #include <dirent.h>
    #define HMODULE void*
    #define FARPROC void*
    #define LoadLibrary(s) dlopen(s, RTLD_LAZY)
    #define GetProcAddress(l, s) dlsym(l, s)
    #define FreeLibrary(l) dlclose(l)

    #define EXTENSION ".so"
#else
    #error Please define WIN32 or LINUX to compile
#endif

#include <string>

#include "Log.h"

#define MAXAPPMODULE 10

class AppModule{
public:
    struct init_info {std::string name; bool wellInit;};

private:
    static int errorCode;
    static AppModule* appModule[MAXAPPMODULE];

public:
    static int init();
    static int end();

    static int getLastError();
    static int load(const std::string& name);
    static int unload(const std::string& name);

private:
    HMODULE lib;
    std::string name;
    init_info (*init_func)(void);
    void (*end_func)(void);
    init_info info;

public:
    AppModule(const std::string& path);
    ~AppModule();
};

#endif // APP_H_INCLUDED
