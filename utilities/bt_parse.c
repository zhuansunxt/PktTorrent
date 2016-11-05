/*
 * bt_parse.c
 *
 * Initial Author: Ed Bardsley <ebardsle+441@andrew.cmu.edu>
 * Class: 15-441 (Spring 2005)
 *
 * Skeleton for 15-441 Project 2 command line parsing.
 *
 */

#include <assert.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "bt_parse.h"
#include "debug.h"
#include "commons.h"

static const char* const _bt_optstring = "p:c:f:m:i:d:h";

void bt_init(bt_config_t *config, int argc, char **argv) {
  bzero(config, sizeof(bt_config_t));

  strcpy(config->output_file, "output.dat");
  config->argc = argc;
  config->argv = argv;
}

void bt_usage() {
  fprintf(stderr,
	  "usage:  peer [-h] [-d <debug>] -p <peerfile> -c <chunkfile> -m <maxconn>\n"
	  "            -f <master-chunk-file> -i <identity>\n");
}

void bt_help() {
  bt_usage();
  fprintf(stderr,
	  "         -h                help (this message)\n"
	  "         -p <peerfile>     The list of all peers\n"
	  "         -c <chunkfile>    The list of chunks\n"
	  "         -m <maxconn>      Max # of downloads\n"
	  "	        -f <master-chunk> The master chunk file\n"
	  "         -i <identity>     Which peer # am I?\n"
	  );
}

bt_peer_t *bt_peer_info(const bt_config_t *config, int peer_id)
{
  assert(config != NULL);

  bt_peer_t *p;
  for (p = config->peers; p != NULL; p = p->next) {
    if (p->id == peer_id) {
      return p;
    }
  }
  return NULL;
}

void bt_parse_command_line(bt_config_t *config) {
  int c, old_optind;
  bt_peer_t *p;

  int argc = config->argc;
  char **argv = config->argv;

  DPRINTF(DEBUG_INIT, "bt_parse_command_line starting\n");
  old_optind = optind;
  while ((c = getopt(argc, argv, _bt_optstring)) != -1) {
    switch(c) {
    case 'h':
      bt_help();
      exit(0);
    case 'd':
      if (set_debug(optarg) == -1) {
	fprintf(stderr, "%s:  Invalid debug argument.  Use -d list  to see debugging options.\n",
		argv[0]);
	exit(-1);
      }
      break;
    case 'p':
      /* -p peer-list-file */
      strcpy(config->peer_list_file, optarg);
      break;
    case 'c':
      /* has-chunk-file */
      strcpy(config->has_chunk_file, optarg);
      break;
    case 'f':
      /* master-chunk-file */
      strcpy(config->chunk_file, optarg);
      break;
    case 'm':
      /* max-connection */
      config->max_conn = atoi(optarg);
      break;
    case 'i':
      /* identity */
      config->identity = atoi(optarg);
      break;
    default:
      bt_usage();
      exit(-1);
    }
  }

  bt_parse_peer_list(config);

  if (config->identity == 0) {
    fprintf(stderr, "bt_parse error:  Node identity must not be zero!\n");
    exit(-1);
  }
  if ((p = bt_peer_info(config, config->identity)) == NULL) {
    fprintf(stderr, "bt_parse error:  No peer information for myself (id %d)!\n",
	    config->identity);
    exit(-1);
  }
  config->myport = ntohs(p->addr.sin_port);

  /* Initialize hashtables */
  bt_parse_chunks_info(config);

  assert(config->identity != 0);
  assert(strlen(config->chunk_file) != 0);
  assert(strlen(config->has_chunk_file) != 0);

  optind = old_optind;
}

void bt_parse_peer_list(bt_config_t *config) {
  FILE *f;
  bt_peer_t *node;
  char line[BT_FILENAME_LEN], hostname[BT_FILENAME_LEN];
  int nodeid, port;
  struct hostent *host;

  assert(config != NULL);

  f = fopen(config->peer_list_file, "r");
  assert(f != NULL);

  while (fgets(line, BT_FILENAME_LEN, f) != NULL) {
    if (line[0] == '#') continue;
    assert(sscanf(line, "%d %s %d", &nodeid, hostname, &port) != 0);

    node = (bt_peer_t *) malloc(sizeof(bt_peer_t));
    assert(node != NULL);

    node->id = nodeid;

    host = gethostbyname(hostname);
    assert(host != NULL);
    node->addr.sin_addr.s_addr = *(in_addr_t *)host->h_addr;
    node->addr.sin_family = AF_INET;
    node->addr.sin_port = htons(port);

    node->next = config->peers;
    config->peers = node;
  }
  fclose(f);
}

void bt_parse_chunks_info(bt_config_t *config) {
  char *line = NULL;
  size_t len;
  FILE *m_chunk_f;
  FILE *has_chunk_f;
  char dummy[BT_FILENAME_LEN];
  char m_data_file[BT_FILENAME_LEN];

  config->chunks = (bt_chunks_t*)malloc(sizeof(bt_chunks_t));


  /* Parse master data file */
  config->chunks->master_chunk_map = hashmap_new();
  m_chunk_f = fopen(config->chunk_file, "r");
  assert(m_chunk_f != NULL);
  assert(getline(&line, &len, m_chunk_f) != -1);
  assert(sscanf(line, "%s %s", dummy, m_data_file) != 0);
  strcpy(config->chunks->master_data_file, m_data_file);
  assert(getline(&line, &len, m_chunk_f) != -1);    // omit one line.
  while(getline(&line, &len, m_chunk_f) != -1) {
    if (line[0] == "#") continue;
    char *hash_key = (char*)malloc(64);
    any_t chunk_id;
    assert(sscanf(line, "%ld %s", (intptr_t*) &chunk_id, hash_key) != 0);
    hashmap_put(config->chunks->master_chunk_map, hash_key, chunk_id);
  }
  fclose(m_chunk_f);

  /* Parse has_chunk_file */
  config->chunks->has_chunk_map = hashmap_new();
  has_chunk_f = fopen(config->has_chunk_file, "r");
  assert(has_chunk_f != NULL);

  line = NULL;
  while(getline(&line, &len, has_chunk_f) != -1) {
    if (line[0] == '#') continue;
    char *hash_key = (char*)malloc(64);
    any_t chunk_id;
    assert(sscanf(line, "%ld %s", (intptr_t*) &chunk_id, hash_key) != 0);
    hashmap_put(config->chunks->has_chunk_map, hash_key, chunk_id);
  }
  free(line);
  fclose(has_chunk_f);
}

void bt_dump_config(bt_config_t *config) {
  /* Print out the results of the parsing. */
  bt_peer_t *p;
  assert(config != NULL);

  console_log("**********PacTorrent Configuration Info**************");
  console_log("peer-identity: %d", config->identity);
  console_log("master-chunk-file: %s", config->chunk_file);
  console_log("has-chunk-file: %s", config->has_chunk_file);
  console_log("max-connection: %d", config->max_conn);
  console_log("peer-list-file: %s", config->peer_list_file);
  console_log("global-peer-info:");
  for (p =config->peers; p; p = p->next) {
    console_log("--- peer id: %d, <%s, %d>", p->id, inet_ntoa(p->addr.sin_addr), ntohs(p->addr.sin_port));
  }
  console_log("this peer's chunk-info:");
  bt_dump_chunkinfo(config);
  console_log("*****************************************************");
}


int hash_map_iter(const char* key, any_t val, any_t args) {
  console_log("--- <%s, %d>", key, (intptr_t) val);
  return MAP_OK;
}

void bt_dump_chunkinfo(bt_config_t *config) {
  assert(config != NULL);
  console_log("--- master-data-file: %s", config->chunks->master_data_file);
  console_log("--- has_chunk_file:");
  hashmap_iterate(config->chunks->has_chunk_map, hash_map_iter, NULL);
  console_log("--- master_chunk_file:");
  hashmap_iterate(config->chunks->master_chunk_map, hash_map_iter, NULL);
}
