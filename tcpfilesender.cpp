#include "tcpfilesender.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QHostAddress>
#include <QRegularExpression>

TcpFileSender::TcpFileSender(QWidget *parent)
    : QDialog(parent)
{
    loadSize = 1024 * 4;
    totalBytes = 0;
    bytesWritten = 0;
    bytesToWrite = 0;

    // Create the UI elements
    clientProgressBar = new QProgressBar;
    clientStatusLabel = new QLabel(QStringLiteral("客戶端就緒"));
    startButton = new QPushButton(QStringLiteral("開始"));
    quitButton = new QPushButton(QStringLiteral("退出"));
    openButton = new QPushButton(QStringLiteral("開檔"));
    startButton->setEnabled(false);

    // URL and Port input fields
    urlLineEdit = new QLineEdit;
    urlLineEdit->setPlaceholderText("請輸入伺服器 URL");
    portLineEdit = new QLineEdit;
    portLineEdit->setPlaceholderText("請輸入端口");

    buttonBox = new QDialogButtonBox;
    buttonBox->addButton(startButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(openButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(quitButton, QDialogButtonBox::RejectRole);

    // Layout setup
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(urlLineEdit);
    mainLayout->addWidget(portLineEdit);
    mainLayout->addWidget(clientProgressBar);
    mainLayout->addWidget(clientStatusLabel);
    mainLayout->addStretch(1);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    setWindowTitle(QStringLiteral("(版本控制Git管理)檔案傳送"));

    // Connect signals to slots
    connect(openButton, &QPushButton::clicked, this, &TcpFileSender::openFile);
    connect(startButton, &QPushButton::clicked, this, &TcpFileSender::start);
    connect(&tcpClient, &QTcpSocket::connected, this, &TcpFileSender::startTransfer);
    connect(&tcpClient, &QTcpSocket::bytesWritten, this, &TcpFileSender::updateClientProgress);
    connect(quitButton, &QPushButton::clicked, this, &TcpFileSender::close);
}

void TcpFileSender::openFile()
{
    fileName = QFileDialog::getOpenFileName(this);
    if (!fileName.isEmpty()) {
        startButton->setEnabled(true);
    }
}

void TcpFileSender::start()
{
    // Validate URL and Port
    QString serverUrl = urlLineEdit->text();
    QString portString = portLineEdit->text();

    if (serverUrl.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("錯誤"), QStringLiteral("請輸入伺服器的 URL"));
        return;
    }

    bool ok;
    int port = portString.toInt(&ok);
    if (!ok || port <= 0 || port > 65535) {
        QMessageBox::warning(this, QStringLiteral("錯誤"), QStringLiteral("請輸入有效的端口號"));
        return;
    }

    startButton->setEnabled(false);
    bytesWritten = 0;
    clientStatusLabel->setText(QStringLiteral("連接中..."));

    // Connect to the server using user-provided URL and port
    tcpClient.connectToHost(serverUrl, port);
}

void TcpFileSender::startTransfer()
{
    localFile = new QFile(fileName);
    if (!localFile->open(QFile::ReadOnly)) {
        QMessageBox::warning(this, QStringLiteral("應用程式"),
                             QStringLiteral("無法讀取 %1:\n%2.").arg(fileName)
                                 .arg(localFile->errorString()));
        return;
    }

    totalBytes = localFile->size();
    QDataStream sendOut(&outBlock, QIODevice::WriteOnly);
    sendOut.setVersion(QDataStream::Qt_4_6);
    QString currentFile = fileName.right(fileName.size() - fileName.lastIndexOf("/") - 1);
    sendOut << qint64(0) << qint64(0) << currentFile;
    totalBytes += outBlock.size();

    sendOut.device()->seek(0);
    sendOut << totalBytes << qint64((outBlock.size() - sizeof(qint64) * 2));
    bytesToWrite = totalBytes - tcpClient.write(outBlock);
    clientStatusLabel->setText(QStringLiteral("已連接"));
    qDebug() << currentFile << totalBytes;
    outBlock.resize(0);
}

void TcpFileSender::updateClientProgress(qint64 numBytes)
{
    bytesWritten += static_cast<int>(numBytes);
    if (bytesToWrite > 0) {
        outBlock = localFile->read(qMin(bytesToWrite, loadSize));
        bytesToWrite -= static_cast<int>(tcpClient.write(outBlock));
        outBlock.resize(0);
    } else {
        localFile->close();
    }

    clientProgressBar->setMaximum(totalBytes);
    clientProgressBar->setValue(bytesWritten);
    clientStatusLabel->setText(QStringLiteral("已傳送 %1 Bytes").arg(bytesWritten));
}

TcpFileSender::~TcpFileSender()
{
}
