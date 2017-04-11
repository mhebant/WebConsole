#include "FileSystem.h"

using namespace std;

vector<FileSystem::Mime> FileSystem::mimeTypes;

int FileSystem::init() {
    return loadMimeType();
}
void FileSystem::end() {

}
int FileSystem::loadMimeType() {
    mimeTypes = vector<Mime>();

    ifstream file("mime.type");
    if(!file.good()) {
        return -1; // ERROR -1 : Impossible to read mime.type file
    }

    int l = 0;
    string line;
    while(getline(file, line))
    {
        l++;
        Mime m;
        int i = line.find_first_of(' ');
        if(i == string::npos) {
            return -2; // ERROR -2 : Invalid file line
        }
        m.ext = line.substr(0, i);
        m.type = line.substr(i+1, line.length()-i-2);
        mimeTypes.push_back(m);
    }

    file.close();

    return 0;
}

bool FileSystem::exist(const string& path) {
    return access( path.c_str(), F_OK ) != -1;
}

bool FileSystem::isDirectory(const string& path) {
    struct stat s;
    if(stat(path.c_str(), &s) == 0 )
        return s.st_mode & S_IFDIR;
    else
        return false; // ERROR !!
}

size_t FileSystem::sizeFile(const string& path) {
    struct stat s;
    if(stat(path.c_str(), &s) != 0)
        return 0; // ERROR
    return s.st_size;
}

time_t FileSystem::mTimeFile(const string& path) {
    struct stat s;
    if(stat(path.c_str(), &s) != 0)
        return 0; // ERROR
    return s.st_mtime;
}

string FileSystem::getMimeType(const string& path) {
    int i = path.find_last_of('.');
    if(i == string::npos)
        i = 0;
    else
        i++;
    string ext = path.substr(i);

    if(mimeTypes.size() == 0)
        return "";

    i = 0;
    while(mimeTypes[i].ext != ext)
    {
        i++;
        if(i == mimeTypes.size())
            return "";
    }
    return mimeTypes[i].type;
}
