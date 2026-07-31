// SPDX-License-Identifier: GPL-3.0-or-later

#include "ocrengine.h"

#if defined(Q_OS_WIN)
#include "tools/ocr/windowsocrengine.h"
#else
#include "tools/ocr/nullocrengine.h"
#endif

OcrEngine* OcrEngine::create()
{
#if defined(Q_OS_WIN)
    return new WindowsOcrEngine();
#else
    return new NullOcrEngine();
#endif
}
