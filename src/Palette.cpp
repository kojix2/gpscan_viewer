#include "Palette.h"

#include <algorithm>

namespace palettes {

namespace {

constexpr int kRainbowColorCount = 12;

const QString kCoffeeBeans = QStringLiteral("CoffeeBeans");
const QString kRainbow = QStringLiteral("Rainbow");

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
    return kCoffeeBeans;
  }

  if (trimmed.compare(kCoffeeBeans, Qt::CaseInsensitive) == 0) {
    return kCoffeeBeans;
  }
  if (trimmed.compare(kRainbow, Qt::CaseInsensitive) == 0) {
    return kRainbow;
  }

  return kCoffeeBeans;
}

QString defaultPaletteName() { return kCoffeeBeans; }

QStringList builtInPaletteNames() { return {kCoffeeBeans, kRainbow}; }

QVector<QColor> paletteForName(const QString &name) {
  const QString key = canonicalNameOrDefault(name);
  if (key == kRainbow) {
    return rainbowPalette(kRainbowColorCount);
  }
  return coffeeBeansPalette();
}

} // namespace palettes
