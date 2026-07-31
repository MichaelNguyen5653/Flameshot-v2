// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "tools/ocr/ocrengine.h"

#include <QImage>

/**
 * @brief Prepare a screenshot crop for OCR.
 *
 * Converts to QImage::Format_RGBA8888, inverts light-on-dark captures
 * (terminals, dark themes — the engine is markedly less accurate on them),
 * and upscales small selections with smooth filtering, since accuracy drops
 * sharply for glyphs under ~30 px. If the image exceeds maxDimension in
 * either direction it is scaled down to fit instead; 0 means unlimited.
 *
 * scaleOverride > 0 replaces the size-based upscale heuristic with an exact
 * factor (still capped by maxDimension); used by the second OCR pass once
 * the real glyph size is known.
 *
 * Pure function, kept separate so it can be tested without an engine.
 */
QImage ocrPreprocessImage(const QImage& image,
                          int maxDimension = 0,
                          qreal scaleOverride = 0.0);

/**
 * @brief Decide whether a second OCR pass on an upscaled image is worth it.
 *
 * The selection size is a bad proxy for glyph size (a full-window capture
 * of a terminal is large but its text is small), so after the first pass
 * the median recognized line height is measured instead. Returns the scale
 * factor to re-run with, or 0 when the text is already large enough.
 */
qreal ocrRefinementScale(const QVector<OcrLine>& lines);
