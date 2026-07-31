// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "tools/ocr/ocrengine.h"

#include <QPixmap>
#include <QScopedPointer>
#include <QWidget>

class QComboBox;
class QLabel;
class QPlainTextEdit;
class QPushButton;
class QTimer;
class LoadSpinner;

/**
 * @brief Runs OCR on one image in a worker thread.
 *
 * Lives in the worker thread; communicates purely via a queued signal, so
 * the results window can be closed while recognition is still running.
 */
class OcrWorker : public QObject
{
    Q_OBJECT
public:
    OcrWorker(QImage image, QString language, quint64 runId);

public slots:
    void process();

signals:
    void finished(const OcrResult& result, quint64 runId);

private:
    QImage m_image;
    QString m_language;
    quint64 m_runId;
};

/**
 * @brief Non-modal window showing the text extracted from a capture.
 */
class OcrResultsWindow : public QWidget
{
    Q_OBJECT
public:
    explicit OcrResultsWindow(const QPixmap& capture,
                              QWidget* parent = nullptr);

private slots:
    void copyAll();
    void onLanguageChanged(int index);
    void onWorkerFinished(const OcrResult& result, quint64 runId);

private:
    void startRecognition();
    void applyResult(const OcrResult& result);

    QPixmap m_capture;
    QScopedPointer<OcrEngine> m_engine;

    QComboBox* m_languageBox{ nullptr };
    QLabel* m_statusLabel{ nullptr };
    QPlainTextEdit* m_textEdit{ nullptr };
    QPushButton* m_copyButton{ nullptr };
    LoadSpinner* m_spinner{ nullptr };
    QTimer* m_busyDelay{ nullptr };

    // Increased on every recognition start; stale worker results are
    // discarded by comparing against it
    quint64 m_runId{ 0 };
};
