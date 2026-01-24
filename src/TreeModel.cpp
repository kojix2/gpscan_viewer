#include "TreeModel.h"

TreeModel::~TreeModel() {
  deleteSubtree(rootNode);
  rootNode = nullptr;
}

void TreeModel::deleteSubtree(TreeNode *node) {
  if (!node) {
    return;
  }

  for (TreeNode *child : node->children) {
    deleteSubtree(child);
  }
  delete node;
}

quint64 TreeModel::computeSize(TreeNode *node) {
  if (!node) {
    return 0;
  }

  if (node->children.isEmpty()) {
    return node->size;
  }

  quint64 total = 0;
  for (TreeNode *child : node->children) {
    total += computeSize(child);
  }

  if (node->size == 0) {
    node->size = total;
  }

  return node->size;
}

void TreeModel::computeDerivedSizes() { computeSize(rootNode); }
