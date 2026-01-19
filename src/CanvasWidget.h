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

private:
  QColor colorForNode(const TreeNode *node, int depth) const;
  
  // Build a 256-level gradient color table (GrandPerspective style)
  void buildGradientTable(const QColor &base, QVector<QRgb> &table) const;
  
  // Draw rectangle with diagonal gradient (GrandPerspective style)
  void drawGradientRect(QImage &image, const QRectF &rect, const QVector<QRgb> &gradientTable);
  
  void drawNode(QImage &image, TreeNode *node, int depth);
  void drawSelection(QPainter &painter, TreeNode *node);
  TreeNode *findNode(TreeNode *node, const QPointF &pos);
  void updateTooltip(const QPointF &pos);

  std::shared_ptr<TreeModel> model;
  TreeNode *selectedNode = nullptr;
  TreeNode *hoveredNode = nullptr;
  QVector<QColor> palette;
  double gradientStrength = 0.5;
  ColorMappingMode colorMappingMode = ColorMappingMode::Extension;
};
