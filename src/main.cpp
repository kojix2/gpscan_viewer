#include <QApplication>
#include <QCoreApplication>

#include "ViewerWindow.h"

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);

  QCoreApplication::setApplicationName("gpscan_viewer");
#ifdef GPSCAN_VIEWER_VERSION
  QCoreApplication::setApplicationVersion(
      QStringLiteral(GPSCAN_VIEWER_VERSION));
#endif

  ViewerWindow window;
  window.resize(1200, 800);
  window.show();

  return app.exec();
}
