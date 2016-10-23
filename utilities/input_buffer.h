#include <stdlib.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include "../core/session.h"

#define USERBUF_SIZE 8191

struct user_iobuf {
  char *buf;
  unsigned int cur;
};

struct user_iobuf *create_userbuf();

void process_user_input(int fd, struct user_iobuf *userbuf, 
			void (*handle_line)(char *, void *, g_state_t*), void *cbdata, g_state_t *g);

