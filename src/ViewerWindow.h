#pragma once

#include <QMainWindow>
#include <memory>

#include "TreeModel.h"

class CanvasWidget;

class ViewerWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit ViewerWindow(QWidget *parent = nullptr);

private slots:
  void openFile();
  void reloadFile();
  void recomputeLayout();
  void showAbout();
  void updateSelection(TreeNode *node);

private:
  void setModel(std::shared_ptr<TreeModel> model, const QString &sourcePath);
  void showError(const QString &message);

  CanvasWidget *canvas = nullptr;
  std::shared_ptr<TreeModel> currentModel;
  QString currentPath;
};
