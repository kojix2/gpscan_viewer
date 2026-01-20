#pragma once

#include <QWidget>
#include <memory>

#include "TreeModel.h"

class CanvasWidget : public QWidget {
  Q_OBJECT

public:
  enum class ColorMappingMode {
    Extension,
    Name,
    Folder,
    TopFolder,
    Level,
    Nothing,
  };

  explicit CanvasWidget(QWidget *parent = nullptr);

  void setModel(std::shared_ptr<TreeModel> model);
  void setColorMappingMode(ColorMappingMode mode);

signals:
  void selectedNodeChanged(TreeNode *node);

protected:
  void paintEvent(QPaintEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;
  bool event(QEvent *event) override;
  void contextMenuEvent(QContextMenuEvent *event) override;

private:
  QColor colorForNode(const TreeNode *node, int depth) const;

  // Draw rectangle with GrandPerspective-style two-triangle gradient
  void drawBevelRect(QImage &image, const QRectF &rect, const QColor &base);
  
  void drawNode(QImage &image, TreeNode *node, int depth);
  void drawSelection(QPainter &painter, TreeNode *node);
  TreeNode *findNode(TreeNode *node, const QPointF &pos);
  void updateTooltip(const QPointF &rawPos);
  void showContextMenu(const QPoint &globalPos, TreeNode *node);
  QPointF mapToLayout(const QPointF &pos) const;

  std::shared_ptr<TreeModel> model;
  TreeNode *selectedNode = nullptr;
  TreeNode *hoveredNode = nullptr;
  QVector<QColor> palette;
  ColorMappingMode colorMappingMode = ColorMappingMode::Extension;
};
