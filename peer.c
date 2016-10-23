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
#include "core/session.h"

void peer_run(g_state_t *g_state);

int main(int argc, char **argv) {
  g_state_t g_state;
  bt_config_t config;
  session_t session;

  bt_init(&config, argc, argv);
  session_init(&session);

  bt_parse_command_line(&config);

#ifdef DEBUG
    //bt_dump_config(&config);
    //bt_dump_chunkinfo(&config);
#endif

  g_state.g_config = &config;
  g_state.g_session = &session;

  peer_run(&g_state);
  return 0;
}


void process_inbound_udp(int sock) {
  #define BUFLEN 1500
  struct sockaddr_in from;
  socklen_t fromlen;
  char buf[BUFLEN];

  fromlen = sizeof(from);
  spiffy_recvfrom(sock, buf, BUFLEN, 0, (struct sockaddr *) &from, &fromlen);

  printf("PROCESS_INBOUND_UDP SKELETON -- replace!\n"
	 "Incoming message from %s:%d\n%s\n\n",
	 inet_ntoa(from.sin_addr),
	 ntohs(from.sin_port),
	 buf);
}

void handle_user_input(char *line, void *cbdata, g_state_t *g) {
  char method[128], chunkf[128], outf[128];

  bzero(chunkf, sizeof(chunkf));
  bzero(outf, sizeof(outf));

  if (sscanf(line, "%120s %120s %120s", method, chunkf, outf)) {

    if (strcmp(method, "GET")) {
      fprintf(stderr, "Invalid method. Method should be GET\n");
      return;
    }

    if (strlen(outf) > 0) {
      process_get(chunkf, outf, g);
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
    exit(-1);
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

  while (1) {
    int nfds;
    FD_SET(STDIN_FILENO, &readfds);
    FD_SET(sock, &readfds);

    nfds = select(sock+1, &readfds, NULL, NULL, NULL);

    if (nfds > 0) {
//      if (FD_ISSET(sock, &readfds)) {
//	      process_inbound_udp(sock);
//      }

      if (FD_ISSET(STDIN_FILENO, &readfds)) {
        process_user_input(STDIN_FILENO, userbuf, handle_user_input, "Currently unused", g_state);
        if (g_state->g_session->state == AWAITING_WHOHAS) {
#ifdef DEBUG
          console_log("Below chunks are missing locally:");
          session_nlchunk_t *p;
          for (p = g_state->g_session->non_local_chunks; p; p = p->next)
            console_log("Non-local-chunk: %s", p->chunk_hash);
#endif
        } else {
#ifdef DEBUG
          console_log("All chunks are accessible locally");
#endif
        }

      }
    }
  }
}
