#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "config.hpp"
#include <QDebug>
#include <QThread>
#include <QtConcurrent/QtConcurrent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), initialized(false)
{
    ui->setupUi(this);

    // Load configuration
    Config &config = Config::instance();
    if (!config.load())
    {
        showErrorScreen("Configuration file not found!");
        return;
    }

    // Initialize card reader
    if (!reader.initialize())
    {
        showErrorScreen("Failed to initialize card reader!");
        return;
    }

    // Initialize API client
    if (!apiClient.initialize())
    {
        showErrorScreen("Failed to initialize API client!");
        return;
    }

    initialized = true;

    // Connect signals
    connect(&reader, &CardReader::cardDetected, this, &MainWindow::onCardDetected);
    connect(&reader, &CardReader::authenticationFailed, this, &MainWindow::onAuthenticationFailed);
    connect(&reader, &CardReader::readProgress, this, &MainWindow::onReadProgress);
    connect(&reader, &CardReader::scanComplete, this, &MainWindow::onScanComplete);

    // Setup timers
    scanTimer = new QTimer(this);
    connect(scanTimer, &QTimer::timeout, this, &MainWindow::startScanning);

    resetTimer = new QTimer(this);
    resetTimer->setSingleShot(true);
    connect(resetTimer, &QTimer::timeout, this, &MainWindow::resetToScanScreen);

    // Start scanning
    showScanScreen();
    QTimer::singleShot(1000, this, &MainWindow::startScanning);
}

MainWindow::~MainWindow()
{
    reader.shutdown();
    delete ui;
}

void MainWindow::showScanScreen()
{
    Config &config = Config::instance();
    ui->stackedWidget->setCurrentWidget(ui->pageScan);
    updateStatusText(config.msgScanning);
}

void MainWindow::showProcessingScreen()
{
    Config &config = Config::instance();
    ui->stackedWidget->setCurrentWidget(ui->pageProcessing);
    updateStatusText(config.msgProcessing);
}

void MainWindow::showErrorScreen(const QString &message)
{
    ui->stackedWidget->setCurrentWidget(ui->pageError);
    updateStatusText(message);

    // Auto-reset after 3 seconds
    resetTimer->start(3000);
}

void MainWindow::showSuccessScreen(const QString &message)
{
    ui->stackedWidget->setCurrentWidget(ui->pageSuccess);
    updateStatusText(message);

    // Auto-reset after 3 seconds
    resetTimer->start(3000);
}

void MainWindow::updateStatusText(const QString &text)
{
    ui->labelScan->setText(text);
    ui->labelError->setText(text);
    ui->labelSuccess->setText(text);
    ui->labelProcessing->setText(text);
}

void MainWindow::resetToScanScreen()
{
    showScanScreen();
    QTimer::singleShot(500, this, &MainWindow::startScanning);
}

void MainWindow::startScanning()
{
    if (!initialized)
    {
        qDebug() << "Card reader not initialized, cannot start scanning";
        return;
    }

    qDebug() << "Starting card scan...";

    // Run scan in separate thread to avoid blocking UI
    QtConcurrent::run([this]()
                      {
        CardReader::CardData cardData = reader.scanCard(30); // 30 second timeout
        
        if (cardData.success) {
            // Convert raw data to hex string
            QString hexData;
            for (int i = 0; i < cardData.rawData.size(); i++) {
                hexData += QString("%1").arg((unsigned char)cardData.rawData[i], 2, 16, QChar('0')).toUpper();
            }
            
            qDebug() << "Card UID:" << cardData.cardUid;
            qDebug() << "Card Data:" << hexData;
            
            // Show processing screen
            QMetaObject::invokeMethod(this, "showProcessingScreen", Qt::QueuedConnection);
            
            // Send to API
            ApiClient::Response apiResp = apiClient.sendCardTap(cardData.cardUid, hexData);
            
            if (apiResp.success) {
                QString successMsg = QString("%1\n\nTransaction: %2")
                    .arg(apiResp.message)
                    .arg(apiResp.transactionId);
                QMetaObject::invokeMethod(this, "showSuccessScreen", Qt::QueuedConnection, 
                                        Q_ARG(QString, successMsg));
            } else {
                Config& config = Config::instance();
                QString errorMsg = apiResp.message.isEmpty() ? config.msgApiError : apiResp.message;
                QMetaObject::invokeMethod(this, "showErrorScreen", Qt::QueuedConnection, 
                                        Q_ARG(QString, errorMsg));
            }
        } });
}

void MainWindow::onCardDetected(QString cardType)
{
    qDebug() << "Card detected:" << cardType;
    updateStatusText(QString("Card detected: %1").arg(cardType));
}

void MainWindow::onAuthenticationFailed()
{
    Config &config = Config::instance();
    showErrorScreen(config.msgAuthFailed);
}

void MainWindow::onReadProgress(QString message)
{
    qDebug() << "Progress:" << message;

    if (message == "Waiting for card...")
        return;

    updateStatusText(message);
}

void MainWindow::onScanComplete(bool success, QString message)
{
    qDebug() << "Scan complete:" << success << "-" << message;

    if (!success)
    {
        Config &config = Config::instance();
        showErrorScreen(config.msgReadFailed);
    }
}