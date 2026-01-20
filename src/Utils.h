#pragma once

#include <QString>

#include "TreeModel.h"

namespace Utils {

// Convert bytes to a human-readable format (e.g., 1024 -> "1.0 KB")
QString formatSize(quint64 bytes);

// Build the full path for a node
QString buildFullPath(const TreeNode *node);

} // namespace Utils
