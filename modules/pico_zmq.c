/*********************************************************************
   PicoTCP. Copyright (c) 2012 TASS Belgium NV. Some rights reserved.
   See LICENSE and COPYING for usage.

   Authors: Stijn Haers, Mathias Devos, Gustav Janssens, Sam Van Den Berge
 *********************************************************************/

#include "stdint.h"
#include "pico_stack.h"
#include "pico_config.h"
#include "pico_ipv4.h"
#include "pico_socket.h"
#include "pico_config.h"

#include "pico_vector.h"

#include "pico_zmq.h"
#include "pico_zmtp.h"

#undef dbg
#define dbg(x,args...) printf("[%s:%s:%i] "x" \n",__FILE__,__func__,__LINE__ ,##args )

/*
static void zmq_zmtp_add(void* socket, struct zmtp_socket* z)
{

}

static void zmq_zmtp_socket_del(struct zmtp_socket* z)
{

}
*/

static void cb_zmtp_sockets(uint16_t ev, struct zmtp_socket* s) 
{
    dbg("In cb_zmtp_sockets!");
    //TODO: process events!!
}

void* zmq_socket(void* context, int type)
{
    struct zmq_socket_base* sock = NULL;
    switch(type)
    {
        case(ZMTP_TYPE_REQ): 
            sock = pico_zalloc(sizeof(struct zmq_socket_req));
            ((struct zmq_socket_req *)sock)->send_enable = ZMQ_SEND_ENABLED;
            break;
        case(ZMTP_TYPE_REP):
            break; 
        case(ZMTP_TYPE_PUB):
            break;
        default:
            pico_free(sock);
            return NULL;
    }
    
    if(!sock) 
    {
        //pico_err = PICO_ERR_ENOMEM;
        return NULL;
    }

    sock->type = type;
        
    sock->sock = zmtp_socket_open(PICO_PROTO_IPV4, PICO_PROTO_TCP, type, &cb_zmtp_sockets);
    
    if(!sock->sock) {
        pico_free(sock);
        return NULL;
    }
    
    /* Init the pico_vector that is going to be used */
    pico_vector_init(&sock->in_vector, 5, sizeof(struct zmq_msg_t));
    pico_vector_init(&sock->out_vector, 5, sizeof(struct zmq_msg_t));

    return sock; 
}

int zmq_bind(void* socket, char* address, uint16_t port)
{
    return 0;
}

int zmq_connect(void* socket, const char* endpoint)
{
    struct zmq_socket_base *base = NULL;
    
    if(!socket || !endpoint)
        return -1;
        //TODO: error handling! => EINVAL

    //TODO: parse endpoint!!!
    base = (struct zmq_socket_base *)socket;
    
    pico_string_to_ipv4("10.40.0.1", &base->addr.addr);
    return zmtp_socket_connect(base->sock, &base->addr.addr, short_be(5555));
}

int zmq_send(void* socket, void* buf, size_t len, int flags)
{
    struct zmtp_frame_t* frame = NULL;
    struct zmq_socket_base* bsock = NULL;

    if(!socket)
        return -1;

    frame = pico_zalloc(sizeof(struct zmtp_frame_t));

    if(!frame)
        return -1;

    frame->buf = pico_zalloc(len);

    if(!frame->buf)
        return -1;

    memcpy(frame->buf, buf, len);
    frame->len = len;

    bsock = (struct zmq_socket_base *)socket;
    

    if(bsock->type == ZMTP_TYPE_REQ && ((struct zmq_socket_req *)bsock)->send_enable == ZMQ_SEND_DISABLED )
            return -1; //For REQ, if send_enable is disabled, then return -1
    
    /* Multi-part messages are described here: http://zguide.zeromq.org/page:all#Multipart-Messages */
    if( (flags & ZMQ_SNDMORE) != 0)
    {
        /* More frames to come. Just add into pico_vector and wait for a later call with a final frame */
        printf("vector_push_back \n");
        pico_vector_push_back(&bsock->out_vector, frame);
    }
    else {
        /* Pass the vector to zmtp layer */
        //TODO: should iterate trough all the zmtp sockets!
        
        /* Push the final frame to the out_vector */
        pico_vector_push_back(&bsock->out_vector, frame);
        
        if( zmtp_socket_send(bsock->sock, &bsock->out_vector) < 0 )
            return -1;

        if(bsock->type == ZMTP_TYPE_REQ)
            ((struct zmq_socket_req *)bsock)->send_enable = ZMQ_SEND_DISABLED;



        //TODO: clear out_vector & delete all related zmq_msg_t
    }

    return 0;
}

int zmq_recv(void* socket, char* txt)
{
    return 0;
}

void zmq_close(void* socket)
{
    
}

int zmq_msg_init_size(struct zmq_msg_t* msg, size_t size)
{
    
}

/* cyclic states
    if(s->state == ST_OPEN && ev & PICO_SOCK_EV_CONN)
    {
        s->state = ST_CONNECTED;
        zmtp_send_greeting(zmtp_s);
    }
    else if(s->state == ST_CONNECTED && ev & PICO_SOCK_EV_RD)
    {
        //read greeting
    }
    return;
*/
