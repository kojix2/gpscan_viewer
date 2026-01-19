#pragma once

#include <QString>

#include "TreeModel.h"

namespace Utils {

// バイト数を人間が読みやすい形式に変換 (例: 1024 → "1.0 KB")
QString formatSize(quint64 bytes);

// ノードのフルパスを構築
QString buildFullPath(const TreeNode *node);

} // namespace Utils
