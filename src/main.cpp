#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QFileInfo>
#include <QString>

#include "ViewerWindow.h"

int main(int argc, char *argv[]) {
  auto setupParser = []() {
    QCommandLineParser parser;
    parser.setApplicationDescription(
        QObject::tr("Yet another viewer for GrandPerspective scan data."));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument(QObject::tr("file"),
                                 QObject::tr("Path to .gpscan or .xml file."));
    return parser;
  };

  bool wantsHelp = false;
  bool wantsVersion = false;
  for (int i = 1; i < argc; ++i) {
    const QString arg = QString::fromLocal8Bit(argv[i]);
    if (arg == QStringLiteral("-h") || arg == QStringLiteral("--help")) {
      wantsHelp = true;
    } else if (arg == QStringLiteral("-v") ||
               arg == QStringLiteral("--version")) {
      wantsVersion = true;
    }
  }

  if (wantsHelp || wantsVersion) {
    QCoreApplication app(argc, argv);

    QCoreApplication::setApplicationName("gpscan_viewer");
#ifdef GPSCAN_VIEWER_VERSION
    QCoreApplication::setApplicationVersion(
        QStringLiteral(GPSCAN_VIEWER_VERSION));
#endif

    QCommandLineParser parser = setupParser();
    parser.process(app);

    if (wantsHelp) {
      parser.showHelp();
    }
    if (wantsVersion) {
      parser.showVersion();
    }
    return 0;
  }

  QApplication app(argc, argv);

  QCoreApplication::setApplicationName("gpscan_viewer");
#ifdef GPSCAN_VIEWER_VERSION
  QCoreApplication::setApplicationVersion(
      QStringLiteral(GPSCAN_VIEWER_VERSION));
#endif

  QCommandLineParser parser = setupParser();
  parser.process(app);

  ViewerWindow window;
  window.resize(1200, 800);
  window.show();

  const QStringList positionalArgs = parser.positionalArguments();
  if (!positionalArgs.isEmpty()) {
    window.openFilePath(QFileInfo(positionalArgs.first()).absoluteFilePath());
  }

  return app.exec();
}
