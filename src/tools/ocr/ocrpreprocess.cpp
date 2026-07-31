// SPDX-License-Identifier: GPL-3.0-or-later

#include "ocrpreprocess.h"

#include <algorithm>

namespace {

bool hasDarkBackground(const QImage& image)
{
    // Mean luminance of a small resample is enough to classify; exact
    // statistics are not needed
    const QImage sample =
      image.scaled(32, 32, Qt::IgnoreAspectRatio, Qt::FastTransformation)
        .convertToFormat(QImage::Format_Grayscale8);
    const uchar* bits = sample.constBits();
    const qsizetype size = sample.sizeInBytes();
    if (size <= 0) {
        return false;
    }
    qint64 sum = 0;
    for (qsizetype i = 0; i < size; ++i) {
        sum += bits[i];
    }
    return (sum / size) < 100;
}

} // namespace

QImage ocrPreprocessImage(const QImage& image,
                          int maxDimension,
                          qreal scaleOverride)
{
    if (image.isNull()) {
        return image;
    }

    QImage prepared = image.convertToFormat(QImage::Format_RGBA8888);

    // The engine is trained mostly on dark-on-light text; terminals and
    // dark themes recognize much better inverted. A second preprocessing
    // pass cannot double-invert: the first pass made the image light.
    if (hasDarkBackground(prepared)) {
        prepared.invertPixels();
    }

    qreal scale = scaleOverride;
    if (scale <= 0.0) {
        // Glyph height is unknown before recognition, so the selection size
        // is used as a proxy: small captures are usually tight crops around
        // text, where upscaling lifts glyphs towards the ~30 px the engine
        // wants. (ocrRefinementScale() covers large captures of small text
        // after the first pass.)
        const int minSide = qMin(prepared.width(), prepared.height());
        if (minSide < 100) {
            scale = 4.0;
        } else if (minSide < 250) {
            scale = 3.0;
        } else if (minSide < 500) {
            scale = 2.0;
        } else {
            scale = 1.0;
        }
    }

    if (maxDimension > 0) {
        const int maxSide = qMax(prepared.width(), prepared.height());
        scale = qMin(scale, qreal(maxDimension) / maxSide);
    }

    if (scale > 1.01) {
        prepared = prepared.scaled(prepared.size() * scale,
                                   Qt::KeepAspectRatio,
                                   Qt::SmoothTransformation);
    }

    // Oversized captures must fit within the engine's limit
    if (maxDimension > 0 && (prepared.width() > maxDimension ||
                             prepared.height() > maxDimension)) {
        prepared = prepared.scaled(maxDimension,
                                   maxDimension,
                                   Qt::KeepAspectRatio,
                                   Qt::SmoothTransformation);
    }

    // scaled() does not necessarily preserve the pixel format
    if (prepared.format() != QImage::Format_RGBA8888) {
        prepared = prepared.convertToFormat(QImage::Format_RGBA8888);
    }
    return prepared;
}

qreal ocrRefinementScale(const QVector<OcrLine>& lines)
{
    QVector<qreal> heights;
    heights.reserve(lines.size());
    for (const OcrLine& line : lines) {
        if (line.boundingBox.height() > 0) {
            heights.append(line.boundingBox.height());
        }
    }
    if (heights.isEmpty()) {
        return 0.0;
    }
    std::sort(heights.begin(), heights.end());
    const qreal median = heights[heights.size() / 2];

    // ~30 px glyphs are the engine's sweet spot; below ~26 px accuracy
    // degrades enough that a second pass pays off
    if (median >= 26.0) {
        return 0.0;
    }
    return qMin<qreal>(4.0, 32.0 / median);
}
