#pragma once

#include <QRectF>
#include <QString>
#include <QVector>

struct TreeNode {
  QString name;
  quint64 size = 0;
  bool isDir = false;

  TreeNode *parent = nullptr;
  QVector<TreeNode *> children;
  QRectF rect;
};

class TreeModel {
public:
  TreeModel() = default;
  ~TreeModel();

  TreeNode *root() const { return rootNode; }
  void setRoot(TreeNode *node) { rootNode = node; }

  void computeDerivedSizes();

  static void deleteSubtree(TreeNode *node);
  static quint64 computeSize(TreeNode *node);

private:
  TreeNode *rootNode = nullptr;
};
