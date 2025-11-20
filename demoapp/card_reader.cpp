/*******************************************************************************
 * Card Reader Implementation with Authentication
 *******************************************************************************/

#include "card_reader.hpp"
#include "config.hpp"
#include <unistd.h>
#include <cstdio>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <QDebug>

using namespace als::Utils;

CardReader::CardReader() : _initialized(false) {}

CardReader::~CardReader()
{
    shutdown();
}

bool CardReader::initialize()
{
    setbuf(stdout, NULL);

    if (!File::Exists(COUPLER_LINK) && symlink(COUPLER_TTY, COUPLER_LINK) == -1)
    {
        qDebug() << "Failed to create symlink to coupler device\n";
        return 1;
    }

    // Turn on the coupler (for CDB4v2 devices)
    File::WriteInt32(COUPLER_POWER, 1);
    if (!_ext.Init(COUPLER_TTY))
    {
        qDebug() << "Error initializing coupler tty serial port";
        File::WriteInt32(COUPLER_POWER, 0);
        return false;
    }

    if (!_coupler.Init(&_ext))
    {
        qDebug() << "Error initializing coupler";
        File::WriteInt32(COUPLER_POWER, 0);
        return false;
    }

    if (!waitForReady(90000))
    {
        qDebug() << "Coupler not ready after 90 seconds";
        File::WriteInt32(COUPLER_POWER, 0);
        return false;
    }

    _initialized = true;
    qDebug() << "Card reader initialized successfully";
    return true;
}

void CardReader::shutdown()
{
    if (_initialized)
    {
        qDebug() << "Shutting down coupler...";
        File::WriteInt32(COUPLER_POWER, 0);
        _initialized = false;
    }
}

bool CardReader::waitForReady(unsigned int millisecondsTimeout)
{
    qDebug() << "Waiting for coupler to be ready...";

    uint16 atrLen;
    uchar atr[256];
    sCARD_Search search;
    unsigned char com;
    uint8 options = 0;
    unsigned long long t_end = Time::GetMilliSeconds() + millisecondsTimeout;

    memset(&search, 0, sizeof(search));

    while (Time::GetMilliSeconds() < t_end)
    {
        if (_coupler.SearchCardExt(search, 1, 1, &com, &atrLen, atr, options) == RCSC_Ok)
        {
            qDebug() << "Coupler is ready!";
            return true;
        }
        usleep(20000);
    }

    qDebug() << "Timeout waiting for coupler to be ready";
    return false;
}

QString CardReader::bytesToHex(const uchar *data, int length)
{
    QString hex;
    for (int i = 0; i < length; i++)
    {
        hex += QString("%1").arg(data[i], 2, 16, QChar('0')).toUpper();
    }
    return hex;
}

bool CardReader::authenticateAndRead(Coupler *coupler, const uint8_t *keyA, int sector,
                                     int startBlock, int endBlock, QByteArray &outData)
{
    CouplerMiFARE *mifareCoupler = (CouplerMiFARE *)coupler;
    uchar ucStatus, ucType;
    uchar serialNumber[7];
    int16 result;

    // Load key into reader
    qDebug() << "Loading authentication key...";
    result = mifareCoupler->LoadReaderKeyIndex(0xFF, (uint8 *)keyA, &ucStatus);
    if (result != RCSC_Ok || ucStatus != 0)
    {
        qDebug() << "Failed to load key: result=" << result << ", status=" << ucStatus;
        emit authenticationFailed();
        return false;
    }
    else
    {
        qDebug() << "LoadReaderKeyIndex result:" << result << ", status:" << ucStatus;
        qDebug() << "Loaded Authentication Key...";
    }

    // Authenticate sector
    qDebug() << "Authenticating sector: " << sector << "with key:" << bytesToHex(keyA, 6);
    result = mifareCoupler->Authenticate(sector, 0x0A, 0xFF, &ucType, serialNumber, &ucStatus);
    if (result != RCSC_Ok || ucStatus != 0)
    {
        qDebug() << "Authentication failed: result=" << result << ", status=" << ucStatus;
        emit authenticationFailed();
        return false;
    }

    qDebug() << "Authentication successful!";

    // Read blocks
    qDebug() << "Preparing to reading block" << startBlock << "to" << endBlock;
    for (int block = startBlock; block <= endBlock; block++)
    {
        uchar data[16];
        qDebug() << QString("Reading block %1...").arg(block);

        result = mifareCoupler->ReadBlock(block, data, &ucStatus);
        if (result != RCSC_Ok || ucStatus != 0)
        {
            qDebug() << "Failed to read block" << block << ": result=" << result << ", status=" << ucStatus;
            return false;
        }

        outData.append((char *)data, sizeof(data));
        qDebug() << "Block" << block << ":" << bytesToHex(data, sizeof(data));
    }

    return true;
}

bool CardReader::processMifareClassic(Coupler *coupler, QByteArray &outData)
{
    Config &config = Config::instance();
    return authenticateAndRead(coupler, config.keyA, config.sector,
                               config.startBlock, config.endBlock, outData);
}

bool CardReader::processMifareUL(Coupler *coupler, QByteArray &outData)
{
    CouplerMiFARE *mifareCoupler = (CouplerMiFARE *)coupler;
    uchar data[16], ucStatus;

    // Ultralight doesn't need authentication
    for (int page = 0; page < 16; page += 4)
    {
        emit readProgress(QString("Reading pages %1-%2...").arg(page).arg(page + 3));

        int16 result = mifareCoupler->ReadBlock(page, data, &ucStatus);
        if (result != RCSC_Ok || ucStatus != 0)
        {
            qDebug() << "Failed to read pages" << page << "-" << (page + 3);
            return false;
        }

        outData.append((char *)data, sizeof(data));
        qDebug() << "Pages" << page << "-" << (page + 3) << ":" << bytesToHex(data, sizeof(data));
    }

    return true;
}

CardReader::CardData CardReader::scanCard(unsigned int timeoutSeconds)
{
    CardData result;
    result.success = false;

    qDebug() << "Waiting for card... (timeout:" << timeoutSeconds << "seconds)";
    emit readProgress("Waiting for card...");

    uint64_t t_max = Time::GetMilliSeconds() + timeoutSeconds * 1000;

    while (Time::GetMilliSeconds() < t_max)
    {
        sCARD_Search search;
        unsigned char com;
        uint8 options = SEARCH_OPT_MAX_SPEED;
        unsigned char atr[256];
        uint16 atrLen;

        memset(&search, 0, sizeof(search));
        memset(&search, 0, sizeof(search));

        search.MIFARE = 1;
        search.ISOA = 1;
        search.ISOB = 1;
        search.INNO = 1;
        search.TICK = 1;
        search.SRX = 1;

        if (_coupler.SearchCardExt(search, 1, 100, &com, &atrLen, atr, options) != RCSC_Ok)
            continue;

        if (com == 0x6F)
            continue;

        // Get card UID
        if (atrLen >= 4)
        {
            result.cardUid = bytesToHex(atr, qMin((int)atrLen, 7));
            qDebug() << "Card UID:" << result.cardUid;
        }

        if (com == 5 && atr[1] == 0x08)
        {
            qDebug() << "Found MIFARE Classic 1K card";
            result.cardType = "MIFARE Classic 1K";

            if (processMifareClassic(&_coupler, result.rawData))
            {
                result.success = true;
                emit cardDetected(result.cardType);
                emit scanComplete(true, "Card read successfully");
            }
            else
            {
                result.errorMessage = "Authentication or read failed";
                emit scanComplete(false, result.errorMessage);
            }
            return result;
        }
        else if (com == 5 && atr[1] == 0x09)
        {
            qDebug() << "Found MIFARE Classic 4K card";
            result.cardType = "MIFARE Classic 4K";
            emit cardDetected(result.cardType);

            if (processMifareClassic(&_coupler, result.rawData))
            {
                result.success = true;
                emit scanComplete(true, "Card read successfully");
            }
            else
            {
                result.errorMessage = "Authentication or read failed";
                emit scanComplete(false, result.errorMessage);
            }
            return result;
        }
        else if (com == 5 && atr[1] == 0x04)
        {
            qDebug() << "Found MIFARE Ultralight card";
            result.cardType = "MIFARE Ultralight";
            emit cardDetected(result.cardType);

            if (processMifareUL(&_coupler, result.rawData))
            {
                result.success = true;
                emit scanComplete(true, "Card read successfully");
            }
            else
            {
                result.errorMessage = "Read failed";
                emit scanComplete(false, result.errorMessage);
            }
            return result;
        }

        else if (com == 8)
        {
            qDebug() << "Found ISO14443-4 card (Simulated read)";
            result.cardType = "ISO14443-4";
            emit cardDetected(result.cardType);

            // Simulate some card data
            QByteArray fakeData = QByteArray::fromHex("112233445566778899AABBCCDDEEFF");
            result.rawData = fakeData;
            result.cardUid = "4645454545";

            // Simulate successful read
            result.success = true;
            emit scanComplete(true, "Card read successfully (simulated)");

            return result;
        }

        else if (com == 9)
        {
            qDebug() << "Found ISO15693 card";
            result.cardType = "ISO15693";
            emit cardDetected(result.cardType);

            result.errorMessage = "ISO15693 card processing not implemented";
            emit scanComplete(false, result.errorMessage);
            return result;
        }

        else if (com == 3 && atr[7] == 1)
        {
            qDebug() << "Found Innovatron card";
            result.cardType = "Innovatron";
            emit cardDetected(result.cardType);

            result.errorMessage = "Innovatron card processing not implemented";
            emit scanComplete(false, result.errorMessage);
            return result;
        }

        usleep(100000); // 100ms delay
    }

    _coupler.Reset();
    return result;
}
