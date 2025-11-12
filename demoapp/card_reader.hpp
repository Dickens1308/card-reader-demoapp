/*******************************************************************************
 * Card Reader Header - Updated with Authentication
 *******************************************************************************/

#ifndef CARD_READER_HPP
#define CARD_READER_HPP

#include <QString>
#include <QObject>
#include <coupler.hpp>
#include <libals.h>

class CardReader : public QObject
{
    Q_OBJECT

public:
    struct CardData
    {
        bool success;
        QString errorMessage;
        QString cardUid;
        QString cardType;
        QByteArray rawData;
    };

    CardReader();
    ~CardReader();

    bool initialize();
    void shutdown();
    bool waitForReady(unsigned int millisecondsTimeout);
    CardData scanCard(unsigned int timeoutSeconds);

signals:
    void cardDetected(QString cardType);
    void authenticationFailed();
    void readProgress(QString message);
    void scanComplete(bool success, QString message);

private:
    bool authenticateAndRead(Coupler *coupler, const uint8_t *keyA, int sector,
                             int startBlock, int endBlock, QByteArray &outData);
    bool processMifareClassic(Coupler *coupler, QByteArray &outData);
    bool processMifareUL(Coupler *coupler, QByteArray &outData);
    QString bytesToHex(const uchar *data, int length);

    Coupler _coupler;
    CouplerExternalDependencies _ext;
    bool _initialized;

    static constexpr const char *COUPLER_TTY = "/dev/aep/coupler_tty";
    static constexpr const char *COUPLER_POWER = "/dev/aep/coupler_power";
    static constexpr const char *COUPLER_LINK = "/dev/ttyCOUPLER";
};

#endif // CARD_READER_HPP
