#pragma once

#include <QString>
#include <QStringList>
#include <QVector>
#include <QColor>

namespace palettes {

// Default palette name used when none is specified.
QString defaultPaletteName();

// Canonicalize a palette name. Returns defaultPaletteName() for empty/unknown values.
QString canonicalNameOrDefault(const QString &name);

// Return a list of built-in palette names (canonical).
QStringList builtInPaletteNames();

// Get colors for the given palette name. Name is canonicalized internally.
QVector<QColor> paletteForName(const QString &name);

} // namespace palettes
