#include "TreeReader.h"

#include <QDir>
#include <QFile>
#include <QXmlStreamReader>

#include <zlib.h>

static bool isTreeElement(const QString &name) {
  return name == QLatin1String("Folder") || name == QLatin1String("File");
}

static bool isGzipData(const QByteArray &data) {
  return data.size() >= 2 && static_cast<unsigned char>(data[0]) == 0x1f &&
         static_cast<unsigned char>(data[1]) == 0x8b;
}

static QByteArray inflateGzip(const QByteArray &compressed, QString *errorOut) {
  if (compressed.isEmpty()) {
    return QByteArray();
  }

  z_stream stream;
  std::memset(&stream, 0, sizeof(stream));

  int initResult = inflateInit2(&stream, 15 + 16);
  if (initResult != Z_OK) {
    if (errorOut) {
      *errorOut = QObject::tr("Failed to initialize gzip decoder.");
    }
    return QByteArray();
  }

  stream.next_in =
      reinterpret_cast<Bytef *>(const_cast<char *>(compressed.data()));
  stream.avail_in = static_cast<uInt>(compressed.size());

  QByteArray output;
  const int chunkSize = 64 * 1024;
  int inflateResult = Z_OK;

  while (inflateResult == Z_OK) {
    int oldSize = output.size();
    output.resize(oldSize + chunkSize);
    stream.next_out = reinterpret_cast<Bytef *>(output.data() + oldSize);
    stream.avail_out = static_cast<uInt>(chunkSize);

    inflateResult = inflate(&stream, Z_NO_FLUSH);
    if (inflateResult != Z_OK && inflateResult != Z_STREAM_END) {
      if (errorOut) {
        *errorOut = QObject::tr("Failed to decompress gzip data (code %1).")
                        .arg(inflateResult);
      }
      inflateEnd(&stream);
      return QByteArray();
    }

    int produced = chunkSize - static_cast<int>(stream.avail_out);
    output.resize(oldSize + produced);
  }

  inflateEnd(&stream);
  return output;
}

std::shared_ptr<TreeModel> TreeReader::readFromFile(const QString &path,
                                                    QString *errorOut) {
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly)) {
    if (errorOut) {
      *errorOut = QObject::tr("Failed to open file.");
    }
    return nullptr;
  }

  const QByteArray rawData = file.readAll();
  if (rawData.isEmpty()) {
    if (errorOut) {
      *errorOut = QObject::tr("File is empty.");
    }
    return nullptr;
  }

  QByteArray xmlData;
  QString decompressError;
  if (isGzipData(rawData)) {
    xmlData = inflateGzip(rawData, &decompressError);
    if (xmlData.isEmpty()) {
      if (errorOut) {
        *errorOut = decompressError.isEmpty()
                        ? QObject::tr("Failed to decompress file.")
                        : decompressError;
      }
      return nullptr;
    }
  } else {
    xmlData = rawData;
  }

  QXmlStreamReader xml(xmlData);
  auto model = std::make_shared<TreeModel>();
  QVector<TreeNode *> stack;
  QString volumePath;

  while (!xml.atEnd()) {
    xml.readNext();

    if (xml.isStartElement()) {
      const QString elementName = xml.name().toString();
      if (elementName == QLatin1String("ScanInfo")) {
        const QXmlStreamAttributes attrs = xml.attributes();
        volumePath = attrs.value(QLatin1String("volumePath")).toString();
        volumePath = QDir::cleanPath(volumePath);
      }
      if (isTreeElement(elementName)) {
        const bool isDir = (elementName == QLatin1String("Folder"));
        const QXmlStreamAttributes attrs = xml.attributes();

        TreeNode *node = new TreeNode();
        node->name = attrs.value(QLatin1String("name")).toString();
        node->size = attrs.value(QLatin1String("size")).toULongLong();
        node->isDir = isDir;

        if (!stack.isEmpty()) {
          node->parent = stack.last();
          stack.last()->children.push_back(node);
        } else {
          if (!volumePath.isEmpty()) {
            const QString rootName = node->name.trimmed();
            if (rootName.isEmpty() || rootName == QLatin1String("/")) {
              node->name = volumePath;
            } else if (!QDir::isAbsolutePath(rootName)) {
              node->name = QDir(volumePath).filePath(rootName);
            }
          }
          model->setRoot(node);
        }

        stack.push_back(node);
      }
    } else if (xml.isEndElement()) {
      if (isTreeElement(xml.name().toString())) {
        if (!stack.isEmpty()) {
          stack.removeLast();
        }
      }
    }
  }

  if (xml.hasError()) {
    if (errorOut) {
      *errorOut = QObject::tr("XML parse error: %1").arg(xml.errorString());
    }
    return nullptr;
  }

  if (!model->root()) {
    if (errorOut) {
      *errorOut = QObject::tr("No root node found in XML.");
    }
    return nullptr;
  }

  model->computeDerivedSizes();
  return model;
}
