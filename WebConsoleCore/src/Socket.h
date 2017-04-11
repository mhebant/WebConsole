/* ----Socket ver3.0(05/02/2016)----
Win 32 / Linux

Represent and control a socket (TCP or UDP) in server or client mode
Define WIN32 if compile for Windows 32 bit or LINUX if compile for Linux
Define SSLSOCKET for SSL supported socket (Not finished).

For Windows:
    Lib: -lws2_32
    
For SSL:
    Lib: -lmbedtls -lmbedx509 -lmbedcrypto
    
Errors:
>0      No Error
0       Timeout (Or no error)
-1      Socket error (call GetLastSocketError)
<-1     mbedtls error
*/

#ifndef SOCKET_H_INCLUDED
#define SOCKET_H_INCLUDED

#ifdef WIN32 /* Windows */
    #include <winsock2.h>
    #define socklen_t int
#elif defined LINUX /* Linux */
    #include <errno.h>
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h> /* close */
    #include <netdb.h> /* gethostbyname */
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define closesocket(s) close(s)
    typedef int SOCKET;
    typedef struct sockaddr_in SOCKADDR_IN;
    typedef struct sockaddr SOCKADDR;
    typedef struct in_addr IN_ADDR;
#else /* Not supported */
    #error Please define WIN32 or LINUX to compile
#endif

#ifdef SSLSOCKET
    #include "mbedtls/ssl.h"
    #include "mbedtls/net.h"
    #include "mbedtls/entropy.h"
    #include "mbedtls/ctr_drbg.h"
    #include "mbedtls/certs.h"

    #include <signal.h>
    
    #include <cstring>
#endif
    
typedef struct sockaddr_in SocketAddress;

class Socket
{
#ifdef SSLSOCKET
private:
    static mbedtls_entropy_context entropy;
    static mbedtls_ctr_drbg_context ctr_drbg;
#endif
    
public:
    static int init();
    static void end();

    static SocketAddress Address(u_long addr, int port);
    static int AddressSetIP(SocketAddress& address, u_long addr);
    static int AddressSetPort(SocketAddress& address, int port);
    static u_long AddressGetIP(SocketAddress& address);
    static int AddressGetPort(SocketAddress& address);

    static int GetLastSocketError();

    static const short TCP = 1;
    static const short UDP = 2;
    
    static const short CLIENT = 0;
    static const short SERVER = 1;

#ifdef SSLSOCKET
    struct SSLConfig {
        mbedtls_ssl_config conf;
        mbedtls_x509_crt srvcert;
        mbedtls_pk_context pkey;
        
        /*static void my_debug(void *ctx, int level, const char *file, int line, const char *str) {
            ((void) level);
            fprintf((FILE *) ctx, "%s:%04d: %s", file, line, str);
            fflush((FILE *) ctx); }*/
        
        SSLConfig() { mbedtls_ssl_config_init(&conf); mbedtls_x509_crt_init(&srvcert); mbedtls_pk_init(&pkey); }
        int LoadCrtFile(const char* crtfile) { return mbedtls_x509_crt_parse_file(&srvcert, crtfile); }
        int LoadCrtPath(const char* crtpath) { return mbedtls_x509_crt_parse_path(&srvcert, crtpath); }
        int Config(short type, short protocol, const char* pkeyfile) {
            int e;
            if ((e = mbedtls_ssl_config_defaults(&conf, type, protocol-1, MBEDTLS_SSL_PRESET_DEFAULT)) != 0 )
                return e;
            mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);
            //mbedtls_ssl_conf_dbg(&conf, my_debug, stdout);
            //mbedtls_ssl_conf_ca_chain(&conf, srvcert.next, NULL);
            if ((e = mbedtls_pk_parse_keyfile(&pkey, pkeyfile, NULL)) != 0)
                return e;
            if ((e = mbedtls_ssl_conf_own_cert(&conf, &srvcert, &pkey)) != 0)
                return e;
            return 0;
        }
        ~SSLConfig() { mbedtls_ssl_config_free(&conf); mbedtls_x509_crt_free(&srvcert); mbedtls_pk_free(&pkey); }
    };
#endif
    
private:
    SOCKET sock;
    struct timeval timeout;
#ifdef SSLSOCKET
    mbedtls_ssl_context* ssl = 0;
#endif
    
public:
    Socket(short protocol);
    Socket(SOCKET sock);
    Socket(short protocol, u_long addr, int port);
    ~Socket();

    int BindAddress(SocketAddress address); /* Bind the socket withe the specified address */
    SocketAddress GetAddress(); /* Get the socket address */

    int SetTimeout(long millisec); /* Set timeout in millisec, no timeout if 0 */
    bool IsValid(); /* Test if the socket is a valid one */
    int Listen(int num); /* Prepare the socket to listen with the num pendding queue */
    Socket Accept(); /* Wait for connection and accept it */
    int Connect(u_long addr, int port);
    int Recive(char* data, int length); /* Wait for data and store them in data */
    int ReciveFrom(char* data, int length, SocketAddress& address);
    int Send(const char* data, int length); /* Send data */
    int SendTo(const char* data, int length, const SocketAddress& address);
    void Close(); /* Close the socket (the connection) */
#ifdef SSLSOCKET
    int SSL(SSLConfig& sslconfig);
#endif
};

#endif // SOCKET_H_INCLUDED
