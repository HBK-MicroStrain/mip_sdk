
#include "tcp_socket.h"

#include "microstrain/logging.h"

#ifdef MICROSTRAIN_PLATFORM_WINDOWS

#include <winsock2.h>
#include <ws2tcpip.h>

static const int SEND_FLAGS = 0;

#ifdef _MSC_VER
typedef int ssize_t;
#endif

#else

#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netdb.h>
#include <string.h>

static const int INVALID_SOCKET = -1;
static const int SEND_FLAGS = MSG_NOSIGNAL;

#endif

#include <stdio.h>

void tcp_socket_init(tcp_socket* socket_ptr)
{
    socket_ptr->handle = INVALID_SOCKET;
}

bool tcp_socket_is_open(const tcp_socket* socket_ptr)
{
    return socket_ptr->handle != INVALID_SOCKET;
}

static bool tcp_socket_open_common(tcp_socket* socket_ptr, const char* hostname, uint16_t port, unsigned int timeout_ms)
{
    //assert(socket_ptr->handle == INVALID_SOCKET);

    // https://man7.org/linux/man-pages/man3/getaddrinfo.3.html
    struct addrinfo hints, *info;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_INET; // AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags    = 0;

    char port_str[6];  // Maximum 5 digits
    snprintf(port_str, sizeof(port_str), "%u", (unsigned int)port);

    int result = getaddrinfo(hostname, port_str, &hints, &info);
    if( result != 0 )
        return false;

    for(struct addrinfo* addr=info; addr!=NULL; addr=addr->ai_next)
    {
        socket_ptr->handle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); //socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
        if( socket_ptr->handle == INVALID_SOCKET )
            continue;

        if( connect(socket_ptr->handle, addr->ai_addr, (int)addr->ai_addrlen) == 0 )
            break;

#ifdef MICROSTRAIN_PLATFORM_WINDOWS
        closesocket(socket_ptr->handle);
#else
        close(socket_ptr->handle);
#endif
        socket_ptr->handle = INVALID_SOCKET;
    }

    freeaddrinfo(info);

    if( socket_ptr->handle == INVALID_SOCKET )
        return false;

#ifdef MICROSTRAIN_PLATFORM_WINDOWS
    if( setsockopt(socket_ptr->handle, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout_ms, sizeof(timeout_ms)) != 0 )
    {
        tcp_socket_close(socket_ptr);
        return false;
    }

    if( setsockopt(socket_ptr->handle, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout_ms, sizeof(timeout_ms)) != 0 )
    {
        tcp_socket_close(socket_ptr);
        return false;
    }
#else
    struct timeval timeout_option;
    timeout_option.tv_sec  = timeout_ms / 1000;
    timeout_option.tv_usec = (timeout_ms % 1000) * 1000;

    if( setsockopt(socket_ptr->handle, SOL_SOCKET, SO_RCVTIMEO, &timeout_option, sizeof(timeout_option)) != 0 )
    {
        tcp_socket_close(socket_ptr);
        return false;
    }

    if( setsockopt(socket_ptr->handle, SOL_SOCKET, SO_SNDTIMEO, &timeout_option, sizeof(timeout_option)) != 0 )
    {
        tcp_socket_close(socket_ptr);
        return false;
    }
#endif

    return true;
}

bool tcp_socket_open(tcp_socket* socket_ptr, const char* hostname, uint16_t port, unsigned int timeout_ms)
{
    MICROSTRAIN_LOG_TRACE(
        "tcp_socket_open: socket=%p, hostname=%s, port=%u, timeout=%u",
        socket_ptr->handle,
        hostname,
        port,
        timeout_ms
    );

#ifdef MICROSTRAIN_PLATFORM_WINDOWS

    // Initialize winsock for each connection since there's no global init function.
    // This is safe to do multiple times, as long as it's shutdown the same number of times.
    struct WSAData wsaData;
    int result = WSAStartup(MAKEWORD(2,2), &wsaData);
    if(result != 0)
    {
        MICROSTRAIN_LOG_ERROR("WSAStartup() failed: %d\n", result);
        return false;
    }

#endif

    if(!tcp_socket_open_common(socket_ptr, hostname, port, timeout_ms))
    {
#ifdef MICROSTRAIN_PLATFORM_WINDOWS
        WSACleanup();
#endif
        return false;
    }

    return true;
}

bool tcp_socket_shutdown(tcp_socket* socket_ptr)
{
    MICROSTRAIN_LOG_TRACE(
        "tcp_socket_shutdown: socket=%p", socket_ptr->handle
    );

    if( socket_ptr->handle == INVALID_SOCKET )
        return false;

#ifdef MICROSTRAIN_PLATFORM_WINDOWS
    shutdown(socket_ptr->handle, SD_BOTH);
#else
    shutdown(socket_ptr->handle, SHUT_RDWR);
#endif

    return true;
}

bool tcp_socket_close(tcp_socket* socket_ptr)
{
    MICROSTRAIN_LOG_TRACE(
        "tcp_socket_close: socket=%p", socket_ptr->handle
    );

    if( socket_ptr->handle == INVALID_SOCKET )
        return false;

#ifdef MICROSTRAIN_PLATFORM_WINDOWS
    closesocket(socket_ptr->handle);
    WSACleanup(); // See tcp_socket_open
#else
    close(socket_ptr->handle);
#endif

    socket_ptr->handle = INVALID_SOCKET;
    return true;
}

bool tcp_socket_send(tcp_socket* socket_ptr, const void* buffer, size_t num_bytes, size_t* bytes_written)
{
    MICROSTRAIN_LOG_TRACE(
        "tcp_socket_send: socket=%p, buffer=%p, num_bytes=%zu",
        socket_ptr->handle,
        buffer,
        num_bytes
    );

    if(socket_ptr == NULL || !tcp_socket_is_open(socket_ptr) || (buffer == NULL && num_bytes > 0) || bytes_written == NULL)
        return false;

    for(*bytes_written = 0; *bytes_written < num_bytes; )
    {
        const unsigned char* data = (const unsigned char*)buffer;
        ssize_t sent = send(socket_ptr->handle, data + *bytes_written, (int)(num_bytes - *bytes_written), SEND_FLAGS);
        if(sent < 0)
            return false;

        if(sent == 0)
            return false;

        *bytes_written += sent;
    }
    return true;
}

bool tcp_socket_recv(tcp_socket* socket_ptr, void* buffer, size_t num_bytes, size_t* bytes_read)
{
    MICROSTRAIN_LOG_TRACE(
        "tcp_socket_recv: socket=%p, buffer=%p, num_bytes=%zu",
        socket_ptr->handle,
        buffer,
        num_bytes
    );

    if(socket_ptr == NULL || !tcp_socket_is_open(socket_ptr) || (buffer == NULL && num_bytes > 0) || bytes_read == NULL)
        return false;

    if(num_bytes == 0)
        return true;

    *bytes_read = 0;
    ssize_t local_bytes_read = recv(socket_ptr->handle, buffer, (int)num_bytes, 0);

    if( local_bytes_read < 0 )
    {
#ifdef MICROSTRAIN_PLATFORM_WINDOWS
        return false;
#else
        if(errno != EAGAIN && errno != EWOULDBLOCK)
            return false;
        else
            return true;
#endif
    }
    // Throw an error if the connection has been closed by the other side.
    else if( local_bytes_read == 0 )
        return false;

    *bytes_read = (size_t)local_bytes_read;
    return true;
}

bool tcp_socket_listen(tcp_socket* socket_ptr, const char* interface_name, uint16_t port, unsigned int backlog)
{
    MICROSTRAIN_LOG_TRACE(
        "tcp_socket_listen: interface_name=%s, port=%u, backlog=%u",
        interface_name,
        port,
        backlog
    );

    if(socket_ptr->handle != INVALID_SOCKET || interface_name == NULL || port == 0)
    {
        MICROSTRAIN_LOG_DEBUG("tcp_socket_listen: socket already open or invalid parameters.\n");
        return false;
    }

    if(backlog == 0)
        backlog = SOMAXCONN;

#ifdef MICROSTRAIN_PLATFORM_WINDOWS
    struct WSAData wsa_data;
    if(WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0)
        return false;
#endif

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    char port_str[6];
    snprintf(port_str, sizeof(port_str), "%u", port);

    struct addrinfo* info = NULL;

    if(getaddrinfo(interface_name, port_str, &hints, &info) == 0)
    {
        for(struct addrinfo* addr=info; addr!=NULL; addr=addr->ai_next)
        {
            MICROSTRAIN_LOG_TRACE(
                "tcp_socket_listen: ai_family=%d, ai_socktype=%d, ai_protocol=%d",
                addr->ai_family,
                addr->ai_socktype,
                addr->ai_protocol
            );

            socket_ptr->handle = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
            if(socket_ptr->handle == INVALID_SOCKET)
            {
                MICROSTRAIN_LOG_WARN("tcp_socket_listen: socket creation failed: %d %s", errno, strerror(errno));
                continue;
            }

            if(bind(socket_ptr->handle, info->ai_addr, (int)info->ai_addrlen) != 0)
            {
                MICROSTRAIN_LOG_WARN("tcp_socket_listen: bind failed: %d %s", errno, strerror(errno));
                close(socket_ptr->handle);
                socket_ptr->handle = INVALID_SOCKET;
                continue;
            }

            if(listen(socket_ptr->handle, backlog) != 0)
            {
                MICROSTRAIN_LOG_WARN("tcp_socket_listen: listen failed: %d %s", errno, strerror(errno));
                close(socket_ptr->handle);
                socket_ptr->handle = INVALID_SOCKET;
                continue;
            }

            break;
        }

        freeaddrinfo(info);
        info = NULL;

        if(socket_ptr->handle != INVALID_SOCKET)
            return true;
    }

#ifdef MICROSTRAIN_PLATFORM_WINDOWS
    WSACleanup();
#endif

    return false;
}

bool tcp_socket_accept(tcp_socket* socket_ptr, tcp_socket* socket_out)
{
    MICROSTRAIN_LOG_TRACE("tcp_socket_accept: %p\n", socket_ptr->handle);

    socket_out->handle = accept(socket_ptr->handle, NULL, NULL);

    return socket_out->handle != INVALID_SOCKET;
}
