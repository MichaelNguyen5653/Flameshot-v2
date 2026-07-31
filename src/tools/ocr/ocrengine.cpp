// SPDX-License-Identifier: GPL-3.0-or-later

#include "ocrengine.h"

#include "tools/ocr/nullocrengine.h"

OcrEngine* OcrEngine::create()
{
    return new NullOcrEngine();
}
