/*
 * peer.c
 *
 * Authors: Ed Bardsley <ebardsle+441@andrew.cmu.edu>,
 *          Dave Andersen
 * Class: 15-441 (Spring 2005)
 *
 * Skeleton for 15-441 Project 2.
 *
 */

#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utilities/commons.h"
#include "utilities/spiffy.h"
#include "utilities/bt_parse.h"
#include "utilities/input_buffer.h"
#include "core/user_handler.h"
#include "core/global.h"
#include "core/location_handler.h"
#include "core/handler.h"

void peer_run(g_state_t *g_state);

int main(int argc, char **argv) {
  g_state_t g_state;
  bt_config_t config;

  g_state_init(&g_state);

  /* Configuration info initialization. */
  bt_init(&config, argc, argv);
  bt_parse_command_line(&config);
  g_state.g_config = &config;

  peer_run(&g_state);
  return 0;
}

/* Entrance for all peer request */
void process_inbound_udp(g_state_t *g_state) {
  struct sockaddr_in from;
  socklen_t fromlen;
  char buf[PACKET_LEN];

  fromlen = sizeof(from);
  spiffy_recvfrom(g_state->peer_socket, buf, PACKET_LEN, 0, (struct sockaddr *) &from, &fromlen);

  short id;
  bt_peer_t *p;
  for (p = g_state->g_config->peers; p; p = p->next) {
    if ((inet_ntoa(from.sin_addr) == inet_ntoa(p->addr.sin_addr) &&
         ntohs(from.sin_port) == ntohs(p->addr.sin_port))) {
      id = p->id;
      break;
    }
  }

  if (p == NULL) {
    fprintf(stderr, "Peer %d: Receive data from unknown peer. Drop it.", g_state->g_config->identity);
    return;
  }

  process_packet(g_state, buf, id);
}

/* Entrance for all user request */
void handle_user_input(char *line, void *cbdata, g_state_t *g) {
  char method[128], chunkf[128], outf[128];

  bzero(chunkf, sizeof(chunkf));
  bzero(outf, sizeof(outf));

  if (sscanf(line, "%120s %120s %120s", method, chunkf, outf)) {
    if (strcmp(method, "GET")) {
      fprintf(stderr, "Invalid method. Method should be GET\n");
      return;
    }

    if (strlen(outf) > 0 && strlen(chunkf) > 0) {
      process_get(chunkf, outf, g);
    } else {
      fprintf(stderr, "Chunk or output file must be specified");
    }
  }
}

void peer_run(g_state_t * g_state) {
  int sock;
  struct sockaddr_in myaddr;
  fd_set readfds;
  struct user_iobuf *userbuf;

  if ((userbuf = create_userbuf()) == NULL) {
    perror("peer_run could not allocate userbuf");
    exit(EXIT_FAILURE);
  }

  if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP)) == -1) {
    perror("peer_run could not create socket");
    exit(EXIT_FAILURE);
  }

  bzero(&myaddr, sizeof(myaddr));
  myaddr.sin_family = AF_INET;
  myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  myaddr.sin_port = htons(g_state->g_config->myport);

  if (bind(sock, (struct sockaddr *) &myaddr, sizeof(myaddr)) == -1) {
    perror("peer_run could not bind socket");
    exit(-1);
  }

  spiffy_init(g_state->g_config->identity, (struct sockaddr *)&myaddr, sizeof(myaddr));
  g_state->peer_socket = sock;

  while (1) {
    int nfds;
    FD_SET(STDIN_FILENO, &readfds);
    FD_SET(sock, &readfds);

    nfds = select(sock+1, &readfds, NULL, NULL, NULL);

    if (nfds > 0) {

      /* Packet from other peers */
      if (FD_ISSET(sock, &readfds)) {
	      process_inbound_udp(g_state);
      }

      /* Request from the user */
      if (FD_ISSET(STDIN_FILENO, &readfds)) {
        /* Init a session for the user */
        session_t session;
        session_init(&session);
        g_state->g_session = &session;

        process_user_input(STDIN_FILENO, userbuf, handle_user_input, "Currently unused", g_state);
        if (g_state->g_session->state == AWAITING_WHOHAS) {
          ask_peers_who_has(g_state);
        } else {
          console_log("All chunks are accessible locally");
        }
      }

    } // End if (nfds > 0).

    do_upload(g_state);
    //do_download(g_state);
  } // End while loop.
} // End peer_run function
