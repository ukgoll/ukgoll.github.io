#include <stdio.h>

struct heap_node
{
  struct heap_node *left;
  struct heap_node *right;
  struct heap_node *parent;
};

typedef int (*heap_compare_fn)(const struct heap_node *a,
                               const struct heap_node *b);
struct heap
{
  struct heap_node *max;
  unsigned int nelts;
};

// 构建最大堆
void max_heapify(struct heap *hp, struct heap_node *hn, heap_compare_fn hcf);

// 交换 heap 中的某两个节点
void swap_heap_node(struct heap *hp, struct heap_node *parent, struct heap_node *child);

void build_max_heap(struct heap *hp);

int main(int argc, char const *argv[])
{
  return 0;
}

// 这个应该是要搭配其他函数在特定情况下使用的，因为交换 之后，parent 不一定 会比他的 parent 还要大
// 所以应用这个函数，`需要保证的是从 heap 最底下开始 heapify` 这个很重要，不然就好出现像上面所有的情况
void max_heapify(struct heap *hp, struct heap_node *hn, heap_compare_fn hcf)
{
  if (hn == NULL)
    return;
  struct heap_node *hnl = hn->left;
  struct heap_node *hnr = hn->right;
  struct heap_node *tmp = hn;
  if (hcf(hnl, hn))
  {
    tmp = hnl;
  }
  if (hcf(hnr, hn))
  {
    tmp = hnr;
  }
  if (tmp != hn)
  {
    swap_heap_node(hp, hn, tmp);
    max_heapify(hp, tmp, hcf);
  }
}

// TODO 这里是换内容，可以尝试换地址
void swap_heap_node(struct heap *hp, struct heap_node *parent, struct heap_node *child)
{
  struct heap_node *sibling;
  struct heap_node t;
  // 改变内容，但是不改变在内存中的地址
  t = *parent;
  *parent = *child;
  *child = t;

  parent->parent = child;
  if (child->left == child)
  {
    child->left = parent;
    sibling = child->right;
  }
  else
  {
    child->right = parent;
    sibling = child->left;
  }
  if (sibling != NULL)
    sibling->parent = child;

  // 考虑到 child 的左右，
  if (parent->left != NULL)
    parent->left->parent = parent;
  if (parent->right != NULL)
    parent->right->parent = parent;

  // 考虑到 parent 的长辈
  if (child->parent == NULL)
    hp->min = child;
  else if (child->parent->left == parent)
    child->parent->left = child;
  else
    child->parent->right = child;
}

void build_max_heap(struct heap *hp)
{
}
