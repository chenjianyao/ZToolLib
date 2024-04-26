#ifndef _ZTL_TCP_SERVER_H_
#define _ZTL_TCP_SERVER_H_

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

#include "ztl_protocol.h"
#include "ztl_threads.h"
#include "ztl_evloop.h"


/* tcp server object declare */
typedef struct ztl_tcp_server_st ztl_tcp_server_t;

/* tcp server event handler */
typedef int(*ztl_tcp_server_ns_ptr)(ztl_tcp_server_t* tcpsvr, sockhandle_t fd);
typedef int(*ztl_tcp_server_event_ptr)(ztl_tcp_server_t* tcpsvr, uint32_t type, void* msg, uint32_t length);

/* protocol type for tcp server */
typedef enum
{
    ZTL_PROTO_None,                         /* none protocol */
    ZTL_PROTO_ZHeader,                      /* user ZHeader protocol */
    ZTL_PROTO_Custom                        /* customize protocol */
}ztl_proto_type;

/* config for tcp server */
typedef struct  
{
    sockhandle_t    listen_fd;              /* the listen fd could be created by outside */
    char            listen_ip[32];          /* listen ip address, leave empty means INADDR_ANY */
    uint16_t        listen_port;            /* listen port */
    uint16_t        work_threads;           /* io worker threads number */

    uint8_t         reuse_addr;             /* sock option */
    uint8_t         tcp_nodelay;            /* sock option */

    uint32_t        poll_timeout;           /* poll timeout ms */

    ztl_tcp_server_ns_ptr       newconn_handler;
    ztl_tcp_server_event_ptr    event_handler;

    ztl_proto_type  proto;                  /* the protocol type */
    uint32_t        head_size;              /* the protocol header size */
    uint32_t        size_var_off_in_head;   /* the header's body size variable offset the header address */
}ztl_tcp_server_config_t;


struct ztl_tcp_server_st
{
    ztl_evloop_t*           evloop;
    ztl_tcp_server_config_t svrconf;
    ztl_thread_t            thd;

    sockhandle_t            listenfd;
    int32_t                 inited;
    uint32_t                running;
};


/* create a tcp server object
 */
int ztl_tcp_server_create(ztl_tcp_server_t** ptcpsvr);

/* release the tcp server object
 */
int ztl_tcp_server_release(ztl_tcp_server_t* tcpsvr);

/* init the tcp server
 */
int ztl_tcp_server_init(ztl_tcp_server_t* tcpsvr, ztl_tcp_server_config_t* config);

/* start the tcp server
 */
int ztl_tcp_server_start(ztl_tcp_server_t* tcpsvr);
int ztl_tcp_server_start_no_thread(ztl_tcp_server_t* tcpsvr);

/* stop the tcp server
 */
int ztl_tcp_server_stop(ztl_tcp_server_t* tcpsvr);


#ifdef __cplusplus
}
#endif//__cplusplus


#endif//_ZTL_TCP_SERVER_H_
