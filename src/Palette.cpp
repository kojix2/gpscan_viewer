#include "Palette.h"

#include <algorithm>

namespace palettes {
namespace {

QVector<QColor> coffeeBeansPalette() {
  return {
      QColor(0x66, 0x66, 0x00), // 666600
      QColor(0x99, 0x33, 0x00), // 993300
      QColor(0xCC, 0x66, 0x66), // CC6666
      QColor(0xCC, 0x66, 0x33), // CC6633
      QColor(0xFF, 0xCC, 0x66), // FFCC66
      QColor(0xCC, 0x99, 0x33), // CC9933
      QColor(0xCC, 0x33, 0x33)  // CC3333
  };
}

QVector<QColor> rainbowPalette(int count) {
  QVector<QColor> colors;
  const int n = std::max(1, count);
  colors.reserve(n);

  for (int i = 0; i < n; ++i) {
    const double hue = static_cast<double>(i) / static_cast<double>(n);
    colors.push_back(QColor::fromHsvF(hue, 0.85, 0.9));
  }

  return colors;
}

} // namespace

QString canonicalNameOrDefault(const QString &name) {
  const QString trimmed = name.trimmed();
  if (trimmed.isEmpty()) {
    return QStringLiteral("CoffeeBeans");
  }

  if (trimmed.compare(QStringLiteral("CoffeeBeans"), Qt::CaseInsensitive) == 0) {
    return QStringLiteral("CoffeeBeans");
  }
  if (trimmed.compare(QStringLiteral("Rainbow"), Qt::CaseInsensitive) == 0) {
    return QStringLiteral("Rainbow");
  }

  return QStringLiteral("CoffeeBeans");
}

QStringList builtInPaletteNames() {
  return {QStringLiteral("CoffeeBeans"), QStringLiteral("Rainbow")};
}

QVector<QColor> paletteForName(const QString &name) {
  const QString key = canonicalNameOrDefault(name);
  if (key == QStringLiteral("Rainbow")) {
    return rainbowPalette(12);
  }
  return coffeeBeansPalette();
}

} // namespace palettes
