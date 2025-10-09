#include <stdio.h>

struct uv__queue
{
  struct uv__queue *next;
  struct uv__queue *prev;
};

union
{
  void *heap[3];
  struct uv__queue queue;
} node;

int main()
{
  printf("heap[0] = %p\n", (void *)&node.heap[0]);
  printf("queue   = %p\n", (void *)&node.queue);
  printf("queue.next = %p\n", (void *)&node.queue.next);
  printf("queue.prev = %p\n", (void *)&node.queue.prev);
  printf("heap[1]    = %p\n", (void *)&node.heap[1]);
  printf("heap[2]    = %p\n", (void *)&node.heap[2]);
}
