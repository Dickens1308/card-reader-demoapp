#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include "card_reader.hpp"
#include "api_client.hpp"

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void startScanning();
    void onCardDetected(QString cardType);
    void onAuthenticationFailed();
    void onReadProgress(QString message);
    void onScanComplete(bool success, QString message);
    void resetToScanScreen();

private:
    Ui::MainWindow *ui;
    CardReader reader;
    ApiClient apiClient;
    QTimer *scanTimer;
    QTimer *resetTimer;

    bool initialized;

    void showScanScreen();
    Q_INVOKABLE void showProcessingScreen();
    Q_INVOKABLE void showErrorScreen(const QString &message);
    Q_INVOKABLE void showSuccessScreen(const QString &message);
    void updateStatusText(const QString &text);
};

#endif // MAINWINDOW_H
