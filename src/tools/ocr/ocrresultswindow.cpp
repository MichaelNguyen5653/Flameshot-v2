// SPDX-License-Identifier: GPL-3.0-or-later

#include "ocrresultswindow.h"

#include "tools/ocr/ocrpreprocess.h"
#include "utils/confighandler.h"
#include "widgets/loadspinner.h"

#include <QApplication>
#include <QClipboard>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QThread>
#include <QTimer>
#include <QVBoxLayout>

#include <utility>

OcrWorker::OcrWorker(QImage image, QString language, quint64 runId)
  : m_image(std::move(image))
  , m_language(std::move(language))
  , m_runId(runId)
{}

void OcrWorker::process()
{
    // The engine is created inside the worker thread so nothing is shared
    // with the GUI thread
    QScopedPointer<OcrEngine> engine(OcrEngine::create());
    const QImage prepared =
      ocrPreprocessImage(m_image, engine->maxImageDimension());
    emit finished(engine->recognize(prepared, m_language), m_runId);
}

OcrResultsWindow::OcrResultsWindow(const QPixmap& capture, QWidget* parent)
  : QWidget(parent, Qt::Window | Qt::WindowStaysOnTopHint)
  , m_capture(capture)
  , m_engine(OcrEngine::create())
{
    // The REQ_ADD_EXTERNAL_WIDGETS handler sets this too; set it here as
    // well so the window cannot leak if created some other way
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("Extracted text"));
    resize(480, 380);

    auto* layout = new QVBoxLayout(this);

    const QStringList languages = m_engine->availableLanguages();
    if (languages.size() > 1) {
        auto* langLayout = new QHBoxLayout();
        langLayout->addWidget(new QLabel(tr("Language:"), this));
        m_languageBox = new QComboBox(this);
        m_languageBox->addItem(tr("System default"), QString());
        for (const QString& tag : languages) {
            m_languageBox->addItem(tag, tag);
        }
        connect(m_languageBox,
                static_cast<void (QComboBox::*)(int)>(
                  &QComboBox::currentIndexChanged),
                this,
                &OcrResultsWindow::onLanguageChanged);
        langLayout->addWidget(m_languageBox);
        langLayout->addStretch();
        layout->addLayout(langLayout);
    }

    auto* statusLayout = new QHBoxLayout();
    m_spinner = new LoadSpinner(this);
    m_spinner->setColor(ConfigHandler().uiColor());
    m_spinner->hide();
    statusLayout->addWidget(m_spinner);
    m_statusLabel = new QLabel(this);
    m_statusLabel->setWordWrap(true);
    m_statusLabel->hide();
    statusLayout->addWidget(m_statusLabel, 1);
    layout->addLayout(statusLayout);

    m_textEdit = new QPlainTextEdit(this);
    layout->addWidget(m_textEdit, 1);

    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    m_copyButton = new QPushButton(tr("Copy all"), this);
    connect(
      m_copyButton, &QPushButton::clicked, this, &OcrResultsWindow::copyAll);
    buttonLayout->addWidget(m_copyButton);
    layout->addLayout(buttonLayout);

    // Busy indicator appears only when recognition is noticeably slow
    m_busyDelay = new QTimer(this);
    m_busyDelay->setSingleShot(true);
    m_busyDelay->setInterval(200);
    connect(m_busyDelay, &QTimer::timeout, this, [this]() {
        m_statusLabel->setText(tr("Recognizing text…"));
        m_statusLabel->show();
        m_spinner->show();
        m_spinner->start();
    });

    startRecognition();
}

void OcrResultsWindow::copyAll()
{
    QApplication::clipboard()->setText(m_textEdit->toPlainText());
}

void OcrResultsWindow::onLanguageChanged(int index)
{
    Q_UNUSED(index)
    startRecognition();
}

void OcrResultsWindow::startRecognition()
{
    const quint64 runId = ++m_runId;
    m_statusLabel->hide();
    m_copyButton->setEnabled(false);
    m_textEdit->setPlainText(QString());
    m_busyDelay->start();

    QString language;
    if (m_languageBox) {
        language = m_languageBox->currentData().toString();
    }

    // The worker communicates only through a queued connection, which Qt
    // severs safely if this window is closed mid-recognition; the thread
    // then finishes on its own and deletes itself
    auto* thread = new QThread();
    auto* worker = new OcrWorker(m_capture.toImage(), language, runId);
    worker->moveToThread(thread);
    connect(thread, &QThread::started, worker, &OcrWorker::process);
    connect(
      worker, &OcrWorker::finished, this, &OcrResultsWindow::onWorkerFinished);
    connect(worker, &OcrWorker::finished, thread, &QThread::quit);
    connect(thread, &QThread::finished, worker, &QObject::deleteLater);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    thread->start();
}

void OcrResultsWindow::onWorkerFinished(const OcrResult& result, quint64 runId)
{
    // A language change may have started a newer run; ignore stale results
    if (runId != m_runId) {
        return;
    }
    applyResult(result);
}

void OcrResultsWindow::applyResult(const OcrResult& result)
{
    m_busyDelay->stop();
    m_spinner->stop();
    m_spinner->hide();
    m_statusLabel->hide();

    switch (result.status) {
        case OcrResult::Status::Ok:
            m_textEdit->setPlainText(result.fullText);
            m_copyButton->setEnabled(true);
            break;
        case OcrResult::Status::NoTextFound:
            m_statusLabel->setText(tr("No text was found in the selection."));
            m_statusLabel->show();
            break;
        case OcrResult::Status::NoLanguageInstalled:
            m_statusLabel->setText(
              tr("No OCR language is installed. Add one under Settings > "
                 "Time & Language > Language & region > Add a language; "
                 "language packs include text recognition."));
            m_statusLabel->show();
            break;
        case OcrResult::Status::Unsupported:
            m_statusLabel->setText(
              tr("Text recognition is not supported on this platform."));
            m_statusLabel->show();
            break;
        case OcrResult::Status::EngineError:
            m_statusLabel->setText(
              result.errorMessage.isEmpty()
                ? tr("Text recognition failed.")
                : tr("Text recognition failed: %1").arg(result.errorMessage));
            m_statusLabel->show();
            break;
    }
}
