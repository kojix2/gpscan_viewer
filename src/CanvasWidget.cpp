#include "CanvasWidget.h"

#include <QClipboard>
#include <QContextMenuEvent>
#include <QDesktopServices>
#include <QFileInfo>
#include <QGuiApplication>
#include <QImage>
#include <QMouseEvent>
#include <QMenu>
#include <QPainter>
#include <QToolTip>
#include <QUrl>

#include <algorithm>
#include <array>
#include <cmath>

#include "TreeLayout.h"
#include "Utils.h"

namespace {

constexpr int kGradientSteps = 256;

std::array<QRgb, kGradientSteps> buildGradientColors(const QColor &base, double colorGradient) {
  std::array<QRgb, kGradientSteps> colors{};

  float hue = 0.0f;
  float saturation = 0.0f;
  float brightness = 0.0f;
  float alpha = 1.0f;
  QColor hsv = base.toHsv();
  hsv.getHsvF(&hue, &saturation, &brightness, &alpha);
  if (hue < 0.0f) {
    hue = 0.0f;
  }

  auto clamp01 = [](double v) { return std::clamp(v, 0.0, 1.0); };

  // Darker colors (0..127)
  for (int j = 0; j < 128; ++j) {
    double adjust = colorGradient * (128.0 - static_cast<double>(j)) / 128.0;
    double b = clamp01(static_cast<double>(brightness) * (1.0 - adjust));
    QColor mod = QColor::fromHsvF(hue, clamp01(saturation), b, clamp01(alpha));
    colors[j] = mod.rgb();
  }

  // Lighter colors (128..255)
  for (int j = 0; j < 128; ++j) {
    double adjust = colorGradient * static_cast<double>(j) / 128.0;
    double dif = 1.0 - static_cast<double>(brightness);
    double absAdjust = (dif + saturation) * adjust;
    double b = brightness;
    double s = saturation;

    if (absAdjust < dif) {
      b = clamp01(static_cast<double>(brightness) + absAdjust);
    } else {
      s = clamp01(saturation + dif - absAdjust);
      b = 1.0;
    }

    QColor mod = QColor::fromHsvF(hue, clamp01(s), clamp01(b), clamp01(alpha));
    colors[128 + j] = mod.rgb();
  }

  return colors;
}

QVector<QColor> defaultPalette() {
  // Original GrandPerspective default palette: CoffeeBeans
  return {
      QColor(0x66, 0x66, 0x00), // 666600
      QColor(0x99, 0x33, 0x00), // 993300
      QColor(0xCC, 0x66, 0x66), // CC6666
      QColor(0xCC, 0x66, 0x33), // CC6633
      QColor(0xFF, 0xCC, 0x66), // FFCC66
      QColor(0xCC, 0x99, 0x33), // CC9933
      QColor(0xCC, 0x33, 0x33)  // CC3333
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

  TreeNode *hit = findNode(model->root(), mapToLayout(event->position()));
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

void CanvasWidget::contextMenuEvent(QContextMenuEvent *event) {
  if (!model || !model->root()) {
    return;
  }

  TreeNode *hit = findNode(model->root(), mapToLayout(event->pos()));
  if (!hit) {
    return;
  }

  if (hit != selectedNode) {
    selectedNode = hit;
    emit selectedNodeChanged(selectedNode);
    update();
  }

  showContextMenu(event->globalPos(), hit);
}

void CanvasWidget::updateTooltip(const QPointF &rawPos) {
  if (!model || !model->root()) {
    QToolTip::hideText();
    return;
  }

  const QPointF layoutPos = mapToLayout(rawPos);
  TreeNode *node = findNode(model->root(), layoutPos);
  if (node && node != hoveredNode) {
    hoveredNode = node;
    QString fullPath = Utils::buildFullPath(node);
    QString sizeText = Utils::formatSize(node->size);
    QString tip = QString("%1\n%2").arg(fullPath, sizeText);
    QToolTip::showText(mapToGlobal(rawPos.toPoint()), tip, this);
  } else if (!node) {
    hoveredNode = nullptr;
    QToolTip::hideText();
  }
}

void CanvasWidget::showContextMenu(const QPoint &globalPos, TreeNode *node) {
  if (!node) {
    return;
  }

  const QString fullPath = Utils::buildFullPath(node);
  if (fullPath.isEmpty()) {
    return;
  }

  QFileInfo info(fullPath);
  const QString revealPath = info.isDir() ? info.absoluteFilePath() : info.absolutePath();

  QMenu menu(this);

  QAction *openAction = menu.addAction(tr("Open"));
  QAction *revealAction = menu.addAction(tr("Reveal"));
  QAction *copyPathAction = menu.addAction(tr("Copy Path"));

  openAction->setEnabled(!fullPath.isEmpty());
  revealAction->setEnabled(!revealPath.isEmpty());
  copyPathAction->setEnabled(!fullPath.isEmpty());

  connect(openAction, &QAction::triggered, this, [fullPath]() {
    QDesktopServices::openUrl(QUrl::fromLocalFile(fullPath));
  });
  connect(revealAction, &QAction::triggered, this, [revealPath]() {
    QDesktopServices::openUrl(QUrl::fromLocalFile(revealPath));
  });
  connect(copyPathAction, &QAction::triggered, this, [fullPath]() {
    if (auto *clipboard = QGuiApplication::clipboard()) {
      clipboard->setText(fullPath);
    }
  });

  menu.exec(globalPos);
}

QPointF CanvasWidget::mapToLayout(const QPointF &pos) const {
  return QPointF(pos.x(), height() - pos.y());
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

  // GrandPerspective original algorithm: two triangles filled by horizontal and vertical
  // gradient lines, using a gradient palette derived from the base color.
  constexpr double kDefaultColorGradient = 0.5;
  const auto gradientColors = buildGradientColors(base, kDefaultColorGradient);

  QRgb *data = reinterpret_cast<QRgb *>(image.bits());
  const int stride = image.bytesPerLine() / static_cast<int>(sizeof(QRgb));

  auto setPixel = [&](int x, int y, QRgb color) {
    if (x < 0 || x >= imgWidth || y < 0 || y >= imgHeight) {
      return;
    }
    data[y * stride + x] = color;
  };

  // Horizontal lines: upper-left triangle
  for (int y = 0; y < rectHeight; ++y) {
    double gradient = 256.0 * (y0 + y + 0.5 - rect.y()) / rect.height();
    int gradientIndex = std::clamp(static_cast<int>(std::lround(gradient)), 0, 255);
    QRgb color = gradientColors[gradientIndex];

    int maxX = (rectHeight - y - 1) * rectWidth / rectHeight;
    int yWrite = imgHeight - y0 - y - 1; // Match original bitmap's flipped Y-axis
    for (int x = 0; x < maxX; ++x) {
      setPixel(x0 + x, yWrite, color);
    }
  }

  // Vertical lines: lower-right triangle
  for (int x = 0; x < rectWidth; ++x) {
    double gradient = 256.0 * (1.0 - (x0 + x + 0.5 - rect.x()) / rect.width());
    int gradientIndex = std::clamp(static_cast<int>(std::lround(gradient)), 0, 255);
    QRgb color = gradientColors[gradientIndex];

    int minY = (rectWidth - x - 1) * rectHeight / rectWidth;
    int startY = imgHeight - y0 - rectHeight;
    int rows = rectHeight - minY;
    for (int y = 0; y < rows; ++y) {
      setPixel(x0 + x, startY + y, color);
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
  QRectF rect = node->rect;
  rect.moveTop(height() - rect.y() - rect.height());
  painter.drawRect(rect.adjusted(1, 1, -1, -1));
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
