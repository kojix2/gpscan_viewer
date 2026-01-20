#include "Utils.h"

#include <QStringList>

namespace Utils {

QString formatSize(quint64 bytes) {
  constexpr double KB = 1024.0;
  constexpr double MB = KB * 1024.0;
  constexpr double GB = MB * 1024.0;
  constexpr double TB = GB * 1024.0;

  if (bytes >= TB) {
    return QString::number(static_cast<double>(bytes) / TB, 'f', 2) + " TB";
  }
  if (bytes >= GB) {
    return QString::number(static_cast<double>(bytes) / GB, 'f', 2) + " GB";
  }
  if (bytes >= MB) {
    return QString::number(static_cast<double>(bytes) / MB, 'f', 2) + " MB";
  }
  if (bytes >= KB) {
    return QString::number(static_cast<double>(bytes) / KB, 'f', 2) + " KB";
  }
  return QString::number(bytes) + " B";
}

QString buildFullPath(const TreeNode *node) {
  if (!node) {
    return QString();
  }

  QStringList parts;
  const TreeNode *current = node;

  while (current) {
    if (!current->name.isEmpty()) {
      parts.prepend(current->name);
    }
    current = current->parent;
  }

  if (parts.isEmpty()) {
    return QStringLiteral("/");
  }

  // Avoid duplication when the root is "/".
  if (parts.first() == QStringLiteral("/")) {
    if (parts.size() == 1) {
      return QStringLiteral("/");
    }
    parts.removeFirst();
    return QStringLiteral("/") + parts.join(QStringLiteral("/"));
  }

  return parts.join(QStringLiteral("/"));
}

} // namespace Utils
