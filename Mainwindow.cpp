#include "MainWindow.h"

#include <QCoreApplication>
#include <QDir>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QMimeData>
#include <QProcess>
#include <QPushButton>
#include <QUrl>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent) : QWidget(parent) {
    setWindowTitle("MP4 to MP3 Converter");
    resize(700, 500);
    setAcceptDrops(true);
    setupUi();
}

void MainWindow::setupUi() {
    auto *mainLayout = new QVBoxLayout(this);

    titleLabel = new QLabel("MP4 to MP3 Converter", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 22px; font-weight: bold;");

    dropLabel = new QLabel("Перетащи сюда MP4 файлы", this);
    dropLabel->setAlignment(Qt::AlignCenter);
    dropLabel->setMinimumHeight(100);
    dropLabel->setStyleSheet(
        "border: 2px dashed #777;"
        "border-radius: 12px;"
        "font-size: 16px;"
        "padding: 20px;"
    );

    fileList = new QListWidget(this);

    convertButton = new QPushButton("Convert to MP3", this);
    clearButton = new QPushButton("Clear", this);

    auto *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(convertButton);
    buttonLayout->addWidget(clearButton);

    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(dropLabel);
    mainLayout->addWidget(fileList);
    mainLayout->addLayout(buttonLayout);

    connect(convertButton, &QPushButton::clicked, this, &MainWindow::onConvertClicked);
    connect(clearButton, &QPushButton::clicked, this, &MainWindow::onClearClicked);
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent *event) {
    const QList<QUrl> urls = event->mimeData()->urls();

    for (const QUrl &url : urls) {
        addFileIfValid(url.toLocalFile());
    }

    event->acceptProposedAction();
}

void MainWindow::addFileIfValid(const QString &filePath) {
    if (!isMp4File(filePath)) {
        return;
    }

    for (int i = 0; i < fileList->count(); ++i) {
        if (fileList->item(i)->text() == filePath) {
            return;
        }
    }

    fileList->addItem(filePath);
}

bool MainWindow::isMp4File(const QString &filePath) const {
    QFileInfo info(filePath);
    return info.exists() && info.isFile() &&
           info.suffix().compare("mp4", Qt::CaseInsensitive) == 0;
}

QString MainWindow::ffmpegPath() const {
    const QString appDir = QCoreApplication::applicationDirPath();
    QDir dir(appDir);

    if (dir.cdUp()) {
        const QString candidate = dir.filePath("ffmpeg/bin/ffmpeg.exe");
        if (QFileInfo::exists(candidate)) {
            return candidate;
        }
    }

    return {};
}

QString MainWindow::outputMp3Path(const QString &inputPath) const {
    QFileInfo info(inputPath);
    return info.path() + "/" + info.completeBaseName() + ".mp3";
}

void MainWindow::onConvertClicked() {
    if (fileList->count() == 0) {
        QMessageBox::information(this, "Info", "Сначала добавь хотя бы один MP4 файл.");
        return;
    }

    const QString ffmpeg = ffmpegPath();
    if (ffmpeg.isEmpty()) {
        QMessageBox::critical(this, "FFmpeg not found",
                              "Не найден ffmpeg.exe\n\n"
                              "Ожидаемый путь:\n"
                              "../ffmpeg/bin/ffmpeg.exe относительно папки с exe.");
        return;
    }

    convertButton->setEnabled(false);
    clearButton->setEnabled(false);

    int successCount = 0;
    QStringList failedFiles;

    for (int i = 0; i < fileList->count(); ++i) {
        const QString inputFile = fileList->item(i)->text();
        const QString outputFile = outputMp3Path(inputFile);

        QProcess process;
        QStringList args;
        args << "-y"
             << "-i" << inputFile
             << "-vn"
             << "-acodec" << "libmp3lame"
             << "-q:a" << "2"
             << outputFile;

        process.start(ffmpeg, args);
        const bool finished = process.waitForStarted() && process.waitForFinished(-1);

        if (finished && process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0) {
            ++successCount;
        } else {
            failedFiles << QFileInfo(inputFile).fileName();
        }
    }

    convertButton->setEnabled(true);
    clearButton->setEnabled(true);

    if (failedFiles.isEmpty()) {
        QMessageBox::information(this, "Done",
                                 QString("Готово. Сконвертировано файлов: %1").arg(successCount));
    } else {
        QMessageBox::warning(this, "Finished with errors",
                             QString("Успешно: %1\nС ошибкой: %2\n\n%3")
                                 .arg(successCount)
                                 .arg(failedFiles.size())
                                 .arg(failedFiles.join("\n")));
    }
}

void MainWindow::onClearClicked() {
    fileList->clear();
}