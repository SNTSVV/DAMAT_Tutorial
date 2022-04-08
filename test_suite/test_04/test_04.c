#include <csp/csp_debug.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <csp/csp.h>
#include <csp/drivers/can_socketcan.h>
#include <csp/drivers/usart.h>
#include <csp/interfaces/csp_if_zmqhub.h>

/* These three functions must be provided in arch specific way */
int router_start(void);
int server_start(void);
int client_start(void);

/* Server port, the port the server listens on for incoming connections from the
 * client. */
#define MY_SERVER_PORT 10

/* Commandline options */
static uint8_t server_address = 255;

/* test mode, used for verifying that host & client can exchange packets over
 * the loopback interface */
static unsigned int server_received = 0;
static unsigned int client_sent = 0;

int sport_server_archive[10];
int dport_server_archive[10];

int sport_client_archive[10];
int dport_client_archive[10];

int print_array(int array[10]) {
  csp_print("[") for (unsigned i = 0; i < 10; i++) {
    csp_print(" %d ", array[i]);
  }
  csp_print("]\n")
}

/* Server task - handles requests from clients */
void server(void) {

  csp_print("Server task started\n");

  /* Create socket with no specific socket options, e.g. accepts CRC32, HMAC,
   * etc. if enabled during compilation */
  csp_socket_t sock = {0};

  /* Bind socket to all ports, e.g. all incoming connections will be handled
   * here */
  csp_bind(&sock, CSP_ANY);

  /* Create a backlog of 10 connections, i.e. up to 10 new connections can be
   * queued */
  csp_listen(&sock, 10);

  /* Wait for connections and then process packets on the connection */
  while (1) {

    /* Wait for a new connection, 10000 mS timeout */
    csp_conn_t *conn;
    if ((conn = csp_accept(&sock, 10000)) == NULL) {
      /* timeout */
      continue;
    }

    /* Read packets on connection, timout is 100 mS */
    csp_packet_t *packet;

    while ((packet = csp_read(conn, 50)) != NULL) {

      switch (csp_conn_dport(conn)) {
      case MY_SERVER_PORT:
        /* Process packet here */
        if (server_received <= 10) {
          csp_print("sport server = %d\n", csp_conn_sport(conn));
          csp_print("dport server = %d\n", csp_conn_dport(conn));
          sport_server_archive[server_received] = csp_conn_sport(conn);
          dport_server_archive[server_received] = csp_conn_dport(conn);
        }

        csp_print("Packet received on MY_SERVER_PORT: %s\n",
                  (char *)packet->data);
        csp_buffer_free(packet);
        ++server_received;

        break;

      default:
        /* Call the default CSP service handler, handle pings, buffer use, etc.
         */
        csp_print("csp_service_handler\n");
        csp_service_handler(packet);
        break;
      }
    }

    /* Close current connection */
    csp_close(conn);
  }

  return;
}
/* End of server task */

/* Client task sending requests to server task */
void client(void) {

  csp_print("Client task started\n");

  unsigned int count = 'A';

  while (1) {

    usleep(200000);

    /* Send data packet (string) to server */

    /* 1. Connect to host on 'server_address', port MY_SERVER_PORT with regular
     * UDP-like protocol and 1000 ms timeout */
    csp_conn_t *conn = csp_connect(CSP_PRIO_NORM, server_address,
                                   MY_SERVER_PORT, 1000, CSP_O_NONE);
    if (conn == NULL) {
      /* Connect failed */
      csp_print("Connection failed\n");
      return;
    }

    /* 2. Get packet buffer for message/data */
    csp_packet_t *packet = csp_buffer_get(100);
    if (packet == NULL) {
      /* Could not get buffer element */
      csp_print("Failed to get CSP buffer\n");
      return;
    }

    /* 3. Copy data to packet */
    memcpy(packet->data, "Hello world ", 12);
    memcpy(packet->data + 12, &count, 1);
    memset(packet->data + 13, 0, 1);
    count++;

    /* 4. Set packet length */
    packet->length =
        (strlen((char *)packet->data) + 1); /* include the 0 termination *

    /* 5. Send packet */
    csp_send(conn, packet);

    if (client_sent <= 10) {
      csp_print("sport client = %d\n", csp_conn_sport(conn));
      csp_print("dport client = %d\n", csp_conn_dport(conn));
      sport_client_archive[client_sent] = csp_conn_sport(conn);
      dport_client_archive[client_sent] = csp_conn_dport(conn);
    }
    ++client_sent;
    /* 6. Close connection */
    csp_close(conn);
  }

  return;
}
/* End of client task */

/* main - initialization of CSP and start of server/client tasks */
int main(void) {

  uint8_t address = 0;
#if (CSP_HAVE_LIBSOCKETCAN)
  const char *can_device = NULL;
#endif
  const char *kiss_device = NULL;
#if (CSP_HAVE_LIBZMQ)
  const char *zmq_device = NULL;
#endif
  const char *rtable = NULL;

  csp_print("Initialising CSP\n");

  /* Init CSP */
  csp_init();

  /* Start router */
  router_start();

  /* Add interface(s) */
  csp_iface_t *default_iface = NULL;
  if (kiss_device) {
    csp_usart_conf_t conf = {.device = kiss_device,
                             .baudrate =
                                 115200, /* supported on all platforms */
                             .databits = 8,
                             .stopbits = 1,
                             .paritysetting = 0,
                             .checkparity = 0};
    int error = csp_usart_open_and_add_kiss_interface(
        &conf, CSP_IF_KISS_DEFAULT_NAME, &default_iface);
    if (error != CSP_ERR_NONE) {
      csp_print("failed to add KISS interface [%s], error: %d\n", kiss_device,
                error);
      exit(1);
    }
  }
#if (CSP_HAVE_LIBSOCKETCAN)
  if (can_device) {
    int error = csp_can_socketcan_open_and_add_interface(
        can_device, CSP_IF_CAN_DEFAULT_NAME, 0, false, &default_iface);
    if (error != CSP_ERR_NONE) {
      csp_print("failed to add CAN interface [%s], error: %d\n", can_device,
                error);
      exit(1);
    }
  }
#endif
#if (CSP_HAVE_LIBZMQ)
  if (zmq_device) {
    int error = csp_zmqhub_init(0, zmq_device, 0, &default_iface);
    if (error != CSP_ERR_NONE) {
      csp_print("failed to add ZMQ interface [%s], error: %d\n", zmq_device,
                error);
      exit(1);
    }
  }
#endif

  if (rtable) {
    int error = csp_rtable_load(rtable);
    if (error < 1) {
      csp_print("csp_rtable_load(%s) failed, error: %d\n", rtable, error);
      exit(1);
    }
  } else if (default_iface) {
    csp_rtable_set(0, 0, default_iface, CSP_NO_VIA_ADDRESS);
  } else {
    /* no interfaces configured - run server and client in process, using
     * loopback interface */
    server_address = address;
  }

  csp_print("Connection table\r\n");
  csp_conn_print_table();

  csp_print("Interfaces\r\n");
  csp_rtable_print();

  csp_print("Route table\r\n");
  csp_iflist_print();

  /* Start server thread */
  if ((server_address ==
       255) || /* no server address specified, I must be server */
      (default_iface == NULL)) { /* no interfaces specified -> run server &
                                    client via loopback */
    server_start();
  }

  /* Start client thread */
  if ((server_address !=
       255) || /* server address specified, I must be client */
      (default_iface == NULL)) { /* no interfaces specified -> run server &
                                    client via loopback */
    client_start();
  }

  /* Wait for execution to end (ctrl+c) */
  while (1) {
    sleep(3);
    csp_print("sport server\n");
    int success = print_array(sport_server_archive);
    csp_print("dport server\n");
    success = print_array(dport_server_archive);
    csp_print("sport client\n");
    success = print_array(sport_client_archive);
    csp_print("dport client\n");
    success = print_array(dport_client_archive);

    /* This test is intended for checking  */

    for (unsigned i = 0; i < 10; ++i) {
      if (sport_server_archive[i] != dport_client_archive[i] ||
          dport_server_archive[i] != sport_client_archive[i]) {
        csp_print(" client dport and server sport do not correspond\n");
        exit(1);
      }

      if (dport_server_archive[i] != sport_client_archive[i] ||
          dport_server_archive[i] != sport_client_archive[i]) {
        csp_print("server dport and client sport do not correspond\n");
        exit(1);
      }
    }

    csp_print("Server received %u packets\n", server_received);
    exit(0);
  }

  return 0;
}
