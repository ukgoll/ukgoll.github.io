#include <stdio.h>
#include <stdint.h>

struct RBTree
{
  struct RBTreeNode *root; //
  uint64_t count;          // 这个树有多少个节点
};

struct RBTreeNode
{
  char color; // R (82) red, B(66) black
  void *key;
  struct RBTreeNode *left;
  struct RBTreeNode *right;
  struct RBTreeNode *parent;
};

// 旋转之前保证了 node 已经在树 T 里面了，旋转只概念 left right parent 指针指向
void leftRotate(struct RBTree *T, struct RBTreeNode *node)
{
  struct RBTreeNode *node_right = node->right;
  if (node_right == NULL)
    return;
  node->right = node_right->left;
  if (node_right->left != NULL)
  {
    node_right->left->parent = node;
  }
  node_right->parent = node->parent;
  if (node->parent == NULL)
  {
    T->root = node_right;
  }
  else if (node->parent->left == node)
  {
    node->parent->left = node_right;
  }
  else
  {
    node->parent->right = node_right;
  }
  node_right->left = node;
  node->parent = node_right;
}

// 旋转之前保证了 node 已经在树 T 里面了，旋转只概念 left right parent 指针指向
void rightRotate(struct RBTree *T, struct RBTreeNode *node)
{
  struct RBTreeNode *node_left = node->left;
  if (node_left == NULL)
    return;
  node->left = node_left->right;
  if (node_left->right != NULL)
  {
    node_left->right->parent = node;
  }
  node_left->parent = node->parent;
  if (node->parent == NULL)
  {
    T->root = node_left;
  }
  else if (node->parent->left == node)
  {
    node->parent->left = node_left;
  }
  else
  {
    node->parent->right = node_left;
  }
  node_left->right = node;
  node->parent = node_left;
}

int main(int argc, char const *argv[])
{
  uint64_t count = 18446744073709551615llu; // (2 ** 64) - 1;
  printf("count is %llu, %llu, %llu\n", count, count + 1, count + 2);
  return 0;
}
