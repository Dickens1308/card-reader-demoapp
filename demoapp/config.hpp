#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <QString>
#include <QSettings>
#include <QFile>
#include <QDebug>

class Config
{
public:
    static Config &instance()
    {
        static Config instance;
        return instance;
    }

    bool load(const QString &filename = "/home/dart/program-files/card_config.ini")
    {
        if (!QFile::exists(filename))
        {
            qDebug() << "Config file not found:" << filename;
            return false;
        }

        qDebug() << "Config file found in from:" << filename;
        QSettings settings(filename, QSettings::IniFormat);

        // API Settings
        settings.beginGroup("API");
        apiUrl = settings.value("url", "http://192.168.8.116:2601/api/third-party/faremedia/fare-media-tap").toString();
        apiTimeout = settings.value("timeout", 30000).toInt();
        settings.endGroup();

        // Certificate Settings
        settings.beginGroup("Certificate");
        privateCertPath = settings.value("privateCertPath", "/home/dart/program-files/afcsPrivateCertificate.pfx").toString();
        publicCertPath = settings.value("publicCertPath", "/home/dart/program-files/afcsPublic116.pfx").toString();
        certPassword = settings.value("password", "Nyapula@3411").toString();
        settings.endGroup();

        // Card Authentication
        settings.beginGroup("Card");
        QString keyStr = settings.value("keyA", "FFFFFFFFFFFF").toString();
        if (keyStr.length() == 12)
        {
            for (int i = 0; i < 6; i++)
            {
                bool ok;
                keyA[i] = keyStr.mid(i * 2, 2).toUInt(&ok, 16);
            }
        }
        sector = settings.value("sector", 1).toInt();
        startBlock = settings.value("startBlock", 4).toInt();
        endBlock = settings.value("endBlock", 7).toInt();
        settings.endGroup();

        // Device Info
        settings.beginGroup("Device");
        deviceName = settings.value("device", "CDB4V2").toString();
        deviceCode = settings.value("deviceCode", "TOM650").toString();
        deviceVersion = settings.value("deviceVersion", "1.0.0").toString();
        stationCode = settings.value("stationCode", "VKZ123").toString();
        agentCode = settings.value("agentCode", "AGNT011").toString();
        cashierName = settings.value("cashierName", "manualc").toString();
        fareMediaCode = settings.value("fareMediaCode", "NCD01").toString();
        tapChannel = settings.value("tapChannel", "One").toString();
        cardTypeId = settings.value("cardTypeId", 1).toInt();
        settings.endGroup();

        // Messages
        settings.beginGroup("Messages");
        msgScanning = settings.value("scanning", "Weka kadi yako hapa").toString();
        msgAuthFailed = settings.value("authFailed", "Kadi sio sahihi!").toString();
        msgReadFailed = settings.value("readFailed", "Kusoma kumeshindikana!").toString();
        msgSuccess = settings.value("success", "Kadi imesomwa vizuri!").toString();
        msgApiError = settings.value("apiError", "Tatizo la kuwasiliana!").toString();
        msgProcessing = settings.value("processing", "Inaendelea...").toString();
        settings.endGroup();

        qDebug() << "Config loaded successfully";
        return true;
    }

    // API Settings
    QString apiUrl;
    int apiTimeout;

    // Certificate Settings
    QString privateCertPath;
    QString publicCertPath;
    QString certPassword;

    // Card Settings
    uint8_t keyA[6];
    int sector;
    int startBlock;
    int endBlock;

    // Device Info
    QString deviceName;
    QString deviceCode;
    QString deviceVersion;
    QString stationCode;
    QString agentCode;
    QString cashierName;
    QString fareMediaCode;
    QString tapChannel;
    int cardTypeId;

    // Messages
    QString msgScanning;
    QString msgAuthFailed;
    QString msgReadFailed;
    QString msgSuccess;
    QString msgApiError;
    QString msgProcessing;

private:
    Config()
    {
        for (int i = 0; i < 6; i++)
        {
            keyA[i] = 0xFF;
        }
        sector = 1;
        startBlock = 4;
        endBlock = 7;
        cardTypeId = 1;
    }
};

#endif // CONFIG_HPP
