#include "MainWindow.h"

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QMimeData>
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

    convertButton = new QPushButton("Convert", this);
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
        const QString filePath = url.toLocalFile();
        addFileIfValid(filePath);
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
    QFileInfo fileInfo(filePath);
    return fileInfo.exists() && fileInfo.isFile() &&
           fileInfo.suffix().compare("mp4", Qt::CaseInsensitive) == 0;
}

void MainWindow::onConvertClicked() {
    if (fileList->count() == 0) {
        QMessageBox::information(this, "Info", "Сначала добавь хотя бы один MP4 файл.");
        return;
    }

    QMessageBox::information(this, "Info", "Дальше подключим FFmpeg и настоящую конвертацию.");
}

void MainWindow::onClearClicked() {
    fileList->clear();
}