#include "CanvasWidget.h"

#include <QImage>
#include <QMouseEvent>
#include <QPainter>
#include <QToolTip>

#include <algorithm>
#include <cmath>

#include "TreeLayout.h"
#include "Utils.h"

namespace {

QVector<QColor> defaultPalette() {
  // Colors similar to GrandPerspective's default palette
  return {
      QColor(0xE6, 0x4B, 0x3C), // red
      QColor(0xF5, 0xA6, 0x23), // orange
      QColor(0xF8, 0xE7, 0x1C), // yellow
      QColor(0x7E, 0xD3, 0x21), // green
      QColor(0x50, 0xE3, 0xC2), // cyan
      QColor(0x4A, 0x90, 0xE2), // blue
      QColor(0xBD, 0x10, 0xE0), // purple
      QColor(0xD0, 0x02, 0x1B), // crimson
      QColor(0x8B, 0x57, 0x2A), // brown
      QColor(0x41, 0x41, 0x41), // gray
      QColor(0x90, 0xA4, 0xAE), // light gray
      QColor(0x7B, 0x8D, 0x8E)  // dark gray
  };
}

} // namespace

CanvasWidget::CanvasWidget(QWidget *parent) : QWidget(parent) {
  setMouseTracking(true);
  palette = defaultPalette();
}

void CanvasWidget::setColorMappingMode(ColorMappingMode mode) {
  if (colorMappingMode != mode) {
    colorMappingMode = mode;
    update();
  }
}

void CanvasWidget::setModel(std::shared_ptr<TreeModel> newModel) {
  model = std::move(newModel);
  selectedNode = nullptr;
  hoveredNode = nullptr;
  update();
}

void CanvasWidget::paintEvent(QPaintEvent *event) {
  Q_UNUSED(event);
  QPainter painter(this);
  painter.fillRect(rect(), Qt::black);

  if (!model || !model->root()) {
    return;
  }

  // Draw pixel-by-pixel using QImage
  QImage image(size(), QImage::Format_RGB32);
  image.fill(Qt::black);

  drawNode(image, model->root(), 0);

  painter.drawImage(0, 0, image);

  // Highlight selected node
  if (selectedNode) {
    drawSelection(painter, selectedNode);
  }
}

void CanvasWidget::mousePressEvent(QMouseEvent *event) {
  if (!model || !model->root()) {
    return;
  }

  TreeNode *hit = findNode(model->root(), event->position());
  if (hit != selectedNode) {
    selectedNode = hit;
    emit selectedNodeChanged(selectedNode);
    update();
  }
}

void CanvasWidget::resizeEvent(QResizeEvent *event) {
  QWidget::resizeEvent(event);
  if (model && model->root()) {
    QRectF bounds(0, 0, width(), height());
    TreeLayout::layout(model->root(), bounds);
  }
}

void CanvasWidget::mouseMoveEvent(QMouseEvent *event) {
  updateTooltip(event->position());
}

bool CanvasWidget::event(QEvent *event) {
  if (event->type() == QEvent::ToolTip) {
    auto *helpEvent = static_cast<QHelpEvent *>(event);
    updateTooltip(helpEvent->pos());
    return true;
  }
  return QWidget::event(event);
}

void CanvasWidget::updateTooltip(const QPointF &pos) {
  if (!model || !model->root()) {
    QToolTip::hideText();
    return;
  }

  TreeNode *node = findNode(model->root(), pos);
  if (node && node != hoveredNode) {
    hoveredNode = node;
    QString fullPath = Utils::buildFullPath(node);
    QString sizeText = Utils::formatSize(node->size);
    QString tip = QString("%1\n%2").arg(fullPath, sizeText);
    QToolTip::showText(mapToGlobal(pos.toPoint()), tip, this);
  } else if (!node) {
    hoveredNode = nullptr;
    QToolTip::hideText();
  }
}

void CanvasWidget::drawBevelRect(QImage &image, const QRectF &rect, const QColor &base) {
  int x0 = static_cast<int>(rect.x() + 0.5);
  int y0 = static_cast<int>(rect.y() + 0.5);
  int rectWidth = static_cast<int>(rect.x() + rect.width() + 0.5) - x0;
  int rectHeight = static_cast<int>(rect.y() + rect.height() + 0.5) - y0;

  if (rectWidth <= 0 || rectHeight <= 0) {
    return;
  }

  const int imgWidth = image.width();
  const int imgHeight = image.height();

  // Symmetric bevel ("chocolate block"):
  // - darker near edges
  // - same shading in all directions (no diagonal / directional lighting)
  const double invW = 1.0 / static_cast<double>(rectWidth);
  const double invH = 1.0 / static_cast<double>(rectHeight);

  const double edgeFade = 0.07;         // normalized edge-distance units
  const double edgeDarkStrength = 0.62; // 0..1

  const int baseR = base.red();
  const int baseG = base.green();
  const int baseB = base.blue();

  const int startX = std::max(0, x0);
  const int endX = std::min(imgWidth, x0 + rectWidth);
  const int startY = std::max(0, y0);
  const int endY = std::min(imgHeight, y0 + rectHeight);

  for (int imgY = startY; imgY < endY; ++imgY) {
    const int y = imgY - y0;
    QRgb *scanline = reinterpret_cast<QRgb *>(image.scanLine(imgY));
    const double v = (static_cast<double>(y) + 0.5) * invH;

    for (int imgX = startX; imgX < endX; ++imgX) {
      const int x = imgX - x0;
      const double u = (static_cast<double>(x) + 0.5) * invW;

      // Distance to nearest edge
      const double edgeDist = std::min({u, v, 1.0 - u, 1.0 - v});
      const double edge = std::clamp(1.0 - (edgeDist / edgeFade), 0.0, 1.0);

      const double factor = std::clamp(1.0 - edgeDarkStrength * edge, 0.0, 1.0);
      const int r = std::clamp(static_cast<int>(std::lround(baseR * factor)), 0, 255);
      const int g = std::clamp(static_cast<int>(std::lround(baseG * factor)), 0, 255);
      const int b = std::clamp(static_cast<int>(std::lround(baseB * factor)), 0, 255);
      scanline[imgX] = qRgb(r, g, b);
    }
  }
}

void CanvasWidget::drawNode(QImage &image, TreeNode *node, int depth) {
  if (!node) {
    return;
  }

  // Skip rectangles smaller than 1 pixel
  if (node->rect.width() < 1.0 || node->rect.height() < 1.0) {
    return;
  }

  QColor base = colorForNode(node, depth);
  drawBevelRect(image, node->rect, base);

  for (TreeNode *child : node->children) {
    drawNode(image, child, depth + 1);
  }
}

void CanvasWidget::drawSelection(QPainter &painter, TreeNode *node) {
  if (!node) return;

  painter.setPen(QPen(Qt::yellow, 2));
  painter.setBrush(Qt::NoBrush);
  painter.drawRect(node->rect.adjusted(1, 1, -1, -1));
}

QColor CanvasWidget::colorForNode(const TreeNode *node, int depth) const {
  if (!node) {
    return QColor(128, 128, 128);
  }

  if (palette.isEmpty()) {
    return QColor(128, 128, 128);
  }

  auto extensionKey = [](const QString &name) -> QString {
    int dot = name.lastIndexOf('.');
    if (dot <= 0 || dot == name.size() - 1) {
      return QString();
    }
    return name.mid(dot + 1).toLower();
  };

  auto folderKey = [](const TreeNode *n) -> QString {
    if (!n) {
      return QString();
    }
    if (n->isDir) {
      return n->name;
    }
    return n->parent ? n->parent->name : QString();
  };

  auto topFolderKey = [](const TreeNode *n) -> QString {
    if (!n) {
      return QString();
    }
    const TreeNode *cur = n;
    // Walk to the node directly under the root.
    while (cur->parent && cur->parent->parent) {
      cur = cur->parent;
    }
    // If the root has name "/", cur might still be a file; use its parent when possible.
    if (!cur->isDir && cur->parent) {
      return cur->parent->name;
    }
    return cur->name;
  };

  int index = 0;
  switch (colorMappingMode) {
  case ColorMappingMode::Extension: {
    // Matches GrandPerspective's "extension" mapping idea.
    QString key = node->isDir ? node->name : extensionKey(node->name);
    if (key.isEmpty()) {
      key = node->name;
    }
    index = static_cast<int>(qHash(key) % static_cast<uint>(palette.size()));
    break;
  }
  case ColorMappingMode::Name: {
    index = static_cast<int>(qHash(node->name) % static_cast<uint>(palette.size()));
    break;
  }
  case ColorMappingMode::Folder: {
    QString key = folderKey(node);
    if (key.isEmpty()) {
      key = node->name;
    }
    index = static_cast<int>(qHash(key) % static_cast<uint>(palette.size()));
    break;
  }
  case ColorMappingMode::TopFolder: {
    QString key = topFolderKey(node);
    if (key.isEmpty()) {
      key = node->name;
    }
    index = static_cast<int>(qHash(key) % static_cast<uint>(palette.size()));
    break;
  }
  case ColorMappingMode::Level: {
    // Similar to GrandPerspective's level mapping: clamp to last color.
    index = std::min(depth, static_cast<int>(palette.size()) - 1);
    break;
  }
  case ColorMappingMode::Nothing:
  default:
    index = 0;
    break;
  }

  return palette[index];
}

TreeNode *CanvasWidget::findNode(TreeNode *node, const QPointF &pos) {
  if (!node || !node->rect.contains(pos)) {
    return nullptr;
  }

  for (TreeNode *child : node->children) {
    if (child->rect.contains(pos)) {
      TreeNode *found = findNode(child, pos);
      if (found) {
        return found;
      }
    }
  }

  return node;
}
