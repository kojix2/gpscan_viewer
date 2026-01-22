#pragma once

#include <QMainWindow>
#include <memory>

#include "TreeModel.h"

class CanvasWidget;
class QToolBar;
class QComboBox;

class ViewerWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit ViewerWindow(QWidget *parent = nullptr);

private slots:
  void openFile();
  void reloadFile();
  void showAbout();
  void updateSelection(TreeNode *node);
  void changeColorMapping(int index);

private:
  void setModel(std::shared_ptr<TreeModel> model, const QString &sourcePath);
  void showError(const QString &message);

  CanvasWidget *canvas = nullptr;
  QToolBar *toolBar = nullptr;
  QComboBox *colorMappingCombo = nullptr;
  std::shared_ptr<TreeModel> currentModel;
  QString currentPath;
};
