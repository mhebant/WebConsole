#include "Helper.h"

using namespace std;

string Helper::chartohex(const char dec) {
    char dig1 = (dec&0xF0)>>4;
    char dig2 = (dec&0x0F);
    if ( 0<= dig1 && dig1<= 9) dig1+=48;    //0,48 in ascii
    else if (10<= dig1 && dig1<=15) dig1+=65-10; //A,65 in ascii
    if ( 0<= dig2 && dig2<= 9) dig2+=48;
    else if (10<= dig2 && dig2<=15) dig2+=65-10;

    string hex;
    hex.append( &dig1, 1);
    hex.append( &dig2, 1);
    return hex;
}

char Helper::hextochar(const char* hex) {
    char dig1 = hex[0];
    char dig2 = hex[1];
    if(48 <= dig1 && dig1 <= 57) dig1=-48;
    else if(65 <= dig1 && dig1 <= 73) dig1=-65+10;
    if(48 <= dig2 && dig2 <= 57) dig2=-48;
    else if(65 <= dig2 && dig2 <= 73) dig2=-65+10;

    char dec;
    dec = (dig1<<4)&dig2;
    return dec;
}

string Helper::url_encode(const string& str) {
    string url;
    int max = str.length();
    for(int i=0; i < max; i++)
    {
        if (
        (48 <= str[i] && str[i] <= 57) ||//0-9
        (65 <= str[i] && str[i] <= 90) ||//ABC...XYZ
        (97 <= str[i] && str[i] <= 122) || //abc...xyz
        (str[i]=='~' || str[i]=='-' || str[i]=='_' || str[i]=='.')
        )
            url.append(&str[i], 1);
        else
        {
            url.append("%");
            url.append(chartohex(str[i]));//converts char 255 to string "FF"
        }
    }
    return url;
}
string Helper::url_decode(const string& url) {
    string str;
    int max = url.length();
    for(int i=0; i < max; i++)
    {
        if(url[i] != '%')
            str.append(&url[i], 1);
        else
        {
            str += hextochar(&url[i+1]);
            i+=2;
        }
    }
    return str;
}

int Helper::defaultResource(Request& request, Response& response, string root) {
    //Extract the resource from the URL, decode and correct path
    int i = request.url.find_first_of('?');
    if(i != string::npos)
        request.url = request.url.substr(0, i);
    request.url = url_decode(request.url);
    i = request.url.find("/../");
    while (i != string::npos)
    {
        if(i == 0) {
            response.code = 403;
            return 403; }
        int slash = request.url.find_last_of('/', i-1);
        request.url.erase(slash, i + 3 - slash);
        i = request.url.find(("/../"));
    }

    //Test if the resource exist
    if(!FileSystem::exist(root + request.url) || FileSystem::isDirectory(root + request.url)) {
        response.code = 404;
        return 404;
    }

    //Find the last modification time of the resource (set lmodif and lmodif_s)
    struct tm* tm;
    char lmodif_c [40];
    time_t t = FileSystem::mTimeFile(root + request.url);
    tm = gmtime(&t);
    strftime (lmodif_c,40,"%a, %d %b %Y %H:%M:%S GMT",tm);
    string lmodif(lmodif_c);

    //Test the IF-Modified-Since field
    string smodif = request.getHeaderField("If-Modified-Since");
    if(smodif == lmodif)
    {
        response.code = 304;
        response.send();
        return 0;
    }

    //Try open the file
    ifstream file((root + request.url).c_str(), ifstream::binary);
    if(!file.good()) {
        response.code = 500;
        return 500;
    }

    //Set the Content-Length header
    int s = FileSystem::sizeFile(root + request.url);
    char data[BUFFSIZE];
    response.setContent(&data[0], s);
    string type = FileSystem::getMimeType(request.url);
    if(type == "")
        response.addHeader("Content-Type", "application/octet-stream");
    else
        response.addHeader("Content-Type", type.c_str());

    //Set the Last-Modified header (using lmodif_c created upper)
    response.addHeader("Last-Modified", lmodif_c);
    response.sendHeader();

    //Send the resource
    for (s = s; s > BUFFSIZE; s = s - BUFFSIZE) {
        file.read(data, BUFFSIZE);
        response.setContent(&data[0], BUFFSIZE);
        response.send();
    }
    file.read(data, s);
    response.setContent(&data[0], s);
    response.send();

    file.close();
    return 0;
}
