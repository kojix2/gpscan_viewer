#pragma once

#include <QRectF>

#include "TreeModel.h"

class TreeLayout {
public:
  static void layout(TreeNode *root, const QRectF &bounds);
};
