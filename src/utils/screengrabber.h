// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include "utils/desktopinfo.h"

#include <QEvent>
#include <QList>
#include <QMap>
#include <QObject>
#include <QPixmap>
#include <QScreen>

class QEventLoop;
class QWidget;
class MonitorPreview;

class ScreenGrabber : public QObject
{
    Q_OBJECT
public:
    explicit ScreenGrabber(QObject* parent = nullptr);
    QPixmap grabEntireDesktop(bool& ok, int preSelectedMonitor = -1);
    QPixmap grabFullDesktop(bool& ok);
    QRect screenGeometry(QScreen* screen);
    QPixmap grabScreen(QScreen* screenNumber, bool& ok);
    void freeDesktopPortal(bool& ok, QPixmap& res);
    QRect desktopGeometry();
    QRect logicalDesktopGeometry();
    int getSelectedMonitor() const { return m_selectedMonitor; }
    QScreen* getSelectedScreen() const;
    QPixmap selectMonitorAndCrop(const QPixmap& fullScreenshot, bool& ok);
    // True when the last grab kept the whole desktop so the capture widget
    // can span all monitors (captureRegionMode == RegionAllMonitors)
    bool isSpanning() const { return m_spanningCapture; }
    // Native (physical pixel) per-screen grabs retained from the last
    // windowsScreenshot() call. QPixmap is implicitly shared, so copying
    // the map is cheap.
    QMap<QScreen*, QPixmap> screenPixmaps() const { return m_screenPixmaps; }

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    void adjustDevicePixelRatio(QPixmap& pixmap);
    QWidget* createMonitorPreviews(const QPixmap& fullScreenshot);
    void cancelMonitorSelection();
    void moveHighlightedMonitorPreview(int offset);
    int previewIndexForMonitor(int monitorIndex) const;
    void selectHighlightedMonitorPreview();
    void selectMonitor(int monitorIndex);
    void setHighlightedMonitorPreview(int previewIndex);
    QPixmap cropToMonitor(const QPixmap& fullScreenshot, int monitorIndex);
    QPixmap windowsScreenshot(int wid);
    QPixmap x11LegacyScreenshot();

    DesktopInfo m_info;
    QPixmap Screenshot;
    QMap<QScreen*, QPixmap> m_screenPixmaps;
    bool m_spanningCapture;
    int m_selectedMonitor;
    int m_highlightedMonitorPreview;
    QList<MonitorPreview*> m_monitorPreviews;
    QEventLoop* m_monitorSelectionLoop;
    bool m_userCancelled;
    static bool m_monitorSelectionActive;
};
