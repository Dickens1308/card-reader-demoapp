#ifndef API_CLIENT_HPP
#define API_CLIENT_HPP

#include <QString>
#include <QByteArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <curl/curl.h>
#include "signature_helper.hpp"

class ApiClient
{
public:
    ApiClient();
    ~ApiClient();

    struct Response
    {
        bool success;
        int statusCode;
        QString status;        // "AS" for success, "AF" for failure
        QString statusCodeStr; // "2101" for success, others for failure
        QString message;
        QString transactionId;
        QJsonObject fareMediaTap;
        QJsonObject fullData;
    };

    bool initialize();
    Response sendCardTap(const QString &cardNumber, const QString &cardData, double amount = 750.0);

private:
    CURL *curl;
    SignatureHelper signatureHelper;

    static size_t writeCallback(void *contents, size_t size, size_t nmemb, void *userp);
    QString buildRequestPayload(const QString &cardNumber, const QString &cardData, double amount);
    qint64 getCurrentTimestamp();
    QString extractDataJson(const QString &responseStr);
};

#endif // API_CLIENT_HPP