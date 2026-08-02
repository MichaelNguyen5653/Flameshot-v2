// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "rectangletool.h"

#include "tools/highlightstyle.h"
#include "utils/confighandler.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <cmath>

namespace {

const int MinFillMode = 0;
const int MaxFillMode = 1;

bool isValidFillMode(int mode)
{
    return mode >= MinFillMode && mode <= MaxFillMode;
}

} // unnamed namespace

RectangleTool::RectangleTool(QObject* parent)
  : AbstractTwoPointTool(parent)
{
    const int configuredFillMode = ConfigHandler().rectangleFillMode();
    if (isValidFillMode(configuredFillMode)) {
        m_fillMode = static_cast<FillMode>(configuredFillMode);
    }

    m_supportsDiagonalAdj = true;
}

QIcon RectangleTool::icon(const QColor& background, bool inEditor) const
{
    Q_UNUSED(inEditor)
    return QIcon(iconPath(background) + "square.svg");
}
QString RectangleTool::name() const
{
    return tr("Rectangle");
}

CaptureTool::Type RectangleTool::type() const
{
    return CaptureTool::TYPE_RECTANGLE;
}

QString RectangleTool::description() const
{
    return tr("Set the Rectangle as the paint tool");
}

QWidget* RectangleTool::configurationWidget()
{
    auto* widget = new QWidget();
    auto* layout = new QHBoxLayout(widget);
    auto* label = new QLabel(tr("Fill:"), widget);
    auto* modeSelector = new QComboBox(widget);

    modeSelector->addItem(tr("Solid"));
    modeSelector->addItem(tr("Highlighter"));
    modeSelector->setCurrentIndex(static_cast<int>(m_fillMode));
    connect(modeSelector,
            qOverload<int>(&QComboBox::currentIndexChanged),
            this,
            &RectangleTool::setFillMode);

    layout->addWidget(label);
    layout->addWidget(modeSelector);

    return widget;
}

void RectangleTool::setFillMode(int mode)
{
    if (!isValidFillMode(mode)) {
        mode = static_cast<int>(FillMode::Solid);
    }
    m_fillMode = static_cast<FillMode>(mode);
    ConfigHandler().setRectangleFillMode(mode);
}

CaptureTool* RectangleTool::copy(QObject* parent)
{
    auto* tool = new RectangleTool(parent);
    copyParams(this, tool);
    return tool;
}

void RectangleTool::copyParams(const RectangleTool* from, RectangleTool* to)
{
    AbstractTwoPointTool::copyParams(from, to);
    to->m_fillMode = from->m_fillMode;
}

QPainterPath RectangleTool::roundedPath() const
{
    QPainterPath path;
    int offset = size() <= 1 ? 1 : static_cast<int>(round(size() / 2 + 0.5));
    path.addRoundedRect(
      QRectF(std::min(points().first.x(), points().second.x()) - offset,
             std::min(points().first.y(), points().second.y()) - offset,
             std::abs(points().first.x() - points().second.x()) + offset * 2,
             std::abs(points().first.y() - points().second.y()) + offset * 2),
      size(),
      size());
    return path;
}

void RectangleTool::process(QPainter& painter, const QPixmap& pixmap)
{
    Q_UNUSED(pixmap)

    if (m_fillMode == FillMode::Highlighter) {
        // Filled only, never stroked: an outline would be composited over
        // the fill it sits on and darken the border, which a marker stroke
        // of a single colour never does
        HighlightStyle::PainterState highlight(painter);
        if (size() == 0) {
            painter.fillRect(
              QRect(points().first, points().second).normalized(), color());
        } else {
            painter.fillPath(roundedPath(), color());
        }
        return;
    }

    QPen orig_pen = painter.pen();
    QBrush orig_brush = painter.brush();
    painter.setPen(
      QPen(color(), size(), Qt::SolidLine, Qt::SquareCap, Qt::RoundJoin));
    painter.setBrush(QBrush(color()));
    if (size() == 0) {
        painter.drawRect(QRect(points().first, points().second));
    } else {
        painter.fillPath(roundedPath(), color());
    }
    painter.setPen(orig_pen);
    painter.setBrush(orig_brush);
}

void RectangleTool::drawStart(const CaptureContext& context)
{
    AbstractTwoPointTool::drawStart(context);
    onSizeChanged(context.toolSize);
}

void RectangleTool::pressed(CaptureContext& context)
{
    Q_UNUSED(context)
}
