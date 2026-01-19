#include <QApplication>

#include "ViewerWindow.h"

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);

  ViewerWindow window;
  window.resize(1200, 800);
  window.show();

  return app.exec();
}
