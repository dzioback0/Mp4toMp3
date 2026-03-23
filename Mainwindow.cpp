#include "MainWindow.h"

#include <QApplication>
#include <QComboBox>
#include <QCoreApplication>
#include <QDir>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QMimeData>
#include <QProcess>
#include <QPushButton>
#include <QTableWidget>
#include <QUrl>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent) : QWidget(parent) {
    setWindowTitle("MP4 to MP3 Converter");
    resize(850, 550);
    setAcceptDrops(true);

    setupUi();
}

void MainWindow::setupUi() {
    auto *mainLayout = new QVBoxLayout(this);

    titleLabel = new QLabel("MP4 to MP3 Converter", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 22px; font-weight: bold;");

    dropLabel = new QLabel("Drop here MP4 files", this);
    dropLabel->setAlignment(Qt::AlignCenter);
    dropLabel->setMinimumHeight(100);
    dropLabel->setStyleSheet(
        "border: 2px dashed #777;"
        "border-radius: 12px;"
        "font-size: 16px;"
        "padding: 20px;"
    );

    formatBox = new QComboBox(this);
    formatBox->addItems({"MP3", "FLAC"});

    bitrateBox = new QComboBox(this);
    bitrateBox->addItems({"96", "128", "192", "320"});

    auto *topLayout = new QHBoxLayout();
    topLayout->addWidget(new QLabel("Format:", this));
    topLayout->addWidget(formatBox);
    topLayout->addSpacing(20);
    topLayout->addWidget(new QLabel("Bitrate:", this));
    topLayout->addWidget(bitrateBox);
    topLayout->addStretch();

    table = new QTableWidget(this);
    table->setColumnCount(5);
    table->setHorizontalHeaderLabels({"File", "Format", "Bitrate", "Estimated Size", "Status"});
    table->horizontalHeader()->setStretchLastSection(true);
    table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);

    convertButton = new QPushButton("Convert", this);
    clearButton = new QPushButton("Clear", this);

    auto *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(convertButton);
    buttonLayout->addWidget(clearButton);

    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(dropLabel);
    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(table);
    mainLayout->addLayout(buttonLayout);

    connect(convertButton, &QPushButton::clicked, this, &MainWindow::onConvertClicked);
    connect(clearButton, &QPushButton::clicked, this, &MainWindow::onClearClicked);

    connect(formatBox, &QComboBox::currentTextChanged, this, [this]() {
        const bool isMp3 = (formatBox->currentText() == "MP3");
        bitrateBox->setEnabled(isMp3);

        for (int i = 0; i < table->rowCount(); ++i) {
            table->item(i, 1)->setText(formatBox->currentText());
            table->item(i, 2)->setText(isMp3 ? bitrateBox->currentText() : "-");
            updateEstimatedSize(i);
        }
    });

    connect(bitrateBox, &QComboBox::currentTextChanged, this, [this]() {
        if (formatBox->currentText() != "MP3") {
            return;
        }

        for (int i = 0; i < table->rowCount(); ++i) {
            table->item(i, 2)->setText(bitrateBox->currentText());
            updateEstimatedSize(i);
        }
    });
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void MainWindow::dragMoveEvent(QDragMoveEvent *event) {
    const QPoint pos = event->position().toPoint();
    const QRect dropRect = dropLabel->geometry();

    if (dropRect.contains(pos)) {
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}

void MainWindow::dropEvent(QDropEvent *event) {
    const QPoint pos = event->position().toPoint();
    const QRect dropRect = dropLabel->geometry();

    if (!dropRect.contains(pos)) {
        event->ignore();
        return;
    }

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

    for (int i = 0; i < table->rowCount(); ++i) {
        if (table->item(i, 0)->text() == filePath) {
            return;
        }
    }

    const int row = table->rowCount();
    table->insertRow(row);

    const QString currentFormat = formatBox->currentText();
    const QString currentBitrate = (currentFormat == "MP3") ? bitrateBox->currentText() : "-";

    table->setItem(row, 0, new QTableWidgetItem(filePath));
    table->setItem(row, 1, new QTableWidgetItem(currentFormat));
    table->setItem(row, 2, new QTableWidgetItem(currentBitrate));
    table->setItem(row, 3, new QTableWidgetItem("calculating..."));
    table->setItem(row, 4, new QTableWidgetItem("waiting"));

    updateEstimatedSize(row);
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

QString MainWindow::outputFilePath(const QString &inputPath) const {
    QFileInfo info(inputPath);
    const QString format = formatBox->currentText().toLower();
    return info.path() + "/" + info.completeBaseName() + "." + format;
}

QString MainWindow::formatSize(double bytes) const {
    const double mb = bytes / (1024.0 * 1024.0);
    return QString("~%1 MB").arg(mb, 0, 'f', 2);
}

void MainWindow::updateEstimatedSize(int row) {
    const QString format = table->item(row, 1)->text();

    // Пока временно используем фиксированную длительность 180 сек.
    // Потом заменим на реальную через ffprobe.
    const double durationSeconds = 180.0;

    if (format == "MP3") {
        const int bitrate = table->item(row, 2)->text().toInt();
        const double sizeBytes = durationSeconds * bitrate * 1000.0 / 8.0;
        table->item(row, 3)->setText(formatSize(sizeBytes));
    } else {
        // Очень грубая оценка для FLAC
        const double estimatedFlacBytes = durationSeconds * 44100.0 * 2.0 * 2.0 * 0.6;
        table->item(row, 3)->setText(formatSize(estimatedFlacBytes));
    }
}

void MainWindow::onConvertClicked() {
    if (table->rowCount() == 0) {
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
    QList<int> rowsToRemove;

    for (int i = 0; i < table->rowCount(); ++i) {
        const QString inputFile = table->item(i, 0)->text();
        const QString format = table->item(i, 1)->text();
        const QString bitrate = table->item(i, 2)->text();
        const QString outputFile = outputFilePath(inputFile);

        table->item(i, 4)->setText("converting...");
        QApplication::processEvents();

        QProcess process;
        QStringList args;
        args << "-y" << "-i" << inputFile << "-vn";

        if (format == "MP3") {
            args << "-acodec" << "libmp3lame"
                 << "-b:a" << (bitrate + "k");
        } else {
            args << "-acodec" << "flac";
        }

        args << outputFile;

        process.start(ffmpeg, args);
        const bool finished = process.waitForStarted() && process.waitForFinished(-1);

        if (finished && process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0) {
            table->item(i, 4)->setText("done");
            ++successCount;
            rowsToRemove.append(i);
        } else {
            table->item(i, 4)->setText("error");
            failedFiles << QFileInfo(inputFile).fileName();
        }
    }

    for (int j = rowsToRemove.size() - 1; j >= 0; --j) {
        table->removeRow(rowsToRemove[j]);
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
    table->setRowCount(0);
}