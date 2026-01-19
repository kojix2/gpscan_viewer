#include <QCoreApplication>
#include <QFile>
#include <QTemporaryDir>

#include <cstring>
#include <iostream>

#include <zlib.h>

#include "TreeLayout.h"
#include "TreeModel.h"
#include "TreeReader.h"
#include "Utils.h"

namespace {

bool expectTrue(bool condition, const char *message) {
  if (!condition) {
    std::cerr << "[FAIL] " << message << "\n";
    return false;
  }
  return true;
}

QByteArray sampleXml() {
  return QByteArray(
      "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
      "<GrandPerspectiveScanDump appVersion=\"3.6.2\" formatVersion=\"7\">\n"
      "  <ScanInfo volumePath=\"/\" volumeSize=\"100\" freeSpace=\"0\" "
      "scanTime=\"2026-01-20 00:00:00 +0000\" fileSizeMeasure=\"logical\">\n"
      "    <Folder name=\"/\">\n"
      "      <File name=\"fileA\" size=\"60\" />\n"
      "      <File name=\"fileB\" size=\"40\" />\n"
      "    </Folder>\n"
      "  </ScanInfo>\n"
      "</GrandPerspectiveScanDump>\n");
}

QString writeTempFile(const QTemporaryDir &dir, const QString &name, const QByteArray &data) {
  QString path = dir.path() + "/" + name;
  QFile file(path);
  if (!file.open(QIODevice::WriteOnly)) {
    return QString();
  }
  file.write(data);
  file.close();
  return path;
}

QByteArray gzipCompress(const QByteArray &data) {
  z_stream stream;
  std::memset(&stream, 0, sizeof(stream));

  if (deflateInit2(&stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 15 + 16, 8,
                   Z_DEFAULT_STRATEGY) != Z_OK) {
    return QByteArray();
  }

  QByteArray output;
  const int chunkSize = 64 * 1024;
  stream.next_in = reinterpret_cast<Bytef *>(const_cast<char *>(data.data()));
  stream.avail_in = static_cast<uInt>(data.size());

  int deflateResult = Z_OK;
  while (deflateResult != Z_STREAM_END) {
    int oldSize = output.size();
    output.resize(oldSize + chunkSize);
    stream.next_out = reinterpret_cast<Bytef *>(output.data() + oldSize);
    stream.avail_out = static_cast<uInt>(chunkSize);

    deflateResult = deflate(&stream, Z_FINISH);
    if (deflateResult != Z_OK && deflateResult != Z_STREAM_END) {
      deflateEnd(&stream);
      return QByteArray();
    }

    int produced = chunkSize - static_cast<int>(stream.avail_out);
    output.resize(oldSize + produced);
  }

  deflateEnd(&stream);
  return output;
}

bool testTreeLayout() {
  auto *root = new TreeNode();
  root->name = "/";
  root->isDir = true;

  auto *childA = new TreeNode();
  childA->name = "A";
  childA->size = 60;
  childA->isDir = false;
  childA->parent = root;

  auto *childB = new TreeNode();
  childB->name = "B";
  childB->size = 40;
  childB->isDir = false;
  childB->parent = root;

  root->children = {childA, childB};
  TreeLayout::layout(root, QRectF(0, 0, 100, 100));

  bool ok = true;
  ok &= expectTrue(childA->rect.width() > 0.0, "childA width > 0");
  ok &= expectTrue(childA->rect.height() > 0.0, "childA height > 0");
  ok &= expectTrue(childB->rect.width() > 0.0, "childB width > 0");
  ok &= expectTrue(childB->rect.height() > 0.0, "childB height > 0");
  ok &= expectTrue(!childA->rect.intersects(childB->rect), "children do not overlap");
  ok &= expectTrue(root->rect.contains(childA->rect), "root contains childA");
  ok &= expectTrue(root->rect.contains(childB->rect), "root contains childB");

  TreeModel::deleteSubtree(root);
  return ok;
}

bool testTreeReaderXml() {
  QTemporaryDir dir;
  if (!dir.isValid()) {
    return expectTrue(false, "temporary directory valid");
  }

  QString path = writeTempFile(dir, "sample.xml", sampleXml());
  if (path.isEmpty()) {
    return expectTrue(false, "write sample xml");
  }

  QString error;
  auto model = TreeReader::readFromFile(path, &error);
  bool ok = expectTrue(model != nullptr, "parse xml") &&
            expectTrue(model->root() != nullptr, "root exists") &&
            expectTrue(model->root()->children.size() == 2, "child count == 2");
  return ok;
}

bool testTreeReaderGzip() {
  QTemporaryDir dir;
  if (!dir.isValid()) {
    return expectTrue(false, "temporary directory valid");
  }

  QByteArray compressed = gzipCompress(sampleXml());
  if (compressed.isEmpty()) {
    return expectTrue(false, "gzip compress sample xml");
  }

  QString path = writeTempFile(dir, "sample.gpscan", compressed);
  if (path.isEmpty()) {
    return expectTrue(false, "write sample gpscan");
  }

  QString error;
  auto model = TreeReader::readFromFile(path, &error);
  bool ok = expectTrue(model != nullptr, "parse gpscan") &&
            expectTrue(model->root() != nullptr, "root exists") &&
            expectTrue(model->root()->children.size() == 2, "child count == 2");
  return ok;
}

bool testFormatSize() {
  bool ok = true;
  ok &= expectTrue(Utils::formatSize(0) == "0 B", "formatSize(0)");
  ok &= expectTrue(Utils::formatSize(512) == "512 B", "formatSize(512)");
  ok &= expectTrue(Utils::formatSize(1024) == "1.00 KB", "formatSize(1024)");
  ok &= expectTrue(Utils::formatSize(1536) == "1.50 KB", "formatSize(1536)");
  ok &= expectTrue(Utils::formatSize(1048576) == "1.00 MB", "formatSize(1MB)");
  ok &= expectTrue(Utils::formatSize(1073741824) == "1.00 GB", "formatSize(1GB)");
  return ok;
}

bool testBuildFullPath() {
  auto *root = new TreeNode();
  root->name = "/";
  root->isDir = true;

  auto *home = new TreeNode();
  home->name = "home";
  home->isDir = true;
  home->parent = root;
  root->children = {home};

  auto *file = new TreeNode();
  file->name = "test.txt";
  file->isDir = false;
  file->parent = home;
  home->children = {file};

  bool ok = true;
  ok &= expectTrue(Utils::buildFullPath(root) == "/", "buildFullPath(root)");
  ok &= expectTrue(Utils::buildFullPath(home) == "/home", "buildFullPath(home)");
  ok &= expectTrue(Utils::buildFullPath(file) == "/home/test.txt", "buildFullPath(file)");

  TreeModel::deleteSubtree(root);
  return ok;
}

} // namespace

int main(int argc, char **argv) {
  QCoreApplication app(argc, argv);

  bool ok = true;
  ok &= testTreeLayout();
  ok &= testTreeReaderXml();
  ok &= testTreeReaderGzip();
  ok &= testFormatSize();
  ok &= testBuildFullPath();

  if (!ok) {
    std::cerr << "One or more tests failed.\n";
    return 1;
  }

  std::cout << "All tests passed.\n";
  return 0;
}
