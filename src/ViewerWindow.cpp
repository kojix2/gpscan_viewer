#include "ViewerWindow.h"

#include <QAction>
#include <QFileDialog>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>

#include "CanvasWidget.h"
#include "TreeLayout.h"
#include "TreeReader.h"
#include "Utils.h"

ViewerWindow::ViewerWindow(QWidget *parent)
    : QMainWindow(parent), canvas(new CanvasWidget(this)) {
  setWindowTitle("gpscan_viewer");
  setCentralWidget(canvas);

  auto *fileMenu = menuBar()->addMenu(tr("&File"));
  QAction *openAction = fileMenu->addAction(tr("&Open..."));
  openAction->setShortcut(QKeySequence::Open);
  QAction *reloadAction = fileMenu->addAction(tr("&Reload"));
  reloadAction->setShortcut(QKeySequence::Refresh);
  fileMenu->addSeparator();
  QAction *quitAction = fileMenu->addAction(tr("&Quit"));
  quitAction->setShortcut(QKeySequence::Quit);

  auto *viewMenu = menuBar()->addMenu(tr("&View"));
  QAction *recomputeAction = viewMenu->addAction(tr("&Recompute Layout"));
  recomputeAction->setShortcut(Qt::Key_F5);

  auto *helpMenu = menuBar()->addMenu(tr("&Help"));
  QAction *aboutAction = helpMenu->addAction(tr("&About"));

  connect(openAction, &QAction::triggered, this, &ViewerWindow::openFile);
  connect(reloadAction, &QAction::triggered, this, &ViewerWindow::reloadFile);
  connect(quitAction, &QAction::triggered, this, &ViewerWindow::close);
  connect(recomputeAction, &QAction::triggered, this, &ViewerWindow::recomputeLayout);
  connect(aboutAction, &QAction::triggered, this, &ViewerWindow::showAbout);
  connect(canvas, &CanvasWidget::selectedNodeChanged, this, &ViewerWindow::updateSelection);

  statusBar()->showMessage(tr("Ready"));
}

void ViewerWindow::openFile() {
  QString path = QFileDialog::getOpenFileName(
      this,
      tr("Open Scan Data"),
      QString(),
      tr("GrandPerspective Scan Data (*.gpscan *.xml);;All Files (*)"));
  if (path.isEmpty()) {
    return;
  }

  QString error;
  std::shared_ptr<TreeModel> model = TreeReader::readFromFile(path, &error);
  if (!model) {
    showError(error.isEmpty() ? tr("Failed to load file.") : error);
    return;
  }

  setModel(model, path);
}

void ViewerWindow::reloadFile() {
  if (currentPath.isEmpty()) {
    statusBar()->showMessage(tr("No file to reload"));
    return;
  }

  QString error;
  std::shared_ptr<TreeModel> model = TreeReader::readFromFile(currentPath, &error);
  if (!model) {
    showError(error.isEmpty() ? tr("Failed to reload file.") : error);
    return;
  }

  setModel(model, currentPath);
}

void ViewerWindow::recomputeLayout() {
  if (!currentModel || !currentModel->root()) {
    return;
  }

  QRectF bounds(0, 0, canvas->width(), canvas->height());
  TreeLayout::layout(currentModel->root(), bounds);
  canvas->update();
}

void ViewerWindow::showAbout() {
  QMessageBox::about(this,
                     tr("About gpscan_viewer"),
                     tr("Minimal viewer for GrandPerspective scan data (XML/gpscan)."));
}

void ViewerWindow::setModel(std::shared_ptr<TreeModel> model, const QString &sourcePath) {
  currentModel = std::move(model);
  currentPath = sourcePath;

  QRectF bounds(0, 0, canvas->width(), canvas->height());
  TreeLayout::layout(currentModel->root(), bounds);
  canvas->setModel(currentModel);

  statusBar()->showMessage(tr("Loaded: %1").arg(currentPath));
}

void ViewerWindow::updateSelection(TreeNode *node) {
  if (!node) {
    statusBar()->showMessage(tr("No selection"));
    return;
  }

  QString fullPath = Utils::buildFullPath(node);
  QString sizeText = Utils::formatSize(node->size);
  statusBar()->showMessage(tr("%1 | %2").arg(fullPath, sizeText));
}

void ViewerWindow::showError(const QString &message) {
  QMessageBox::critical(this, tr("Error"), message);
}
