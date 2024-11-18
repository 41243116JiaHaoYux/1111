#ifndef TCPFILESENDER_H
#define TCPFILESENDER_H

#include <QDialog>
#include <QtNetwork>
#include <QtWidgets>

class TcpFileSender : public QDialog
{
    Q_OBJECT

public:
    explicit TcpFileSender(QWidget *parent = nullptr);  // 使用 nullptr 替换 0
    ~TcpFileSender();

public slots:
    void start();
    void startTransfer();
    void updateClientProgress(qint64 numBytes);
    void openFile();

private:
    // UI components
    QProgressBar     *clientProgressBar;
    QLabel           *clientStatusLabel;
    QPushButton      *startButton;
    QPushButton      *quitButton;
    QPushButton      *openButton;
    QDialogButtonBox *buttonBox;

    // Input fields for URL and port
    QLineEdit        *urlLineEdit;  // 用于输入服务器 URL
    QLineEdit        *portLineEdit; // 用于输入端口

    // File transfer variables
    QTcpSocket       tcpClient;
    qint64           totalBytes;
    qint64           bytesWritten;
    qint64           bytesToWrite;
    qint64           loadSize;
    QString          fileName;
    QFile            *localFile;
    QByteArray       outBlock;
};

#endif // TCPFILESENDER_H
