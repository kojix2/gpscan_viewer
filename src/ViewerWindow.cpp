#include "ViewerWindow.h"

#include <QAction>
#include <QApplication>
#include <QComboBox>
#include <QFileDialog>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>
#include <QToolBar>

#include "CanvasWidget.h"
#include "TreeLayout.h"
#include "TreeReader.h"
#include "Utils.h"

ViewerWindow::ViewerWindow(QWidget *parent)
    : QMainWindow(parent), canvas(new CanvasWidget(this)) {
  setWindowTitle("gpscan_viewer");
  setCentralWidget(canvas);

  // Create toolbar
  toolBar = addToolBar(tr("Main Toolbar"));
  toolBar->setMovable(false);
  toolBar->setIconSize(QSize(24, 24));

  // Open action with icon
  QAction *openAction = new QAction(style()->standardIcon(QStyle::SP_DialogOpenButton), tr("Open"), this);
  openAction->setShortcut(QKeySequence::Open);
  openAction->setToolTip(tr("Open scan data file"));
  toolBar->addAction(openAction);

  // Reload action with icon
  QAction *reloadAction = new QAction(style()->standardIcon(QStyle::SP_BrowserReload), tr("Reload"), this);
  reloadAction->setShortcut(QKeySequence::Refresh);
  reloadAction->setToolTip(tr("Reload current file"));
  toolBar->addAction(reloadAction);

  toolBar->addSeparator();

  // Color mapping selector
  toolBar->addWidget(new QLabel(tr("Color by: "), this));
  colorMappingCombo = new QComboBox(this);
  colorMappingCombo->addItem(tr("Extension"));
  colorMappingCombo->addItem(tr("Name"));
  colorMappingCombo->addItem(tr("Folder"));
  colorMappingCombo->addItem(tr("Top Folder"));
  colorMappingCombo->addItem(tr("Level"));
  colorMappingCombo->addItem(tr("Nothing"));
  colorMappingCombo->setCurrentIndex(0); // Default: Extension
  colorMappingCombo->setToolTip(tr("Color mapping scheme"));
  toolBar->addWidget(colorMappingCombo);

  toolBar->addSeparator();

  // Recompute layout action
  QAction *recomputeAction = new QAction(style()->standardIcon(QStyle::SP_BrowserReload), tr("Recompute"), this);
  recomputeAction->setShortcut(Qt::Key_F5);
  recomputeAction->setToolTip(tr("Recompute layout"));
  toolBar->addAction(recomputeAction);

  // Menus
  auto *fileMenu = menuBar()->addMenu(tr("&File"));
  fileMenu->addAction(openAction);
  fileMenu->addAction(reloadAction);
  fileMenu->addSeparator();
  QAction *quitAction = fileMenu->addAction(tr("&Quit"));
  quitAction->setShortcut(QKeySequence::Quit);

  auto *viewMenu = menuBar()->addMenu(tr("&View"));
  viewMenu->addAction(recomputeAction);

  auto *helpMenu = menuBar()->addMenu(tr("&Help"));
  QAction *aboutAction = helpMenu->addAction(tr("&About"));

  // Connections
  connect(openAction, &QAction::triggered, this, &ViewerWindow::openFile);
  connect(reloadAction, &QAction::triggered, this, &ViewerWindow::reloadFile);
  connect(quitAction, &QAction::triggered, this, &ViewerWindow::close);
  connect(recomputeAction, &QAction::triggered, this, &ViewerWindow::recomputeLayout);
  connect(aboutAction, &QAction::triggered, this, &ViewerWindow::showAbout);
  connect(canvas, &CanvasWidget::selectedNodeChanged, this, &ViewerWindow::updateSelection);
  connect(colorMappingCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, &ViewerWindow::changeColorMapping);

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

  QApplication::setOverrideCursor(Qt::WaitCursor);

  QString error;
  std::shared_ptr<TreeModel> model = TreeReader::readFromFile(path, &error);
  
  QApplication::restoreOverrideCursor();
  
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

  QApplication::setOverrideCursor(Qt::WaitCursor);

  QString error;
  std::shared_ptr<TreeModel> model = TreeReader::readFromFile(currentPath, &error);
  
  QApplication::restoreOverrideCursor();
  
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

void ViewerWindow::changeColorMapping(int index) {
  CanvasWidget::ColorMappingMode mode = static_cast<CanvasWidget::ColorMappingMode>(index);
  canvas->setColorMappingMode(mode);
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
