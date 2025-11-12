#include "api_client.hpp"
#include "config.hpp"
#include <QDateTime>
#include <QDebug>
#include <cstring>

ApiClient::ApiClient() : curl(nullptr)
{
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
}

ApiClient::~ApiClient()
{
    if (curl)
    {
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
}

bool ApiClient::initialize()
{
    Config &config = Config::instance();

    // Load certificates
    if (!signatureHelper.loadPrivateCertificate(config.privateCertPath, config.certPassword))
    {
        qDebug() << "Failed to load private certificate";
        return false;
    }

    if (!signatureHelper.loadPublicCertificate(config.publicCertPath))
    {
        qDebug() << "Public certificate encountered an error";
        return false;
    }

    qDebug() << "API Client initialized successfully";
    return true;
}

size_t ApiClient::writeCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((QString *)userp)->append((char *)contents);
    return size * nmemb;
}

qint64 ApiClient::getCurrentTimestamp()
{
    return QDateTime::currentMSecsSinceEpoch();
}

QString ApiClient::buildRequestPayload(const QString &cardNumber, const QString &cardData, double amount)
{
    Config &config = Config::instance();
    qint64 timestamp = getCurrentTimestamp();
    QString readableTime = QDateTime::fromMSecsSinceEpoch(timestamp).toString("yyyy-MM-dd HH:mm:ss");
    qDebug() << "Current time:" << readableTime;

    QString transactionId = QString("afcs-tom%1").arg(timestamp);

    // Prepare JSON manually to preserve order
    QString dataString = QString(
                             "{"
                             "\"amount\": %1,"
                             "\"cardData\": \"%2\","
                             "\"fareMediaCode\": \"%3\","
                             "\"cardNumber\": \"%4\","
                             "\"entryTime\": \"%5\","
                             "\"stationCode\": \"%6\","
                             "\"tapChannel\": \"%7\","
                             "\"cardTypeId\": %8,"
                             "\"requestTime\": \"%5\","
                             "\"reservedField1\": \"\","
                             "\"reservedField2\": \"\","
                             "\"reservedField3\": \"\","
                             "\"transactionId\": \"%9\""
                             "}")
                             .arg(amount)
                             .arg(cardData)
                             .arg(config.fareMediaCode)
                             .arg(cardNumber)
                             .arg(readableTime)
                             .arg(config.stationCode)
                             .arg(config.tapChannel)
                             .arg(config.cardTypeId)
                             .arg(transactionId);

    // Sign the compact data string
    QString alg = "SHA1withRSA";
    QString signature = signatureHelper.signData(dataString, alg);

    // Build full JSON request manually
    QString requestJson = QString(
                              "{\"data\": %1, \"signature\": \"%2\"}")
                              .arg(dataString, signature);

    qDebug().noquote() << "Request Body:\n"
                       << requestJson;
    return requestJson;
}

ApiClient::Response ApiClient::sendCardTap(const QString &cardNumber, const QString &cardData, double amount)
{
    Response response;
    response.success = false;
    response.statusCode = 0;

    if (!curl)
    {
        response.message = "cURL not initialized";
        return response;
    }

    Config &config = Config::instance();
    QString responseString;

    // Build request payload
    QString jsonPayload = buildRequestPayload(cardNumber, cardData, amount);
    QByteArray jsonBytes = jsonPayload.toUtf8();
    qDebug().noquote() << "Request Body:\n"
                       << jsonPayload;

    qDebug() << "Sending to API:" << config.apiUrl;

    // Setup cURL
    curl_easy_setopt(curl, CURLOPT_URL, config.apiUrl.toStdString().c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonBytes.constData());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, jsonBytes.size());
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, config.apiTimeout / 1000);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseString);

    // IMPORTANT: Tell cURL to handle chunked encoding automatically
    curl_easy_setopt(curl, CURLOPT_HTTP_TRANSFER_DECODING, 1L);

    // Setup headers
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "Accept: application/json");

    // Use dynamic values from Config
    headers = curl_slist_append(headers, QString("Device: %1").arg(config.deviceName).toStdString().c_str());
    headers = curl_slist_append(headers, QString("Afcs-Code: %1").arg(config.deviceCode).toStdString().c_str());
    headers = curl_slist_append(headers, QString("Version-Number: %1").arg(config.deviceVersion).toStdString().c_str());
    headers = curl_slist_append(headers, QString("Agent-Code: %1").arg(config.agentCode).toStdString().c_str());
    headers = curl_slist_append(headers, QString("Cashier-Code: %1").arg(config.cashierName).toStdString().c_str());

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    // Perform request
    CURLcode res = curl_easy_perform(curl);

    if (res == CURLE_OK)
    {
        long httpCode = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
        response.statusCode = httpCode;

        // Clean up response string - remove any chunked encoding artifacts
        // Find the last '}' which should be the end of actual JSON
        int lastBrace = responseString.lastIndexOf('}');
        if (lastBrace != -1 && lastBrace < responseString.length() - 1)
        {
            qDebug() << "Trimming extra data from response. Original length:" << responseString.length();
            responseString = responseString.left(lastBrace + 1);
            qDebug() << "Cleaned length:" << responseString.length();
        }

        qDebug() << "API Response [" << httpCode << "]:" << responseString;

        // Parse JSON response
        QJsonDocument doc = QJsonDocument::fromJson(responseString.toUtf8());
        if (doc.isObject())
        {
            QJsonObject root = doc.object();

            // Extract signature
            QString respSignature = root.value("signature").toString();

            // CRITICAL: Extract the exact "data" JSON string from raw response
            QString dataJsonStr = extractDataJson(responseString);

            if (!dataJsonStr.isEmpty() && !respSignature.isEmpty())
            {
                qDebug() << "Extracted data JSON:" << dataJsonStr;
                qDebug() << "Signature:" << respSignature;

                // Verify signature
                bool signatureValid = signatureHelper.verifySignature(dataJsonStr, respSignature);
                qDebug() << "Signature verification:" << (signatureValid ? "VALID" : "INVALID");

                if (!signatureValid)
                {
                    qWarning() << "WARNING: Signature verification failed!";
                    // Continue processing anyway for now
                }
            }

            // Extract data object for business logic
            QJsonObject dataObj = root.value("data").toObject();

            // Extract response data
            response.status = dataObj.value("status").toString();
            response.statusCodeStr = dataObj.value("statusCode").toString();
            response.message = dataObj.value("message").toString();
            response.transactionId = dataObj.value("transactionId").toString();
            response.fareMediaTap = dataObj.value("fareMediaTap").toObject();
            response.fullData = dataObj;

            // Check if successful (AS status and 2101 code)
            response.success = (response.status == "AS" && response.statusCodeStr == "2101");

            qDebug() << "Status:" << response.status;
            qDebug() << "Status Code:" << response.statusCodeStr;
            qDebug() << "Message:" << response.message;
            qDebug() << "Transaction ID:" << response.transactionId;
        }
        else
        {
            response.message = "Invalid JSON response";
        }
    }
    else
    {
        response.message = QString("Network error: %1").arg(curl_easy_strerror(res));
        qDebug() << response.message;
    }

    curl_slist_free_all(headers);
    return response;
}

// Add this helper function to your ApiClient class
QString ApiClient::extractDataJson(const QString &responseStr)
{
    int dataStart = responseStr.indexOf("\"data\":");
    if (dataStart == -1)
    {
        return QString();
    }

    dataStart += 7; // Skip past "data":

    // Skip whitespace
    while (dataStart < responseStr.length() && responseStr[dataStart].isSpace())
    {
        dataStart++;
    }

    // Find the matching closing brace for the data object
    int braceCount = 0;
    int dataEnd = dataStart;
    bool inString = false;
    bool escapeNext = false;

    for (int i = dataStart; i < responseStr.length(); i++)
    {
        QChar c = responseStr[i];

        if (escapeNext)
        {
            escapeNext = false;
            continue;
        }

        if (c == '\\')
        {
            escapeNext = true;
            continue;
        }

        if (c == '"')
        {
            inString = !inString;
            continue;
        }

        if (!inString)
        {
            if (c == '{')
            {
                braceCount++;
            }
            if (c == '}')
            {
                braceCount--;
                if (braceCount == 0)
                {
                    dataEnd = i + 1;
                    break;
                }
            }
        }
    }

    // Extract the exact data JSON string
    return responseStr.mid(dataStart, dataEnd - dataStart);
}