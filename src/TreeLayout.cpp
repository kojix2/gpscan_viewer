#include "TreeLayout.h"

#include <algorithm>
#include <memory>
#include <queue>

namespace {

void mirrorRects(TreeNode *node, const QRectF &rootBounds) {
  if (!node) {
    return;
  }

  const QRectF r = node->rect;
  const double newX = rootBounds.x() + rootBounds.width() -
                      (r.x() - rootBounds.x()) - r.width();
  const double newY = rootBounds.y() + rootBounds.height() -
                      (r.y() - rootBounds.y()) - r.height();
  node->rect = QRectF(newX, newY, r.width(), r.height());

  for (TreeNode *child : node->children) {
    mirrorRects(child, rootBounds);
  }
}

struct LayoutNode {
  TreeNode *leaf = nullptr;
  LayoutNode *left = nullptr;
  LayoutNode *right = nullptr;
  double size = 0.0;
};

LayoutNode *
buildBalancedTree(const QVector<TreeNode *> &items,
                  std::vector<std::unique_ptr<LayoutNode>> &storage) {
  auto makeNode = [&](TreeNode *leaf, LayoutNode *left, LayoutNode *right,
                      double size) {
    storage.emplace_back(std::make_unique<LayoutNode>());
    LayoutNode *node = storage.back().get();
    node->leaf = leaf;
    node->left = left;
    node->right = right;
    node->size = size;
    return node;
  };

  struct NodeRef {
    LayoutNode *node = nullptr;
    double size = 0.0;
  };

  struct Compare {
    bool operator()(const NodeRef &a, const NodeRef &b) const {
      return a.size > b.size;
    }
  };

  std::priority_queue<NodeRef, std::vector<NodeRef>, Compare> queue;

  for (TreeNode *item : items) {
    if (!item || item->size == 0) {
      continue;
    }
    LayoutNode *leaf =
        makeNode(item, nullptr, nullptr, static_cast<double>(item->size));
    queue.push({leaf, leaf->size});
  }

  if (queue.empty()) {
    return nullptr;
  }

  while (queue.size() > 1) {
    NodeRef a = queue.top();
    queue.pop();
    NodeRef b = queue.top();
    queue.pop();

    LayoutNode *parent = makeNode(nullptr, a.node, b.node, a.size + b.size);
    queue.push({parent, parent->size});
  }

  return queue.top().node;
}

void layoutBinary(LayoutNode *node, const QRectF &rect,
                  QVector<TreeNode *> &leaves) {
  if (!node) {
    return;
  }

  if (node->leaf) {
    node->leaf->rect = rect;
    leaves.push_back(node->leaf);
    return;
  }

  if (!node->left || !node->right) {
    return;
  }

  double total = node->left->size + node->right->size;
  if (total <= 0.0) {
    return;
  }

  double ratio = node->left->size / total;
  if (rect.width() >= rect.height()) {
    double w = rect.width() * ratio;
    QRectF leftRect(rect.x(), rect.y(), w, rect.height());
    QRectF rightRect(rect.x() + w, rect.y(), rect.width() - w, rect.height());
    layoutBinary(node->left, leftRect, leaves);
    layoutBinary(node->right, rightRect, leaves);
  } else {
    double h = rect.height() * ratio;
    QRectF topRect(rect.x(), rect.y(), rect.width(), h);
    QRectF bottomRect(rect.x(), rect.y() + h, rect.width(), rect.height() - h);
    layoutBinary(node->left, topRect, leaves);
    layoutBinary(node->right, bottomRect, leaves);
  }
}

void layoutGroup(const QVector<TreeNode *> &items, const QRectF &bounds,
                 std::vector<std::unique_ptr<LayoutNode>> &storage);

void layoutNode(TreeNode *node, const QRectF &bounds,
                std::vector<std::unique_ptr<LayoutNode>> &storage) {
  if (!node) {
    return;
  }

  node->rect = bounds;
  if (node->children.isEmpty()) {
    return;
  }

  QVector<TreeNode *> files;
  QVector<TreeNode *> dirs;
  files.reserve(node->children.size());
  dirs.reserve(node->children.size());

  double fileSize = 0.0;
  double dirSize = 0.0;

  for (TreeNode *child : node->children) {
    if (!child || child->size == 0) {
      continue;
    }
    if (child->isDir) {
      dirs.push_back(child);
      dirSize += static_cast<double>(child->size);
    } else {
      files.push_back(child);
      fileSize += static_cast<double>(child->size);
    }
  }

  double total = fileSize + dirSize;
  if (total <= 0.0) {
    return;
  }

  if (!files.isEmpty() && !dirs.isEmpty()) {
    double ratio = fileSize / total;
    if (bounds.width() >= bounds.height()) {
      double w = bounds.width() * ratio;
      QRectF fileRect(bounds.x(), bounds.y(), w, bounds.height());
      QRectF dirRect(bounds.x() + w, bounds.y(), bounds.width() - w,
                     bounds.height());
      layoutGroup(files, fileRect, storage);
      layoutGroup(dirs, dirRect, storage);
    } else {
      double h = bounds.height() * ratio;
      QRectF fileRect(bounds.x(), bounds.y(), bounds.width(), h);
      QRectF dirRect(bounds.x(), bounds.y() + h, bounds.width(),
                     bounds.height() - h);
      layoutGroup(files, fileRect, storage);
      layoutGroup(dirs, dirRect, storage);
    }
  } else {
    // Either only files or only dirs
    QVector<TreeNode *> all = files;
    all += dirs;
    layoutGroup(all, bounds, storage);
  }
}

void layoutGroup(const QVector<TreeNode *> &items, const QRectF &bounds,
                 std::vector<std::unique_ptr<LayoutNode>> &storage) {
  if (items.isEmpty()) {
    return;
  }

  LayoutNode *root = buildBalancedTree(items, storage);
  if (!root) {
    return;
  }

  QVector<TreeNode *> leaves;
  layoutBinary(root, bounds, leaves);

  for (TreeNode *leaf : leaves) {
    layoutNode(leaf, leaf->rect, storage);
  }
}

} // namespace

void TreeLayout::layout(TreeNode *root, const QRectF &bounds) {
  std::vector<std::unique_ptr<LayoutNode>> storage;
  layoutNode(root, bounds, storage);

  // GrandPerspective-compatible orientation: mirror both X and Y within the
  // root bounds.
  mirrorRects(root, bounds);
}
