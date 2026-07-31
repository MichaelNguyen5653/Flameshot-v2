// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QImage>

/**
 * @brief Prepare a screenshot crop for OCR.
 *
 * Converts to QImage::Format_RGBA8888 and upscales small selections with
 * smooth filtering, since engine accuracy drops sharply when glyphs are
 * under ~30 px tall. If the image exceeds maxDimension in either direction
 * it is scaled down to fit instead. maxDimension of 0 means unlimited.
 *
 * Pure function, kept separate so it can be tested without an engine.
 */
QImage ocrPreprocessImage(const QImage& image, int maxDimension = 0);
