// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "tools/ocr/ocrengine.h"

/**
 * @brief OcrEngine backed by Windows.Media.Ocr via C++/WinRT.
 *
 * Uses the OCR engine built into Windows 10/11; languages come from the
 * installed language packs (Settings > Time & Language). No external
 * dependencies or trained data files.
 */
class WindowsOcrEngine : public OcrEngine
{
public:
    bool isAvailable() const override;
    QStringList availableLanguages() const override;
    OcrResult recognize(const QImage& image,
                        const QString& language = QString()) override;
    int maxImageDimension() const override;
};
