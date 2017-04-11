/*  ----Socket ver3.0(05/02/2016)---- */
#include "Socket.h"

using namespace std;

#ifdef SSLSOCKET
    mbedtls_entropy_context Socket::entropy;
    mbedtls_ctr_drbg_context Socket::ctr_drbg;
#endif

int Socket::init() {
#ifdef WIN32
    WSADATA wsa;
    if(WSAStartup(MAKEWORD(2, 2), &wsa) < 0)
    {
        return -1;
    }
#endif

#ifdef SSLSOCKET
    //SLL Entropy
    mbedtls_ctr_drbg_init(&ctr_drbg);
    mbedtls_entropy_init(&entropy);
    const char* custom = "customseedgenerator";
    int e;
    if ((e = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, (const unsigned char*)custom, strlen(custom))) != 0)
        return e;

    //To avoid SIGPIE signal when peer closing connection prematuraly with SSL
    signal(SIGPIPE, SIG_IGN);
#endif
    
    return 0;
}

void Socket::end() {
#ifdef WIN32
    WSACleanup();
#endif
    
#ifdef SSLSOCKET
    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_entropy_free(&entropy);
#endif
}

SocketAddress Socket::Address(u_long addr, int port) {
    SocketAddress address = {0};
    address.sin_family = AF_INET;
    AddressSetIP(address, addr);
    AddressSetPort(address, port);
    return address;
}
int Socket::AddressSetIP(SocketAddress& address, u_long addr) {
    address.sin_addr.s_addr = htonl(addr);
    return 0;
}
int Socket::AddressSetPort(SocketAddress& address, int port) {
    address.sin_port = htons(port);
    return 0;
}
u_long Socket::AddressGetIP(SocketAddress& address) {
    return ntohl(address.sin_addr.s_addr);
}
int Socket::AddressGetPort(SocketAddress& address) {
    return ntohs(address.sin_port);
}

int Socket::GetLastSocketError() {
#ifdef WIN32
    return WSAGetLastError();
#elif defined (LINUX)
    return errno;
#endif
}




Socket::Socket(short protocol) {
    sock = socket(AF_INET, protocol, 0);
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
}
Socket::Socket(SOCKET so) : sock(so) {
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
}
Socket::Socket(short protocol, u_long addr, int port) : Socket(protocol) {
    if (IsValid())
    {
        SocketAddress address = Address(addr, port);
        BindAddress(address);
    }
}

Socket::~Socket() {
    //Close();
#ifdef SSLSOCKET
    delete ssl;
#endif
}

int Socket::BindAddress(SocketAddress address) {
    if(bind(sock, (SOCKADDR*) &address, sizeof address) == SOCKET_ERROR)
    {
        return -1;
    }
    return 0;
}

SocketAddress Socket::GetAddress() {
    SocketAddress address = {};
    int len = sizeof address;
    if (getsockname(sock, (SOCKADDR*) &address, (socklen_t*)&len) < 0)
    {
        return {0};
    }
    return address;
}

int Socket::SetTimeout(long millisec) {
    timeout.tv_sec = millisec/1000;
    timeout.tv_usec = (millisec%1000)*1000;
    return 0;
}

bool Socket::IsValid() {
    return sock != INVALID_SOCKET;
}

int Socket::Listen(int num) {
    if(listen(sock, num)  == SOCKET_ERROR)
    {
         return -1;
    }
    return 0;
}

Socket Socket::Accept() {
    if(timeout.tv_usec > 0 || timeout.tv_sec > 0)
    {
        fd_set readfs;
        FD_ZERO(&readfs);
        FD_SET(sock, &readfs);
        timeval ctimeout = timeout;
        int s = select(sock + 1, &readfs, NULL, NULL, &ctimeout);
        if(s < 0)
        {
            return Socket(INVALID_SOCKET);
        }
        if(s == 0)
            return Socket(INVALID_SOCKET);
    }

    SocketAddress clientAddress = { 0 };
    SOCKET csock;
    int len = sizeof clientAddress;
    csock = accept(sock, (SOCKADDR*) &clientAddress, (socklen_t*)&len);

    return Socket(csock);
}

int Socket::Connect(u_long addr, int port) {
    SocketAddress address = { 0 };
    address.sin_addr.s_addr = htonl(addr);
    address.sin_family = AF_INET;
    address.sin_port = htons(port);

    if(connect(sock,(SOCKADDR*) &address, sizeof address) == SOCKET_ERROR)
    {
        return -1;
    }
    return 0;
}

int Socket::Recive(char* data, int length) {
    if(timeout.tv_usec > 0 || timeout.tv_sec > 0)
    {
        fd_set readfs;
        FD_ZERO(&readfs);
        FD_SET(sock, &readfs);
        timeval ctimeout = timeout;
        int s = select(sock + 1, &readfs, NULL, NULL, &ctimeout);
        if(s < 0)
        {
            return -1;
        }
        if(s == 0)
            return 0;
    }
    
#ifdef SSLSOCKET
    if(ssl != 0)
        return mbedtls_ssl_read(ssl, (unsigned char*)data, length);
#endif
    return recv(sock, data, length, 0);
}

int Socket::ReciveFrom(char* data, int length, SocketAddress& address) {
    if(timeout.tv_usec > 0 || timeout.tv_sec > 0)
    {
        fd_set readfs;
        FD_ZERO(&readfs);
        FD_SET(sock, &readfs);
        timeval ctimeout = timeout;
        int s = select(sock + 1, &readfs, NULL, NULL, &ctimeout);
        if(s < 0)
        {
            return -1;
        }
        if(s == 0)
            return 0;
    }

    int len = sizeof address;
    return recvfrom(sock, data, length, 0, (SOCKADDR*) &address, (socklen_t*)&len);
}

int Socket::Send(const char* data, int length) {
#ifdef SSLSOCKET
    if (ssl != 0) {
        int ret = mbedtls_ssl_write(ssl, (const unsigned char*)data, length);
        if (ret > 0 && ret < length)
            return mbedtls_ssl_write(ssl, (const unsigned char*)data+ret, length-ret);
        else
            return ret;
    }
#endif
    
    if(send(sock, data, length, 0) < 0)
    {
        return -1;
    }
    return 0;
}

int Socket::SendTo(const char* data, int length, const SocketAddress& address) {
    if(sendto(sock, data, length, 0, (SOCKADDR*) &address, sizeof address) < 0)
    {
        return -1;
    }
    return 0;
}

void Socket::Close() {
#ifdef SSLSOCKET
    mbedtls_ssl_close_notify(ssl);
    mbedtls_ssl_free(ssl);
#endif    
    closesocket(sock);
}

#ifdef SSLSOCKET
int Socket::SSL(SSLConfig& sslconfig) {
    ssl = new mbedtls_ssl_context();
    mbedtls_ssl_init(ssl);
    
    int e;
    if ((e = mbedtls_ssl_setup( ssl, &(sslconfig.conf))) != 0)
        return e;
    
    mbedtls_ssl_session_reset(ssl);
    
    mbedtls_ssl_set_bio( ssl, &sock, mbedtls_net_send, mbedtls_net_recv, NULL);
    
    if ((e = mbedtls_ssl_handshake(ssl)) != 0) //while...
        return e;
        
    return 0;
}
#endif