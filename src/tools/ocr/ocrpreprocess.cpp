// SPDX-License-Identifier: GPL-3.0-or-later

#include "ocrpreprocess.h"

QImage ocrPreprocessImage(const QImage& image, int maxDimension)
{
    if (image.isNull()) {
        return image;
    }

    QImage prepared = image.convertToFormat(QImage::Format_RGBA8888);

    // The glyph height is unknown before recognition, so the selection size
    // is used as a proxy: small captures are usually tight crops around
    // text, where upscaling lifts glyphs towards the ~30 px the engine
    // wants. Large captures are left alone.
    const int minSide = qMin(prepared.width(), prepared.height());
    int scale = 1;
    if (minSide < 100) {
        scale = 4;
    } else if (minSide < 250) {
        scale = 3;
    } else if (minSide < 500) {
        scale = 2;
    }

    if (maxDimension > 0) {
        const int maxSide = qMax(prepared.width(), prepared.height());
        while (scale > 1 && maxSide * scale > maxDimension) {
            --scale;
        }
    }

    if (scale > 1) {
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
