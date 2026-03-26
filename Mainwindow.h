#pragma once

#include <QWidget>

class QLabel;
class QTableWidget;
class QPushButton;
class QComboBox;
class QDragEnterEvent;
class QDropEvent;

class MainWindow : public QWidget {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private slots:
    void onConvertClicked();
    void onClearClicked() ;

private:
    void setupUi();
    void addFileIfValid(const QString &filePath);
    bool isMp4File(const QString &filePath) const;

    QString ffmpegPath() const;
    QString ffprobePath() const;
    QString outputFilePath(const QString &inputPath) const;

    QString calculateMp3Size(double durationSeconds, int bitrate) const;
    double getVideoDuration(const QString &filePath);

    void updateEstimatedSize(int row);
    QString formatSize(double bytes) const;

    QLabel *titleLabel{};
    QLabel *dropLabel{};
    QTableWidget *table{};
    QPushButton *convertButton{};
    QPushButton *clearButton{};
    QComboBox *formatBox{};
    QComboBox *bitrateBox{};
};