// Routines for debugging network code
//
// 4/1999, -kuk-

#include <stdio.h>
#include "NetTools.h"
#include "MsgLogger.h"

// Convert the given address into a string "<IP> (<port>)"
void GetSocketAddrInfo(const sockaddr_in &sin, stdString &text)
{
    char buffer[200];

    switch (sin.sin_family)
    {
        case AF_INET:
#ifdef WIN32
            if (sin.sin_addr.S_un.S_addr == htonl(INADDR_ANY))
#else
                if (sin.sin_addr.s_addr == htonl(INADDR_ANY))
#endif
                    sprintf(buffer, "IP <INADDR_ANY>: %d", ntohs(sin.sin_port));
                else
                    sprintf(buffer, "IP %s (%d)",
                            inet_ntoa(sin.sin_addr), ntohs(sin.sin_port));
            break;
        default:
            sprintf(buffer, "Unknown address family: 0x%X", sin.sin_family);
    }
    text = buffer;
}

void GetSocketLocal(SOCKET sock, stdString &local)
{
    sockaddr_in sin;
    socklen_t   len = sizeof(sockaddr_in);
    if (getsockname(sock, (sockaddr *) &sin, &len) == 0)
        GetSocketAddrInfo(sin, local);
    else
        local = "<undefined>";
}

void GetSocketPeer(SOCKET sock, stdString &peer)
{
    sockaddr_in sin;
    socklen_t   len = sizeof(sockaddr_in);
    if (getpeername(sock, (sockaddr *) &sin, &len) == 0)
        GetSocketAddrInfo(sin, peer);
    else
        peer = "<undefined>";
}

// Get info for socket: local address, peer's address
void GetSocketInfo(SOCKET sock, stdString &local, stdString &peer)
{
    GetSocketLocal(sock, local);
    GetSocketPeer(sock, peer);
}

#ifdef WIN32
// Create a broadcast socket, bound to port.
// Returns a sockaddr for broadcasting also.
bool CreateBroadcastSocket(int port /*I*/, SOCKET &sock /*O*/, 
                           int peer_port, sockaddr_in &broadcast /*O*/)
{
    // Create Socket
    sock = epicsSocketCreate(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET)
        return false;

    // Allow broadcasts from this socket
    int on=true;
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST,
                   (const char*)&on, sizeof(on)) != 0)
    {
        epicsSocketDestroy(sock);
        return false;
    }

    // Bind socket to given port on local machine
    broadcast.sin_port = htons(port);
    broadcast.sin_family = AF_INET;
    broadcast.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
    if (bind(sock, (const sockaddr *)&broadcast, sizeof(struct sockaddr_in))
        == SOCKET_ERROR)
    {
        epicsSocketDestroy(sock);
        return false;
    }

    // Try to find a broadcast address for this socket
#ifdef WIN32
    DWORD bytes_returned;
    if (WSAIoctl(sock, SIO_GET_BROADCAST_ADDRESS,
                 NULL, 0, (LPVOID) &broadcast, sizeof(struct sockaddr_in),
                 &bytes_returned, NULL, NULL)  != 0)
    {
        LOG_MSG("WSAIoctl (GET_BROADCAST_ADDRESS) failed: %s, "
                "using INADDR_BROADCAST\n",
                GetSocketErrorString());
        broadcast.sin_addr.S_un.S_addr = htonl(INADDR_BROADCAST);
    }
    broadcast.sin_port = htons(peer_port);
#else
#error Try to implement this for Non-Win32!
#endif

    return true;
}

//    Note:
//    Error codes can be found in .....include/winsock.h
const char *GetSocketErrorString()
{
    DWORD    err = WSAGetLastError();

#define HANDLE(txt)\
    case txt: return #txt;

    switch (err)
    {
        HANDLE(WSANOTINITIALISED)
            HANDLE(WSAENETDOWN)
            HANDLE(WSAEADDRINUSE)
            HANDLE(WSAEINPROGRESS)
            HANDLE(WSAEADDRNOTAVAIL)
            HANDLE(WSAEAFNOSUPPORT)
            HANDLE(WSAECONNREFUSED)
            HANDLE(WSAEDESTADDRREQ)
            HANDLE(WSAEFAULT)
            HANDLE(WSAEINVAL)
            HANDLE(WSAEISCONN)
            HANDLE(WSAEMFILE)
            HANDLE(WSAENETUNREACH)
            HANDLE(WSAENOBUFS)
            HANDLE(WSAENOTSOCK)
            HANDLE(WSAETIMEDOUT)
            HANDLE(WSAEMSGSIZE)
            HANDLE(WSAEWOULDBLOCK)
            HANDLE(WSAEACCES)
            default:
        return "unknown";
    }
}
#endif
