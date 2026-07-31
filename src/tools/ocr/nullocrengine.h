// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "tools/ocr/ocrengine.h"

/**
 * @brief Stub engine for platforms without an OCR implementation.
 */
class NullOcrEngine : public OcrEngine
{
public:
    bool isAvailable() const override { return false; }

    QStringList availableLanguages() const override { return {}; }

    OcrResult recognize(const QImage& image,
                        const QString& language = QString()) override
    {
        Q_UNUSED(image)
        Q_UNUSED(language)
        OcrResult result;
        result.status = OcrResult::Status::Unsupported;
        return result;
    }
};
