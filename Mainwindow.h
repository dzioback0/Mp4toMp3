#pragma once

#include <QWidget>

class QLabel;
class QListWidget;
class QPushButton;
class QDragEnterEvent;
class QDropEvent;

class MainWindow : public QWidget {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private slots:
    void onConvertClicked();
    void onClearClicked();

private:
    void setupUi();
    void addFileIfValid(const QString &filePath);
    bool isMp4File(const QString &filePath) const;
    QString ffmpegPath() const;
    QString outputMp3Path(const QString &inputPath) const;

    QLabel *titleLabel{};
    QLabel *dropLabel{};
    QListWidget *fileList{};
    QPushButton *convertButton{};
    QPushButton *clearButton{};
};