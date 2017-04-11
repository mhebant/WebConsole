#include "AppModule.h"

using namespace std;

int AppModule::errorCode = 0;
AppModule* AppModule::appModule[MAXAPPMODULE];

int AppModule::init() {
    int i = 0;

#ifdef WIN32
    HANDLE dir;
    WIN32_FIND_DATA file_data;

    if ((dir = FindFirstFile("./app/*", &file_data)) != INVALID_HANDLE_VALUE)
    {
        do {
            const string file_name = file_data.cFileName;
            const bool is_directory = (file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
#elif defined LINUX
    DIR *dir;
    class dirent *ent;
    class stat st;

    if ((dir = opendir("./app/")) != NULL)
    	while ((ent = readdir(dir)) != NULL) {
        	const string file_name = ent->d_name;
        	const bool is_directory = (st.st_mode & S_IFDIR) != 0;
        	/*if (stat(full_file_name.c_str(), &st) == -1)
            	continue;*/
        	if (stat(file_name.c_str(), &st) == -1)
            	continue;
#endif

    if (file_name[0] == '.' || file_name.substr(file_name.length()-strlen(EXTENSION), strlen(EXTENSION)) != EXTENSION)
        continue;
    if (is_directory)
        continue;

    if(i < MAXAPPMODULE)
    {
        appModule[i] = new AppModule("./app/" + file_name);
        if(!appModule[i]->info.wellInit)
        {
            delete appModule[i];
            appModule[i] = 0;
            Log::print("Error loading an AppModule. File: " + file_name + " AppModuleError: " + to_string(errorCode), Log::ERR);
            continue;
        }
        else
            i++;
    }
    else
    {
        Log::print("Too many AppModule loaded !", Log::WARNING);
        errorCode = -3;
        break;
    }

#ifdef WIN32
        } while (FindNextFile(dir, &file_data));
    }

    FindClose(dir);
#elif defined LINUX
    	}
    closedir(dir);
#endif
    string msg = "";
    i = 0;
    while(appModule[i] != 0)
    {
        if(i == 0)
            msg.append("AppModule correctly loaded: " + appModule[i]->info.name);
        else
            msg.append(", " + appModule[i]->info.name);
        i++;
    }
    if(msg != "")
        Log::print(msg, Log::INFO);
    return 0;
}
int AppModule::end() {
    for(int i = 0; i < MAXAPPMODULE; i++)
    {
        if(appModule[i] == 0)
            delete appModule[i];
    }
    return 0;
}

int AppModule::getLastError() {
    return errorCode;
}
int AppModule::load(const string& name) {
    for(int i = 0; i < MAXAPPMODULE; i++)
    {
        if(appModule[i]==0)
        {
            appModule[i] = new AppModule("./app/" + name + EXTENSION);
            if(!appModule[i]->info.wellInit)
            {
                delete appModule[i];
                appModule[i] = 0;
                return errorCode;
            }
            else
                return 0;
        }
    }
    return -3;
}
int AppModule::unload(const string& name) {
    for(int i = 0; i < MAXAPPMODULE; i++)
    {
        if(appModule[i] != 0 && appModule[i]->info.name == name)
        {
            delete appModule[i];
            appModule[i] = 0;
            return 0;
        }
    }
    return -4;
}

AppModule::AppModule(const string& path) {
    init_func = 0;
    end_func = 0;
    lib = LoadLibrary(path.c_str());
    if(lib == 0)
    {
        errorCode = -1;
        info.wellInit = false;
        return;
    }
    init_func = (init_info (*)(void)) GetProcAddress(lib, "init");
    end_func = (void (*)(void)) GetProcAddress(lib, "end");
    if(init_func == 0 || end_func == 0)
    {
        errorCode = -2;
        info.wellInit = false;
        return;
    }
    info = init_func();
    if(!info.wellInit)
    {
        errorCode = -5;
        return;
    }
}
AppModule::~AppModule() {
    if(end_func != 0)
        end_func();
    if(lib != 0)
        FreeLibrary(lib);
}
