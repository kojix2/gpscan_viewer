#include "ViewerWindow.h"

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QComboBox>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QIcon>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QSettings>
#include <QStatusBar>
#include <QToolBar>

#include "CanvasWidget.h"
#include "Palette.h"
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
  QAction *openAction = new QAction(
      style()->standardIcon(QStyle::SP_DialogOpenButton), tr("Open"), this);
  openAction->setShortcut(QKeySequence::Open);
  openAction->setToolTip(tr("Open scan data file"));
  toolBar->addAction(openAction);

  // Reload action with icon
  QAction *reloadAction = new QAction(
      style()->standardIcon(QStyle::SP_BrowserReload), tr("Reload"), this);
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

  // Menus
  auto *fileMenu = menuBar()->addMenu(tr("&File"));
  fileMenu->addAction(openAction);
  fileMenu->addAction(reloadAction);
  fileMenu->addSeparator();
  QAction *quitAction = fileMenu->addAction(tr("&Quit"));
  quitAction->setShortcut(QKeySequence::Quit);

  auto *paletteMenu = menuBar()->addMenu(tr("&Palette"));
  auto *paletteGroup = new QActionGroup(this);
  paletteGroup->setExclusive(true);

  auto *settings = new QSettings("GrandPerspective", "gpscan_viewer", this);
  const QString initialPalette = palettes::canonicalNameOrDefault(
      settings->value("paletteName", palettes::defaultPaletteName())
          .toString());

  for (const QString &name : palettes::builtInPaletteNames()) {
    QAction *action = paletteMenu->addAction(name);
    action->setCheckable(true);
    action->setActionGroup(paletteGroup);
    action->setData(name);

    connect(action, &QAction::triggered, this, [this, action, settings]() {
      const QString chosen =
          palettes::canonicalNameOrDefault(action->data().toString());
      canvas->setPaletteName(chosen);
      settings->setValue("paletteName", canvas->paletteName());
    });

    if (name == initialPalette) {
      action->setChecked(true);
    }
  }

  auto *helpMenu = menuBar()->addMenu(tr("&Help"));
  QAction *aboutAction = helpMenu->addAction(tr("&About"));

  // Connections
  connect(openAction, &QAction::triggered, this, &ViewerWindow::openFile);
  connect(reloadAction, &QAction::triggered, this, &ViewerWindow::reloadFile);
  connect(quitAction, &QAction::triggered, this, &ViewerWindow::close);
  connect(aboutAction, &QAction::triggered, this, &ViewerWindow::showAbout);
  connect(canvas, &CanvasWidget::selectedNodeChanged, this,
          &ViewerWindow::updateSelection);
  connect(canvas, &CanvasWidget::requestDeletePath, this,
          &ViewerWindow::deletePath);
  connect(colorMappingCombo,
          QOverload<int>::of(&QComboBox::currentIndexChanged), this,
          &ViewerWindow::changeColorMapping);

  canvas->setPaletteName(initialPalette);

  statusBar()->showMessage(tr("Ready"));
}

void ViewerWindow::openFile() {
  QString path = QFileDialog::getOpenFileName(
      this, tr("Open Scan Data"), QString(),
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
  std::shared_ptr<TreeModel> model =
      TreeReader::readFromFile(currentPath, &error);

  QApplication::restoreOverrideCursor();

  if (!model) {
    showError(error.isEmpty() ? tr("Failed to reload file.") : error);
    return;
  }

  setModel(model, currentPath);
}

void ViewerWindow::showAbout() {
  QMessageBox box(this);
  box.setWindowTitle(tr("About gpscan_viewer"));
  const QString version = QCoreApplication::applicationVersion().isEmpty()
                              ? tr("(unknown)")
                              : QCoreApplication::applicationVersion();
#ifdef GPSCAN_VIEWER_REPO_URL
  const QString repoUrl = QStringLiteral(GPSCAN_VIEWER_REPO_URL);
#else
  const QString repoUrl =
      QStringLiteral("https://github.com/kojix2/gpscan_viewer");
#endif

  const QString grandPerspectiveUrl =
      QStringLiteral("https://grandperspectiv.sourceforge.net/");
  const QString qtUrl = QStringLiteral("https://www.qt.io/");
  const QString aboutText =
      tr("Yet another viewer for GrandPerspective scan data.") +
      QStringLiteral("<br>") + tr("Built with Qt.") +
      QStringLiteral("<br><br>") + tr("Version: %1").arg(version) +
      QStringLiteral("<br>") +
      tr("Repository: <a href=\"%1\">%1</a>").arg(repoUrl) +
      QStringLiteral("<br>") + tr("License: GPL-2.0-or-later") +
      QStringLiteral("<br><br>") +
      tr("Acknowledgements: Based on the macOS app GrandPerspective by Erwin "
         "Bonsma.") +
      QStringLiteral("<br>") +
      tr("<a href=\"%1\">%1</a>").arg(grandPerspectiveUrl) +
      QStringLiteral("<br>") + tr("License: GPL-2.0-or-later");
  box.setTextFormat(Qt::RichText);
  box.setText(aboutText);
  box.setTextInteractionFlags(Qt::TextBrowserInteraction);
  if (auto *label =
          box.findChild<QLabel *>(QStringLiteral("qt_msgbox_label"))) {
    label->setOpenExternalLinks(true);
  }

  QIcon appIcon = QIcon::fromTheme(QStringLiteral("gpscan_viewer"));
  if (appIcon.isNull()) {
    const QString appDir = QCoreApplication::applicationDirPath();
    const QStringList iconCandidates = {
        appDir + QStringLiteral("/../resources/gpscan_viewer.svg"),
        appDir + QStringLiteral(
                     "/../share/icons/hicolor/scalable/apps/gpscan_viewer.svg"),
        QStringLiteral(
            "/usr/share/icons/hicolor/scalable/apps/gpscan_viewer.svg")};
    for (const QString &path : iconCandidates) {
      if (QFileInfo::exists(path)) {
        appIcon = QIcon(path);
        break;
      }
    }
  }

  if (!appIcon.isNull()) {
    box.setIconPixmap(appIcon.pixmap(128, 128));
  }

  box.exec();
}

void ViewerWindow::changeColorMapping(int index) {
  CanvasWidget::ColorMappingMode mode =
      static_cast<CanvasWidget::ColorMappingMode>(index);
  canvas->setColorMappingMode(mode);
}

void ViewerWindow::setModel(std::shared_ptr<TreeModel> model,
                            const QString &sourcePath) {
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

void ViewerWindow::deletePath(const QString &path) {
  const QString cleaned = QDir::cleanPath(path);
  if (cleaned.isEmpty()) {
    showError(tr("Nothing to delete."));
    return;
  }

  if (!QDir::isAbsolutePath(cleaned)) {
    showError(tr("Refusing to delete a relative path: %1").arg(cleaned));
    return;
  }

  if (cleaned == QDir::rootPath()) {
    showError(tr("Refusing to delete the root directory."));
    return;
  }

  QFileInfo info(cleaned);
  const bool isSymLink = info.isSymLink();
  const bool exists = info.exists() || isSymLink;
  if (!exists) {
    showError(tr("Path not found: %1").arg(cleaned));
    return;
  }

  QString prompt;
  if (isSymLink) {
    prompt = tr("Delete symlink \"%1\"?").arg(cleaned);
  } else if (info.isDir()) {
    prompt = tr("Delete folder \"%1\" and its contents?").arg(cleaned);
  } else {
    prompt = tr("Delete file \"%1\"?").arg(cleaned);
  }

  QMessageBox::StandardButton reply = QMessageBox::warning(
      this, tr("Delete"), prompt, QMessageBox::Yes | QMessageBox::Cancel,
      QMessageBox::Cancel);

  if (reply != QMessageBox::Yes) {
    return;
  }

  bool ok = false;
  if (isSymLink || info.isFile()) {
    ok = QFile::remove(cleaned);
  } else if (info.isDir()) {
    QDir dir(cleaned);
    ok = dir.removeRecursively();
  }

  if (!ok) {
    showError(tr("Failed to delete: %1").arg(cleaned));
    return;
  }

  statusBar()->showMessage(tr("Deleted: %1").arg(cleaned));
}
