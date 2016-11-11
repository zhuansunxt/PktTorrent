################################################################################
# README                                                                       #
#                                                                              #
# Description: This file serves as a README and documentation for a P2P file   #
#               transfer system with congestion control.                       #
#                                                                              #
# Authors: Xiaotong Sun (xiaotons@cs.cmu.edu)                                  #
#           Longqi Cai (longqic@cs.cmu.edu)                                    #
#                                                                              #
################################################################################


[TOC-1] Table of Contents
--------------------------------------------------------------------------------

    [TOC-1] Table of Contents
    [DES-2] Description of Files
    [DAI-3] Design and Implementation

[DES-2] Description of Files
--------------------------------------------------------------------------------

Documentations:

    readme.txt          - Design documentation
    tests.txt            - Test documentation
    vulnerabilities.txt - Documentations on potential vulnerability of the system

Source code:
    
    core/*.[hc]          - Core implementation, including upload/download and peer
                          communication logic
    debug/*.[hc]        - Debug utilities.
    lib/*.[hc]          - Data stucture library from open-source project.
    packet/*.[hc]        - Packet structure definition.
    utilities/*.[hc]    - All utilities and helpers that used across project.
    utests/*.[hc]        - Unit tests.
    peer.c               - Main entrance of a single peer.


[DAI-4] Design and Implementation
--------------------------------------------------------------------------------

4.1 A peer's Life Cycle

A peer in the P2P system can serve up to one user at a time. The user identifies
a `get-chunk-file` to a peer to request downloading a file. The whole downloading
process for this user is called a `session`. When a peer is during a session, no
other users can be served. After downloading the file, the session will be over.

A peer will firstly check whether all requested chunks are local. Chunks that a
peer can not access locally are called `non-local chunk`. A peer would flood WHOHAS
packet then to identify these non-local chunks' location (which peers in the system
have them locally). If another peer do own the non-local chunk, a peer will receive
IHAVE packet from this peer, sent out a GET packet to indicate its interest in 
getting this chunk, and establish a download connection. During a download connection,
a peer will keep receiving DATA packets containing data for the chunk, and reply
ACK packet to confirm the receipt. Upon a chunk's data is fully received, the 
downloading peer will close the connection. If all non-local chunks requested are 
downloaded, a peer will tell the user the final output file's location. 

To make this whole process clear, the below diagram should help you to understand:

         chunks
User ------------> Peer                       Other peers in the system                         
                    |                                      |
                    |                                      |
          Check non-local chunks                           |
                    |                                      |
                    | ---------------------------------->  |    
                    |        Broadcast WHOHAS packets      |
                    |                                      |
                    | <----------------------------------- | 
                    |        Receive IHAVE packets         |
                    |                                      |
                    | ---------------------------------->  | 
                    |          Sent GET packets            |
                    |                                      |
                    | <----------------------------------- | 
                    |        Receive DATA packet           |
                    | ---------------------------------->  | 
                    |          Reply ACK packets           |
                    | <----------------------------------- | 
                    |        Receive DATA packet           |
                    | ---------------------------------->  | 
                    |          Reply ACK packets           |
                    |                                      |
                    |              ........                |
                    |                                      |
        Receive all chunks requested            Close upload connection
            Close download connection
                    |
User <------------  |
       output file

4.2 Download/Upload Connection

For a downloader in this system, a download connecton will be constructed for every 
chunk's downloading, which can be represented by the below pseudo code.

              *************************************************
              * typedef struct recv_window_s {                *
              *  char chunk_hash[HASH_STR_LEN];               *
              *  download_state state;                        *
              *  packet_t* buffer[MAX_DATAPKT_FOR_CHUNK];     *
              *  uint32_t next_packet_expected;               *
              *  struct timeval last_datapac_recvd;           *
              *  unsigned accumulate_bytes;                   *
              * } recv_window_t;                              *
              *************************************************

When it receives a DATA packet, it will check whether its sequence number is larger,
equal or smaller than the `next_packet_expected` number. If the sequence number is
larger than it, it means there's a gap, and the downloader will send duplicate 
ACK to the uploader to indicate this lost cap. This mechanism is similar to TCP 
fast retransmit.

For a peer acts like a downloader, this connection will be maintained in a global
downloading pool. A connecton will only be active if a chunk is being downloaded.
At any given time, only one connection could be maintained with each other peer in
the system.

For an uploader in the system, similarly an upload connection will be constructed
for a specific downloader, which can be represented as below.

*******************************************************************************
* typedef struct send_window_s {                                              *
*  packet_t *buffer[MAX_SEQ_NUM+1];  // buffer all DATA packet to be sent.    *
*  size_t max_window_size;                                                    *
*                                                                             *
*  uint32_t last_packet_acked;       // last packet that get ACKed.           *
*  uint32_t last_packet_sent;        // last packet that is sent out.         *
*  uint32_t last_packet_available;   // serves as window boundary.            *
*                                                                             *
*  uint8_t dup_ack_map[MAX_SEQ_NUM+1];       // keep track of duplicate ACK.  *
*  struct timeval timestamp[MAX_SEQ_NUM+1]; // Timer for each DATA packet.    *
*  congctrl_t cc;                            // congestion control.           *                  
* } send_window_t;                                                            *
*******************************************************************************

The implementation for an upload connecton is more complex, since it involves
congeston control, fast retransmit and sliding window mechanism. Similar to the
download connection, it is maintained in a global pool and only one connection
can be active for a single downloader peer.

4.3 Congestion Control

We implement simple congeston control mechanism in this project. Similar to TCP,
we implement slow start and congestion avoidance, while the fast recovery phase
is left for future work. 

In the slow start state, the window size on the uploader side begins at 1 packet
and increase by 1 packet every time a transmitted packet is acknowledged. This
process results in a doubling of the sending rate every RTT. Thus, the send rate
starts slow but grows exponentially during the slow start phase. The slow start
state is over until the threshold window size is reached, transitting to congeston
avoidance state.

On entry to the congestion avoidance state, the value of the window size is increased
by just a single packet every RTT, and this is achieved by increase the window size
by (1/window_size) in the code.

Two events' occurence will cause the congestion avoidance state to be transitted
back to slow start state: DATA packet timeout and receiving duplicate ACK packet.


                           ssthresh reached
          **************  ------------------> ************************
          * Slow Start *                      * Congestion Avoidance *
          ************** <------------------- ************************
                          timeout / dup ACK

The `cc` member in an upload connecton acts as a controller
for congeston control during the upload process. To verify the effect, one could check
Problem2-peer.txt every time after an upload connection finishes to see variance of 
the sender's window size.

4.4 Concurrency

The concurrency of this P2P system is achieved by using select() system call to
multiplex network I/O to a single thread. One peer in the system can establish
multiple download connections with multiple peers, and one peer can simultanuously
upload multiple chunks to more than one peer. The restriction of the concurrecy
is

1) One peer can only download one file chunk from another peer at any given time.
In other words, two specific peers are not able to transfer more than one chunks
at a time. Instead, file transferring between these two peers are serialized.

2) Maximum connection can be set by passing througth command line argument -m. This
value restrict the parallelism on download/upload connection.

4.5 Fault tolerance

This P2P system is able to detect and recover from peer crashes, as long as the file
chunk can be retrieved from other alive peers in the system. The approach to this 
fault tolerance property is by setting an extra timeout for every download connection
(struct timeval last_datapac_recvd member in recv_window). Every time a downloader
peer receives a DATA packet, it will update the timestamp indicating the last time
a peer receives a DATA packet. Upon a timeout threshold is reached by checking this
timestamp, the peer will consider the other side of the connection crashes and redo
the broadcasting of WHOHAS packets to locate the not yet finished file chunk on 
other peers in the system.

4.6 Other Design Decisions

- Queuing GET packet: there are two potential reasons for a built GET packet can not
be sent right away: maximum number of connection is reached, or there's already an
existing connection with the target peer. When a GET packet could not be sent,
this packet will be queued and a peer will try to resend it periodically until
succeeding in doing so.

- Global number of peers is static: this project does not support dynamically adding
peers to the current system. The number of peers should be determined before starting
the system. The default value of the maximum number of peers is 64.









