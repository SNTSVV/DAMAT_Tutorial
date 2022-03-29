/* WARNING! All changes made to this file will be lost! */

#ifndef W_CSP_AUTOCONFIG_H_WAF
#define W_CSP_AUTOCONFIG_H_WAF

#define GIT_REV "v1.6-748-g038c208"
/* #undef CSP_FREERTOS */
#define CSP_POSIX 1
#define HAVE_STDIO_H 1
#define CSP_HAVE_STDIO 1
#define HAVE_SYS_SOCKET_H 1
#define HAVE_ARPA_INET_H 1
#define CSP_HAVE_LIBSOCKETCAN 1
#define CSP_HAVE_LIBZMQ 1
#define CSP_QFIFO_LEN 15
#define CSP_PORT_MAX_BIND 16
#define CSP_CONN_RXQUEUE_LEN 15
#define CSP_CONN_MAX 8
#define CSP_BUFFER_SIZE 256
#define CSP_BUFFER_COUNT 15
#define CSP_RDP_MAX_WINDOW 5
#define CSP_RTABLE_SIZE 10
#define CSP_ENABLE_CSP_PRINT 1
#define CSP_PRINT_STDIO 1
#define CSP_USE_RDP 1
#define CSP_USE_HMAC 1
#define CSP_USE_PROMISC 1
#define CSP_USE_DEDUP 1

#endif /* W_CSP_AUTOCONFIG_H_WAF */
