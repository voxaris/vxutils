/**
 * Copyright 2008 Voxaris Inc, George Howitt.
 */

#ifndef _VX_SOCKET_H_
#define _VX_SOCKET_H_

#include <stdio.h>

#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#include <sys/socket.h>
#if defined (Linux)
#include <linux/sockios.h>
#include <sys/ioctl.h>
#endif
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum vx_status
{
   VX_OK = 0,
   VX_TIMEOUT,
   VX_FAIL,
} vx_status_t;

typedef struct vx_sockaddr
{
   sa_family_t domain;
   in_port_t port;
   char ipaddr[INET_ADDRSTRLEN + 1];
   struct sockaddr_in sad;
} vx_sockaddr_t;

typedef enum
{
   SOCK_NONE,
   SOCK_BIND,
   SOCK_LISTEN,
   SOCK_CONNECT,
   SOCK_SHUTDOWN,
   SOCK_CLOSE,
} sock_state_t;

struct vx_socket
{
   sock_state_t state;
   int fd;
   int domain;
   int type;
   int protocol;
   int error;
   vx_sockaddr_t local;
   vx_sockaddr_t remote;
   int connected;
   int32_t options; /** linger, keepalive, nonblock etc */
};
typedef struct vx_socket vx_socket_t;

vx_status_t vx_socket_create (vx_socket_t **sock, int domain, int type, int protocol);
vx_status_t vx_socket_shutdown (vx_socket_t *sock, int how);
vx_status_t vx_socket_close (vx_socket_t *sock);
vx_status_t vx_socket_bind (vx_socket_t *sock, const char * ip, in_port_t port);
vx_status_t vx_socket_listen (vx_socket_t *sock, int backlog);
vx_status_t vx_socket_accept (vx_socket_t *sock, vx_socket_t **newsock);
vx_status_t vx_socket_connect (vx_socket_t *sock, const char * ip, in_port_t port);
vx_status_t vx_socket_addr (vx_socket_t *sock, vx_sockaddr_t **addr, int which);

#ifdef __cplusplus
}
#endif

#endif

#define VX_ADDR_LOCAL  0
#define VX_ADDR_REMOTE 1

vx_status_t vx_socket_connect (vx_socket_t *sock, const char * ip, in_port_t port)
{
   struct sockaddr_in sad;
   memset (&sad, 0, sizeof (struct sockaddr_in));
   sad.sin_family = sock->domain;
   sad.sin_port = htons(port);
   if (inet_pton (sock->domain, ip, &sad.sin_addr) == -1)
   {
      sock->error = errno;
      return (VX_FAIL);
   }
   if (connect (sock->fd, &sad, sizeof (struct sockaddr_in)) == -1 )
   {
      sock->error = errno;
      return (VX_FAIL);
   }
   sock->state = SOCK_CONNECT;
   return (VX_OK);
}

vx_status_t vx_socket_accept (vx_socket_t *sock, vx_socket_t **newsock)
{
   socklen_t addrlen = sizeof (struct sockaddr_in);
   (*newsock) = calloc (1, sizeof (vx_socket_t));
   assert ((*newsock) != NULL);
   if (((*newsock)->fd = accept (sock->fd, &(*newsock)->remote.sad, &addrlen)) == -1)
   {
      sock->error = errno;
      free ((*newsock));
      return (VX_FAIL);
   }
   (*newsock)->domain = sock->domain;
   (*newsock)->type = sock->type;
   (*newsock)->protocol = sock->protocol;
   (*newsock)->state = SOCK_CONNECT;
   (*newsock)->options = sock->options;
   
   return (VX_OK);
}

vx_status_t vx_socket_listen (vx_socket_t *sock, int backlog)
{
   if (listen (sock->fd, SOMAXCONN) == -1 )\
   {
      sock->error = errno;
      return (VX_FAIL);
   }
   return (VX_OK);
}

/**
 * how = [SHUT_RD | SHUT_WR | SHUT_RDWR];
 */
vx_status_t vx_socket_shutdown (vx_socket_t *sock, int how)
{
   if (shutdown (sock->fd, how) == -1)
   {
      sock->error = errno;
      return (VX_FAIL);
   }
   sock->state = SOCK_SHUTDOWN;
   return (VX_OK);
}

vx_status_t vx_socket_close (vx_socket_t *sock)
{
   if (close (sock->fd) == -1 )
   {
      sock->error = errno;
      return (VX_FAIL);
   }
   sock->state = SOCK_CLOSE;
   return (VX_OK);
}

vx_status_t vx_socket_addr (vx_socket_t *sock, vx_sockaddr_t **addr, int which)
{
   struct sockaddr_in sad;
   socklen_t sad_len = sizeof (struct sockaddr_in);

   memset (&sad, 0, sizeof(sad));
   sad.sin_family = sock->domain;

   if (which == VX_ADDR_LOCAL)
   {
      if (getsockname (sock->fd, &sad, &sad_len) == -1)
      {
         sock->error = errno;
         return (VX_FAIL);
      }
      sad.sin_family = sock->local.domain = sock->domain;
      if (inet_ntop (sock->domain, &(sad.sin_addr), sock->local.ipaddr, INET_ADDRSTRLEN) == NULL)
      {
         sock->error = errno;
         return (VX_FAIL);
      }
      sock->local.sad = sad;
      sock->local.port = ntohs(sad.sin_port);
      (*addr) = &(sock->local);
   }
   else if (which == VX_ADDR_REMOTE)
   {
      if (getpeername (sock->fd, &sad, &sad_len) == -1)
      {
         sock->error = errno;
         return (VX_FAIL);
      }
      sad.sin_family = sock->remote.domain = sock->domain;
      if (inet_ntop (sock->domain, &(sad.sin_addr), sock->remote.ipaddr, INET_ADDRSTRLEN) == NULL)
      {
         sock->error = errno;
         return (VX_FAIL);
      }
      sock->remote.sad = sad;
      sock->remote.port = ntohs(sad.sin_port);
      (*addr) = &(sock->remote);
   }
   return (VX_OK);
}

vx_status_t vx_socket_create (vx_socket_t **sock, int domain, int type, int protocol)
{
   if ((domain != AF_INET) && (type != SOCK_STREAM))
      return VX_FAIL;

   *sock = calloc (1, sizeof (vx_socket_t));
   assert (*sock != NULL);

   (*sock)->domain = domain;
   (*sock)->type = type;
   (*sock)->protocol = protocol;
   if (((*sock)->fd = socket (domain, type, protocol)) == -1 )
   {
      (*sock)->error = errno;
      return (VX_FAIL);
   }
   return (VX_OK);
}

vx_status_t vx_socket_bind (vx_socket_t *sock, const char *ip, in_port_t port)
{
   struct sockaddr_in sad;
   struct linger ling;
   int option, optlen;

   memset ((char *) &sad, 0, sizeof (sad));
   sad.sin_family = sock->domain;
   sad.sin_port = htons ((uint16_t) port);

   if (inet_pton(sad.sin_family, ip, &sad.sin_addr) == 0)
   {
      sock->error = errno;
      return (VX_FAIL);
   }

   option = 1;
   optlen = sizeof (int);
   setsockopt (sock->fd, SOL_SOCKET, SO_REUSEADDR, (const void *) &option, (size_t) optlen);
   ling.l_onoff = 0;
   ling.l_linger = 0;
   optlen = sizeof (struct linger);
   setsockopt (sock->fd, SOL_SOCKET, SO_LINGER, (void *) &ling, optlen);

   if (bind (sock->fd, (struct sockaddr *) &sad, sizeof (sad)) == -1)
   {
      sock->error = errno;
      return (VX_FAIL);
   }
   return (VX_OK);
}

int main (int argc, char *argv[])
{
   vx_socket_t *sock;
   vx_sockaddr_t *addr;

   char *ip = "127.0.0.1";
   in_port_t port = 2345;

   if (vx_socket_create (&sock, AF_INET, SOCK_STREAM, 0) != VX_OK)
   {
      printf ("vx_socket_create error: %d %s\n", sock->error, strerror(sock->error));
      return (-1);
   }
   if (vx_socket_addr(sock, &addr, VX_ADDR_LOCAL) != VX_OK)
   {
      printf ("vx_sock_addr error: %d %s\n", sock->error, strerror(sock->error));
      return (-1);
   }
   printf ("socket: fd = %d, addr = %s, port = %d, domain = %d, type = %d\n",
      sock->fd, addr->ipaddr, addr->port, addr->domain, sock->type);

   if (vx_socket_bind (sock, ip, port) != VX_OK)
   {
      printf ("vx_socket_bind error: %d %s\n", sock->error, strerror(sock->error));
      return (-1);
   }

   if (vx_socket_addr(sock, &addr, VX_ADDR_LOCAL) != VX_OK)
   {
      printf ("vx_sock_addr error: %d %s\n", sock->error, strerror(sock->error));
      return (-1);
   }
   printf ("socket: fd = %d, addr = %s, port = %d, domain = %d, type = %d\n",
      sock->fd, addr->ipaddr, addr->port, addr->domain, sock->type);

   if (vx_socket_addr(sock, &addr, VX_ADDR_REMOTE) != VX_OK)
   {
      printf ("vx_sock_addr error: %d %s\n", sock->error, strerror(sock->error));
      return (-1);
   }
   printf ("socket: fd = %d, addr = %s, port = %d, domain = %d, type = %d\n",
      sock->fd, addr->ipaddr, addr->port, addr->domain, sock->type);

   return (0);
}
