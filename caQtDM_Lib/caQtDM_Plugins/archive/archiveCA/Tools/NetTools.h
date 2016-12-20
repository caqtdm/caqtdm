#ifndef __NETTOOLS_H__
#define __NETTOOLS_H__

// Tools for network code
//
// 4/1999, -kuk-

#include <ToolsConfig.h>
#ifdef solaris
// Hack around clash of struct map in inet headers with std::map
#define map xxxMapxxx
#endif
#include <osiSock.h>
#ifdef solaris
#undef map
#endif
#ifdef Linux
#include<sys/types.h>
#include<sys/socket.h>
#endif

// Convert the given address into a string "<IP> (<port>)"
void GetSocketAddrInfo (const sockaddr_in &sin, stdString &text);

// Get info for socket: local address, peer's address
void GetSocketLocal (SOCKET sock, stdString &local);
void GetSocketPeer (SOCKET sock, stdString &peer);
void GetSocketInfo (SOCKET sock, stdString &local, stdString &peer);

#ifdef WIN32
// Create a broadcast socket, bound to port.
// Returns a sockaddr for broadcasting also.
bool CreateBroadcastSocket (int port /*I*/, SOCKET &sock /*O*/, 
                            int peer_port, sockaddr_in &broadcast /*O*/);

// Get description for the last error
const char *GetSocketErrorString ();
#endif

#endif //__NETTOOLS_H__
