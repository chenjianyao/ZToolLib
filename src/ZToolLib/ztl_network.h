#ifndef _ZTL_NETWORK_H_
#define _ZTL_NETWORK_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef _WIN32

#undef  FD_SETSIZE
#define FD_SETSIZE 1024

#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

typedef int             socklen_t;
typedef SOCKET          sockhandle_t;

typedef struct _WSABUF  EIOVEC;
#define EIOVEC_BASE     buf
#define EIOVEC_LEN      len

#else
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/poll.h>
#include <sys/epoll.h>
#include <sys/uio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <net/if.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>

/// ignore sigpipe
void ignore_sigpipe();
#define INVALID_SOCKET  (-1)

typedef int             sockhandle_t;

typedef struct iovec    EIOVEC;
#define EIOVEC_BASE     iov_base
#define EIOVEC_LEN      iov_len

#endif//WIN32

#define IS_VALID_SOCKET(fd)     ((fd) != -1 && (fd) != 0)


#ifdef __cplusplus
extern "C" {
#endif

/// network start only work on windows platform
void net_init();
void net_cleanup();

/// errno relative
const char* get_strerror(int no);
int  get_errno();
bool is_wouldblock(int nErrno);
bool is_einterrupt(int nErrno);

/// create a socket, socktype: SOCK_STREAM, SOCK_DGRAM, SOCK_RAW
sockhandle_t create_socket(int socktype);

/// close the socket
void close_socket(sockhandle_t sockfd);

/// shutdown the socket, how: SHUT_RD-0, SHUT_WT-1, SHUT_RDWT-2
int shutdown_socket(sockhandle_t sockfd, int how);

/// set non-block
int set_nonblock(sockhandle_t sockfd, bool on);

/// set reuse addr
int set_reuseaddr(sockhandle_t sockfd, bool on);

/// set tcp nodelay
int set_tcp_nodelay(sockhandle_t sockfd, bool on);

/// set tcp keep alive
int set_tcp_keepalive(sockhandle_t sockfd, bool on);

/// set close on exec
int set_closeonexec(sockhandle_t sockfd);

/// set broadcast property for udp
int set_broadcast(sockhandle_t sockfd, bool on);

/// multicast operations
int join_multicast(sockhandle_t sockfd, const char* multiip, const char* bindip);
int leave_multicast(sockhandle_t sockfd, const char* multiip, const char* bindip);
int set_multicast_interface(sockhandle_t sockfd, const char* bindip);
int enable_multicast_loopback(sockhandle_t sockfd, bool enable);
int set_multicast_ttl(sockhandle_t sockfd, int ttl);

/// set sock buffer size
int set_rcv_buffsize(sockhandle_t sockfd, int bufsize);
int set_snd_buffsize(sockhandle_t sockfd, int bufsize);

/// set sock timeout with milli-second
int set_rcv_timeout(sockhandle_t sockfd, int timeout_ms);
int set_snd_timeout(sockhandle_t sockfd, int timeout_ms);

/// get local or peer socket's address
int get_localaddr(sockhandle_t sockfd, struct sockaddr_in* localaddr);
int get_peeraddr(sockhandle_t sockfd, struct sockaddr_in* peeraddr);

/// get ip and port from sa
int get_ipport(char* ip, int size, uint16_t* port, const struct sockaddr_in* psa);

/// make sockaddr by ip and port
int make_sockaddr(struct sockaddr_in* psa, const char* ip, uint16_t port);

/// get inet address from ip string
uint32_t string_to_inetaddr(const char* ip);
int inetaddr_to_string(char* ip, int size, uint32_t addr);

/// get local machine's ip
int get_local_ip(char* ip, int size);

/// get socket error
int get_socket_error(sockhandle_t sockfd, int* perror);

/// peek n bytes from socket's buffer by MSG_PEEK flag
int tcp_msg_peek(sockhandle_t sockfd, char* buf, int len);

/// accept a new socket descriptor
sockhandle_t tcp_accept(sockhandle_t listenfd, struct sockaddr_in* fromaddr);
sockhandle_t tcp_accept2(sockhandle_t listenfd, char fromip[], int sz, uint16_t* port);

/// try detect events, the event fds will put front at sockfds array, and return count
int poll_reads(sockhandle_t sockfds[], int nfds, int timeout_ms);
int poll_read(sockhandle_t sockfd, int timeout_ms);

int poll_writes(sockhandle_t sockfds[], int nfds, int timeout_ms);
int poll_write(sockhandle_t sockfd, int timeout_ms);

/// send iovec, return send count
int send_iov(sockhandle_t sockfd, EIOVEC* iovec, int iovec_cnt);

//////////////////////////////////////////////////////////////////////////
/// connect to server with a blocking socket
int net_connect(sockhandle_t sockfd, const char* ip, uint16_t port);

/// non-block connect to server, timeout is milli-second
int net_connect_nonb(sockhandle_t sockfd, const char* ip, uint16_t port, int timeoutms);

/// pass a tcp socket desc and make listening, return 0 if listen success
int tcp_listen(sockhandle_t listenfd, const char* ip, uint16_t port, bool reuse, int backlog/* = SOMAXCONN*/);

sockhandle_t tcp_listen_ex(const char* bindip, uint16_t port, bool nonblock, bool nodelay);

/// read count bytes from socket
int tcp_readn(sockhandle_t sockfd, char* buf, int count);

/// make a simple tcp server, if new event, the callback functions will be invoked
typedef int (*pfonevent)(void* udata, sockhandle_t fd, int isoutev);
int tcp_simple_server(sockhandle_t listenfd, pfonevent eventcb, void* udata);

/// make a tcp echo server
int tcp_echo_server(const char* listenip, uint16_t listenport);

/// write data
int net_send(sockhandle_t sockfd, const char* buf, int size, int flag);

/// create a udp receiver, return the udp socket descriptor
sockhandle_t udp_receiver(const char* localip, uint16_t localport, bool reuseaddr);

/// udp recv data with timeout, and fromaddr could be null
/// @returns 0 timeout, -1 error, else received length, timeout is micro-seconds
int udp_recv(sockhandle_t sockfd, char* buf, int size, struct sockaddr_in* fromaddr, int timeoutms);

/// udp send data
int udp_send(sockhandle_t sockfd, const char* buf, int len, struct sockaddr_in* toaddr);
int udp_sendex(sockhandle_t sockfd, const char* buf, int len, const char* ip, uint16_t port);

/// make a udp echo server
typedef bool(*pfonrecv)(sockhandle_t sockfd, char* buf, int length, struct sockaddr_in* peeraddr);
int udp_echo_server(const char* localip, uint16_t localport, pfonrecv msgcallback);

/// make a pair of socket descriptors, type is SOCK_STREAM or SOCK_DGRAM
int make_sockpair(sockhandle_t sockfds[], int type);

#ifdef __cplusplus
}
#endif

#endif//_ZTL_NETWORK_H_
