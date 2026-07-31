// SPDX-License-Identifier: GPL-3.0-or-later

#include "ocrtool.h"

#include "tools/ocr/ocrresultswindow.h"

OcrTool::OcrTool(QObject* parent)
  : AbstractActionTool(parent)
{}

bool OcrTool::closeOnButtonPressed() const
{
    return false;
}

QIcon OcrTool::icon(const QColor& background, bool inEditor) const
{
    Q_UNUSED(inEditor)
    return QIcon(iconPath(background) + "ocr.svg");
}

QString OcrTool::name() const
{
    return tr("OCR");
}

CaptureTool::Type OcrTool::type() const
{
    return CaptureTool::TYPE_OCR;
}

QString OcrTool::description() const
{
    return tr("Extract text from the selection");
}

QWidget* OcrTool::widget()
{
    return new OcrResultsWindow(m_capture);
}

CaptureTool* OcrTool::copy(QObject* parent)
{
    return new OcrTool(parent);
}

void OcrTool::pressed(CaptureContext& context)
{
    m_capture = context.selectedScreenshotArea();
    // Unlike most action tools this does not close the editor: the results
    // window lives on its own and the user may keep annotating
    emit requestAction(REQ_ADD_EXTERNAL_WIDGETS);
}
