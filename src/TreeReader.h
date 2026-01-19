#pragma once

#include <memory>

#include "TreeModel.h"

class TreeReader {
public:
  static std::shared_ptr<TreeModel> readFromFile(const QString &path, QString *errorOut);
};
