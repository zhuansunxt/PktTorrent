#ifndef _SPIFFY_H_
#define _SPIFFY_H_

#include <sys/types.h>
#include <sys/socket.h>
#include "debug.h"

struct spiffy_header_s {
	int ID;
	int lSrcAddr;
	int lDestAddr;
	short lSrcPort;
	short lDestPort;
};
typedef struct spiffy_header_s spiffy_header;

/**
 * Spiffy wrapper for system API sendto()
 * @param s socket fd
 * @param msg data pointer
 * @param len size of data
 * @param flags same parameter in system call sendto()
 * @param to destination address
 * @param tolen destination address length
 * @return Upon successful completion, the number of bytes which were sent is returned.
 * Otherwise, -1 is returned and the global variable errno is set to indicate the error.
 */
ssize_t spiffy_sendto(int s, const void *msg, size_t len, int flags, const struct sockaddr *to, socklen_t tolen);

/**
 * Spiffy wrapper for system API recvfrom
 * @param socket socket fd
 * @param buffer receiving buffer
 * @param size buffer size
 * @param flags
 * @param addr sender address
 * @param lengthptr sender address length
 * @return
 */
int spiffy_recvfrom (int socket, void *buffer, size_t size, int flags, struct sockaddr *addr, socklen_t *lengthptr);

/**
 * Spiffy wrapper initializer
 * @param lNodeID node identity in network topology
 * @param addr address of the node
 * @param addrlen address length
 * @return 0 on success. -1 on internal error. 1 on no SPIFFY_ROUTER environment set
 */
int spiffy_init (long lNodeID, const struct sockaddr *addr, socklen_t addrlen);

#endif /* _SPIFFY_H_ */
