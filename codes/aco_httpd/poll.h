#ifndef poll_header
#define poll_header
#include <netinet/ip.h>
#include "lib/aco.h"

typedef enum poll_event {
  eread=0,
  ewrite=1
} epe;


typedef struct conn_s{
  int fd;
  aco_t *routine;
  struct sockaddr_in *sa;
} conn_t;

typedef struct poll_s
{
  int fd;
  aco_t *main_co;
  aco_share_stack_t *s_stack;
} poll_t;

poll_t *get_loop();

// get parent fd
int poll_init(poll_t *loop);

// add event
int poll_add(poll_t *loop, int fd, epe pe, conn_t *c);


// delete event
int poll_delete(conn_t *conn);



#endif
