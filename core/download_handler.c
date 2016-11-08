/**
 * @file download_handler.c
 * @brief Implementation for download component for the system.
 * @author Xiaotong Sun <xiaotons@andrew.cmu.edu>
 * @author Longqi Cai   <longqic@andrew.cmu.edu>
 */

#include <sys/file.h>
#include <assert.h>
#include <time.h>
#include <unistd.h>
#include "download_handler.h"
#include "../utilities/io.h"
#include "../utilities/bt_parse.h"
#include <unistd.h>

/**
 * GET packet contains only the chunk hash for the chunk
 * the client wants to fetch.
 * @param chunk_hash  string of hash_value of chunk.
 * @return built GET packet.
 */
packet_t *build_get_packet(const char *chunk_hash) {
  packet_t *get_packet = pkt_new();
  get_packet->hdr->magic = htons(15441);
  get_packet->hdr->version = 1;
  get_packet->hdr->type = 2;
  get_packet->hdr->hlen = htons(HDRSZ);
  get_packet->hdr->plen = htons(HDRSZ+SHA1_HASH_SIZE);
  get_packet->hdr->seqn = htonl(0);
  get_packet->hdr->ackn = htonl(0);

  /* Copy chunk hash to packet. */
  char hash_hex[SHA1_HASH_SIZE];
  ascii2hex(chunk_hash, SHA1_HASH_SIZE*2, (uint8_t*)hash_hex);
  memcpy(get_packet->payload, hash_hex, SHA1_HASH_SIZE);
  return get_packet;
}

/**
 * Construct DATA packet.
 * @param seq seqeunce number.
 * @param data_size size of attached data.
 * @param data pointer to attached data.
 * @return built DATA packet.
 */
packet_t *build_data_packet(unsigned int seq, size_t data_size, char *data) {
  packet_t *data_packet = pkt_new();
  uint16_t packet_len = HDRSZ + (uint16_t)data_size;

  data_packet->hdr->magic = htons(15441);
  data_packet->hdr->version = 1;
  data_packet->hdr->type = 3;                       // packet type: DATA
  data_packet->hdr->hlen = htons(HDRSZ);
  data_packet->hdr->plen = htons(packet_len);
  data_packet->hdr->seqn = htonl((uint32_t)seq);
  data_packet->hdr->ackn = htonl(0);

  memcpy(data_packet->payload, data, data_size);
  return data_packet;
}

/**
 * Build DATA packets and buffer them.
 * @param chunk_hash The given hashvalue for the chunk.
 * @param g Global state.
 * @param des_peer Destination peer of generated DATA packet.
 */
void build_chunk_data_packets(const char *chunk_hash, g_state_t *g, short des_peer) {
  /* Get chunk's offset in master-data-file */
  any_t offset;
  hashmap_get(g->g_config->chunks->master_chunk_map, chunk_hash, &offset);
  console_log("Building DATA packet for chunk %s with offset %d",
              chunk_hash, (intptr_t) offset);

  int i;
  FILE *f = fopen(g->g_config->chunks->master_data_file, "r");
  for (i = 1; i <= MAX_SEQ_NUM; i++) {
    size_t data_size;
    char data[DATA_PACKET_SIZE+1];

    size_t data_offset = ((intptr_t) offset) * CHUNK_SIZE + (i-1) * DATA_PACKET_SIZE;
    fseek(f, data_offset, SEEK_SET);
    if (i < MAX_SEQ_NUM) {
      data_size = DATA_PACKET_SIZE;
    } else {
      data_size = CHUNK_SIZE % DATA_PACKET_SIZE;
    }
    fread(data, sizeof(uint8_t), data_size, f);
    packet_t *data_packet = build_data_packet(i, data_size, data);
    g->upload_conn_pool[des_peer]->buffer[i] = data_packet;
  }

  fclose(f);
}

/**
 * Build ACK packet
 * @param ack ACK number.
 * @return  Built ACK packet
 */
packet_t *build_ack_packet(unsigned int ack) {
  packet_t *ack_packet = pkt_new();

  ack_packet->hdr->magic = htons(15441);
  ack_packet->hdr->version = 1;
  ack_packet->hdr->type = 4;                    //packet type: ACK
  ack_packet->hdr->hlen = htons(HDRSZ);
  ack_packet->hdr->plen = htons(HDRSZ);         //packet_len is header_len
  ack_packet->hdr->seqn = htonl(0);
  ack_packet->hdr->ackn = htonl((uint32_t)ack);

  return ack_packet;
}

/**
 * Send GET packet to target peer.
 * If GET packet can not be sent due to concurrency constriction or ongoing connection
 * conflict, buffer the GET packet to a queue for later re-sending.
 * @param id target peer id.
 * @param get_packet GET packet.
 * @param g global state.
 * @param chunk_hash SHA1 hash value of chunk to download.
 */
void try_send_get_packet(short id, packet_t *get_packet, g_state_t *g) {
  char chunk_hash[HASH_STR_LEN];
  hex2ascii(get_packet->payload, SHA1_HASH_SIZE, chunk_hash);
//  console_log("Peer %d: Trying to send GET packet to peer %d on chunk %s...",
//              g->g_config->identity, id, chunk_hash);

  if (g->download_conn_pool[id] == NULL &&
          g->curr_download_conn_cnt < g->g_config->max_conn) {
    send_packet(id, get_packet, g);
    pkt_free(get_packet); // TODO(xiaotons): should store this in case of peer crash.
    console_log("Peer %d: Sent GET packet to peer %d", g->g_config->identity, id);

    /* Init downloading connection with corresponding peer */
    init_recv_window(g, id, chunk_hash);
    g->curr_download_conn_cnt++;
    gettimeofday(&(g->download_conn_pool[id]->get_timestamp), NULL);    // set timestamp of GET.
    console_log("Peer %d: Initiate download session with peer %d for chunk %s",
                g->g_config->identity, id, chunk_hash);
  } else {
    if (g->download_conn_pool[id] != NULL) {
//      console_log("Peer %d: Existing downloading connection with peer %d. Queue GET packet",
//                  g->g_config->identity, id);
    }
    if (g->curr_download_conn_cnt == g->g_config->max_conn) {
//      console_log("Peer %d: Maximum downloading connections reached! Queue GET packet",
//                  g->g_config->identity);
    }
    pending_packet_t *pending_GET_packet = build_pending_packet(get_packet, id);
    enqueue(g->pending_get_packets, pending_GET_packet);
  }
}


/**
 * Send GET packet to designated peer.
 * The invoker of this function is the receiver in downloading process.
 * @param id designated peer's id.
 * @param get_packet GET packet
 * @param g global state to retrieve socket and peers info
 */
void send_packet(short id, packet_t *packet, g_state_t *g){
  bt_peer_t *peer = bt_peer_info(g->g_config, id);
  spiffy_sendto(g->peer_socket, packet->raw, ntohs(packet->hdr->plen),
                0, (struct sockaddr *)&(peer->addr), sizeof(peer->addr));
}

/**
 * Process incoming GET packet from peer.
 * The invoker of this function is the sender who uploads DATA packets.
 * @param g Global state.
 * @param get_packet GET packet.
 * @param from peer id.
 */
void process_get_packet(g_state_t *g, packet_t *get_packet, short from) {
  /* Retrieve hashed chunk */
  const uint8_t *chunk_ptr = get_packet->payload;
  char chunk_hash[2*SHA1_HASH_SIZE+1];
  hex2ascii(chunk_ptr, SHA1_HASH_SIZE, chunk_hash);
  chunk_hash[2*SHA1_HASH_SIZE] = '\0';

  any_t dummy;
  if (hashmap_get(g->g_config->chunks->has_chunk_map, chunk_hash, &dummy)
          == MAP_MISSING) {
    console_log("Peer %d: received GET packet for nl-chunk %s, drop it!",
                g->g_config->identity, chunk_hash);
    return;
  }

  console_log("Peer %d: received GET packet for chunk %s", g->g_config->identity, chunk_hash);

  if (g->upload_conn_pool[from] == NULL && g->curr_upload_conn_cnt < g->g_config->max_conn) {
    /* Establish new uploading connection with the requesting peer */
    console_log("Peer %d: ****** Establish new upload connection with peer %d ******",
                g->g_config->identity, from);
    init_send_window(g, from);
    g->curr_upload_conn_cnt++;
    build_chunk_data_packets(chunk_hash, g, from);
    console_log("Peer %d: DATA packets are built", g->g_config->identity);
  } else {
    if (g->curr_upload_conn_cnt == g->g_config->max_conn) {
      console_log("Peer %d: Maximum number of uploading connections reached. Reject GET.",
                  g->g_config->identity);
    }
    if (g->upload_conn_pool[from] != NULL) {
      /* There's existing uploading connection with the peer. Reject GET */
      console_log("Peer %d: Existing upload connection with peer %d, reject GET.",
                  g->g_config->identity, from);
    }
  }
}

/**
 * Process incoming DATA packet from peer.
 * The invoker of this function is the receiver who downloads DATA packets.
 * @param g Global state.
 * @param data_packet Received DATA packet.
 * @param from Uploading peer's id.
 */
void process_data_packet(g_state_t *g, packet_t *data_packet, short from) {
  if (g->download_conn_pool[from] == NULL) {
    console_log("Peer %d: Error! There's no download connection with peer %d",
                g->g_config->identity, from);
    return;
  }

  recv_window_t *window = g->download_conn_pool[from];

  if (window->state == IDLE) {
    /* First DATA packet received */
    console_log("Peer %d: Starting to download chunk %s from peer %d",
                g->g_config->identity, window->chunk_hash, from);
    window->state = INPROGRESS;
  }

  /* Make a local copy of DATA packet. Since argument <data_packet>  will
   * be freed outside this function */
  packet_t* data_packet_copy = pkt_new();
  memcpy(data_packet_copy, data_packet, ntohs(data_packet->hdr->plen));

  uint32_t seq_number = ntohl(data_packet_copy->hdr->seqn);
  window->buffer[seq_number] = data_packet_copy;

  if (seq_number == window->next_packet_expected) {

    /* Upon receiving next_packet_expected packet. Scan buffer to
     * update next_packet_expected.
     * e.g. If current next_packet_expected is 4, but packet 5 and 6 is
     * already in the buffer, then next_packet_expected should be set to 7
     * and should send ACK packet with ack number 6 (next_packet_expected-1).
     */

    while (window->next_packet_expected <= MAX_SEQ_NUM
          && window->buffer[window->next_packet_expected] != NULL) {
      window->next_packet_expected++;
    }

    if (window->next_packet_expected == MAX_SEQ_NUM+1) {
      /* Received all DATA packets for this chunk */
      packet_t *ack_packet = build_ack_packet(window->next_packet_expected-1);
      send_packet(from, ack_packet, g);
      console_log("Peer %d: Received all DATA packets from peer %d",
                    g->g_config->identity, from);
      window->state = DONE;
      return;
    }
    packet_t *ack_packet = build_ack_packet(window->next_packet_expected-1);
    send_packet(from, ack_packet, g);
//    console_log("Peer %d: Sent ACK %u back to peer %d",
//                g->g_config->identity, window->next_packet_expected-1, from);
  } else if (seq_number > window->next_packet_expected){

    /* Upon receiving packet whose SEQ is larger than next_packet_expected,
     * it means currently there's a GAP in the buffer.
     * Should send duplicate ACK to uploader to indicate this GAP.
     */

    packet_t *dup_ack_packet = build_ack_packet(window->next_packet_expected-1);
    send_packet(from, dup_ack_packet, g);
    console_log("Peer %d: Sent DUP ACK %u back to peer %d",
                g->g_config->identity, window->next_packet_expected-1, from);
  } else {
    /* next_packet_expected > SEQ. Discard it */
    console_log("Peer %d: Received SEQ is %u, smaller than expected (%u). Discard it!",
                g->g_config->identity, seq_number, window->next_packet_expected);
  }

//  console_log("Peer %d: current next_packet_expected is %u",
//              g->g_config->identity, window->next_packet_expected);
}

int dup_map_iter(const char* key, any_t val, map_t map) {
  console_log("--- <%s, %d>", key, (intptr_t)val);
  return MAP_OK;
}

/**
 * Process incoming ACK packet from peer.
 * The invoker of this function is the sender who uploads DATA packets.
 * @param g Global state.
 * @param ack_packet Received ACK packet.
 * @param from Downloading peer's id.
 */
void process_ack_packet(g_state_t *g, packet_t *ack_packet, short from) {
  assert(g->upload_conn_pool[from] != NULL);

  uint32_t ack_number = htonl(ack_packet->hdr->ackn);
  send_window_t *window = g->upload_conn_pool[from];

//  console_log("Peer %d: Received ACK %u from peer %d",
//              g->g_config->identity, ack_number, from);

  if (window->dup_ack_map[ack_number] == 0) {
    /* First appearance accumulative ACK.
     * It means all packet whose SEQ is less or equal to ACK is received by receiver.
     * Update last_packet_acked and last_packet_available to move forward.
     */

    if (ack_number == MAX_SEQ_NUM) {
      /* Downloader received all DATA packet! */
      console_log("Peer %d: peer %d received all DATA packet!",
                  g->g_config->identity, from);
      console_log("Peer %d: Closing uploading connection with peer %d",
                  g->g_config->identity, from);
      free_send_window(g, from);
      g->curr_upload_conn_cnt--;
      return;
    }

    window->dup_ack_map[ack_number] = 1;
    window->last_packet_acked = ack_number;
    window->last_packet_available =
            MIN(window->last_packet_acked + window->max_window_size, MAX_SEQ_NUM);

    /* Congestion Control */
    congctrl_t* cc = &window->cc;
    if (cc->state == SLOW_START) {
      cc->cwnd += 1;
      window->max_window_size = (size_t) cc->cwnd;
      if (cc->cwnd >= cc->ssthresh) {
        cc->state = CONG_AVOID;
      }
    } else {
      cc->cwnd += 1 / cc->cwnd;
      window->max_window_size = (size_t) cc->cwnd;
    }
    cc_log(cc);

  } else if (window->dup_ack_map[ack_number] == 1) {
    window->dup_ack_map[ack_number] = 2;
  } else if (window->dup_ack_map[ack_number] == 2) {
    /* 3-time duplicate ACK, which indicates lost packet whose SEQ == ACK.
     * Resend lost packet */
    console_log("Peer %d: Received DUP ACK %u from peer %d",
                g->g_config->identity, ack_number, from);
    send_packet(from, window->buffer[ack_number + 1], g);

    /* Congestion Control */
    congctrl_t* cc = &window->cc;
    cc->state = SLOW_START;
    cc->ssthresh = MAX(2, cc->cwnd/2);
    cc->cwnd = 1;
    window->max_window_size = 1;
    cc_log(cc);

  } else {
    /* Control should never reach here */
    assert(0);
  }
}

/**
 * Handle uploading.
 * This function will be invoked periodically in the peer's main event loop
 * @param g Global state.
 */
void do_upload(g_state_t *g) {
  int i;
  for (i = 0; i < MAX_PEER_NUM; i++) {
    if (g->upload_conn_pool[i] != NULL) {
      send_window_t * window= g->upload_conn_pool[i];

      /* Check Timeout */
      uint32_t sent_iter = window->last_packet_acked+1;
      struct timeval curr_time;
      // TODO: what if window size gets smaller?
      for (; sent_iter <= window->last_packet_sent; sent_iter++) {
        gettimeofday(&curr_time, NULL);
        long time_diff = get_time_diff(&curr_time, &(window->timestamp[sent_iter]));
        if(time_diff > g->data_timeout_millsec) {
          /* Timeout detected */
          console_log("Peer %d: DATA packet with SEQ %u TIMEOUT(%ld ms)! Resend now...",
                      g->g_config->identity, sent_iter, time_diff);
          gettimeofday(&(window->timestamp[sent_iter]), NULL);
          send_packet(i, window->buffer[sent_iter], g);

          /* Congestion Control */
          congctrl_t* cc = &window->cc;
          cc->state = SLOW_START;
          cc->ssthresh = MAX(2, cc->cwnd/2);
          cc->cwnd = 1;
          window->max_window_size = 1;
          cc_log(cc);
        }
      }

      while (window->last_packet_sent < window->last_packet_available) {
        /* Send packets in range (last_packet_sent, last_packet_available] */
        gettimeofday(&(window->timestamp[window->last_packet_sent+1]), NULL);
        send_packet(i, window->buffer[window->last_packet_sent+1], g);
        window->last_packet_sent++;
      }
    }
  }
}

/**
 * Handle downloading.
 * This function will be invoked periodically in the peer's main event loop
 * @param g Global state.
 */
void do_download(g_state_t *g) {
  int i;

  /* Check for done download connection */
  for (i = 0; i < MAX_PEER_NUM; i++) {
    if (g->download_conn_pool[i] != NULL) {
      recv_window_t * recv_window = g->download_conn_pool[i];

      if (recv_window->state == DONE) {
        console_log("Peer %d: Closing download connection with peer %d on chunk %s",
                    g->g_config->identity, i, recv_window->chunk_hash);

        /* Upon finish downloading, write all data packet's payload to master_data_file */
        any_t m_file_offset;
        hashmap_get(g->g_session->chunk_map, recv_window->chunk_hash, &m_file_offset);
        console_log("Peer %d: This chunk's offset in output file is: %ld",
                    g->g_config->identity, (intptr_t)m_file_offset);

        try_file(g->g_session->temp_output_file);
        FILE *output_f = fopen(g->g_session->temp_output_file, "r+");
        int packet_idx;
        for (packet_idx = 1; packet_idx <= MAX_SEQ_NUM; packet_idx++) {
          size_t data_size;
          packet_t *data_packet = recv_window->buffer[packet_idx];
          size_t w_offset = ((intptr_t) m_file_offset) * CHUNK_SIZE + (packet_idx-1) * DATA_PACKET_SIZE;
          fseek(output_f, w_offset, SEEK_SET);
          if (packet_idx == MAX_SEQ_NUM) {
            data_size = CHUNK_SIZE % DATA_PACKET_SIZE;
          } else {
            data_size = DATA_PACKET_SIZE;
          }
          fwrite(data_packet->payload, sizeof(uint8_t), data_size, output_f);
        }
        fclose(output_f);

        // Update global states.
        hashmap_remove(g->g_session->nlchunk_map, recv_window->chunk_hash);
        g->curr_download_conn_cnt--;
        free_recv_window(g, i);
        dump_session(g->g_session);

        if (hashmap_length(g->g_session->nlchunk_map) == 0) {
          /* All user requested chunks are ready in local storage */
          assemble_chunks(g->g_config->chunks->master_data_file,
                          g->g_config->chunks->has_chunk_map,
                          g->g_session->temp_output_file,
                          g->g_session->chunk_map);

          rename(g->g_session->temp_output_file, g->g_session->output_file);
          console_log("[Finish Downloading] File is at %s", g->g_session->output_file);
          session_free(g->g_session);
          g->g_session = NULL;
        }
      }
    }
  }

  /* Check for pending GET packet */
  if (g->pending_get_packets->size > 0) {
    for (i = 0; i < g->pending_get_packets->size; i++) {
      pending_packet_t *pending_get_packet =
              (pending_packet_t*)dequeue(g->pending_get_packets);
      short des_peer = pending_get_packet->to_peer;
      /* If sending fails, will re-enqueue this GET packet */
      try_send_get_packet(des_peer, pending_get_packet->packet, g);
    }
  }
}

/**
 * @brief Congestion control related log.
 * @param fd File descriptor of the log file.
 * @param sender Sender id.
 * @param recver Receiver id.
 * @param sz Current window size.
 */
void cc_log(congctrl_t* cc) {
  static char line[CC_LOG_LINESZ+1];
  static struct timeval now;
  gettimeofday(&now, NULL);

  flock(cc->fd, LOCK_EX);
  int linesz = snprintf(line, CC_LOG_LINESZ, "%hi->%hi\t%li\t%f\n",
                        cc->sender, cc->recver,
                        get_time_diff(&now, &cc->start),
                        cc->cwnd);
  write(cc->fd, line, linesz);
  flock(cc->fd, LOCK_UN);
}
