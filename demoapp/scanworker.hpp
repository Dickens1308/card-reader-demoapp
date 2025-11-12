#ifndef SCANWORKER_HPP
#define SCANWORKER_HPP

#include <QObject>
#include <QThread>
#include "card_reader.hpp"

class ScanWorker : public QObject
{
    Q_OBJECT
public:
    explicit ScanWorker(CardReader *reader, QObject *parent = nullptr)
        : QObject(parent), reader(reader) {}
    CardReader *reader;
    bool stopRequested = false;

public slots:
    void startScanning()
    {
        while (!stopRequested)
        {
            CardReader::CardData data = reader->scanCard(5);
            if (data.success)
                emit cardDetected(data);
            QThread::msleep(200);
        }
    }
    void stop() { stopRequested = true; }

signals:
    void cardDetected(CardReader::CardData cardData);
    void scanProgress(QString message);
};

#endif // SCANWORKER_HPP
