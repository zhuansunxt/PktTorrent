cmake_minimum_required(VERSION 3.6)
project(PacTorrent)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    example/A.gif
    example/B.gif
    bt_parse.c
    bt_parse.h
    chunk.c
    chunk.h
    client.c
    debug-text.h
    debug.c
    debug.h
    debugparse.pl
    hashmap.c
    hashmap.h
    hupsim.pl
    input_buffer.c
    input_buffer.h
    make_chunks.c
    Makefile
    nodes.map
    packet.c
    packet.h
    peer.c
    PROJ2_HOWTO
    readme.txt
    server.c
    sha.c
    sha.h
    spiffy.c
    spiffy.h
    test_hashmap.c
    test_input_buffer.c
    test_packet.c
    topo.map)

add_executable(PacTorrent ${SOURCE_FILES})