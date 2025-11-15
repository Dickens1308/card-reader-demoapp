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
        return false;
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
    qDebug() << "Device:" << Config::instance().deviceName;
    qDebug() << "Station:" << Config::instance().stationCode;
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

    qDebug() << "=== Starting Authentication Sequence ===";
    qDebug() << "Sector:" << sector << "Blocks:" << startBlock << "-" << endBlock;
    qDebug() << "Key A:" << bytesToHex(keyA, 6);

    // Step 1: Request card
    emit readProgress("Requesting card serial...");
    result = mifareCoupler->Request(0x52, &ucType, &ucStatus); // 0x52 = REQA (Request All)
    if (result != RCSC_Ok || ucStatus != 0)
    {
        qDebug() << "Request failed: result=" << result << ", status=" << ucStatus;
        emit authenticationFailed();
        return false;
    }
    qDebug() << "✓ Request successful";

    // Step 2: Get anti-collision data (serial number)
    emit readProgress("Getting card serial...");
    result = mifareCoupler->Anticollision(serialNumber, &ucStatus);
    if (result != RCSC_Ok || ucStatus != 0)
    {
        qDebug() << "Anticollision failed: result=" << result << ", status=" << ucStatus;
        emit authenticationFailed();
        return false;
    }
    qDebug() << "✓ Anticollision successful, Serial:" << bytesToHex(serialNumber, 4);

    // Step 3: Select the card
    result = mifareCoupler->Select(serialNumber, &ucType, &ucStatus);
    if (result != RCSC_Ok || ucStatus != 0)
    {
        qDebug() << "Select failed: result=" << result << ", status=" << ucStatus;
        emit authenticationFailed();
        return false;
    }
    qDebug() << "✓ Select successful";

    // Step 4: Load key into reader
    emit readProgress("Loading authentication key...");
    result = mifareCoupler->LoadReaderKeyIndex(0xFF, (uint8 *)keyA, &ucStatus);
    if (result != RCSC_Ok || ucStatus != 0)
    {
        qDebug() << "Failed to load key: result=" << result << ", status=" << ucStatus;
        emit authenticationFailed();
        return false;
    }
    qDebug() << "✓ Key loaded successfully";

    // Step 5: Authenticate sector with Key A (0x60)
    emit readProgress("Authenticating sector...");
    result = mifareCoupler->Authenticate(sector, 0x60, 0xFF, &ucType, serialNumber, &ucStatus);
    if (result != RCSC_Ok || ucStatus != 0)
    {
        qDebug() << "Authentication failed: result=" << result << ", status=" << ucStatus;
        emit authenticationFailed();
        return false;
    }

    qDebug() << "✓✓✓ Authentication successful! ✓✓✓";

    // Step 6: Read blocks
    for (int block = startBlock; block <= endBlock; block++)
    {
        uchar data[16];
        emit readProgress(QString("Reading block %1...").arg(block));

        result = mifareCoupler->ReadBlock(block, data, &ucStatus);
        if (result != RCSC_Ok || ucStatus != 0)
        {
            qDebug() << "Failed to read block" << block << ": result=" << result << ", status=" << ucStatus;
            return false;
        }

        outData.append((char *)data, sizeof(data));
        qDebug() << "Block" << block << ":" << bytesToHex(data, sizeof(data));
    }

    qDebug() << "=== Read Complete: Total bytes read:" << outData.length() << "===";
    return true;
}

bool CardReader::processMifareClassic(Coupler *coupler, QByteArray &rawData)
{
    qDebug() << "\n=== Processing MIFARE Classic Card ===";

    Config &config = Config::instance();

    // Get configuration values
    const uint8_t *keyA = config.keyA;
    int sector = config.sector;
    int startBlock = config.startBlock;
    int endBlock = config.endBlock;

    // Validate configuration
    qDebug() << "Configuration:";
    qDebug() << "  Key A:" << bytesToHex(keyA, 6);
    qDebug() << "  Sector:" << sector;
    qDebug() << "  Blocks:" << startBlock << "-" << endBlock;

    // Calculate expected data length
    int expectedBlocks = (endBlock - startBlock + 1);
    int expectedBytes = expectedBlocks * 16;
    qDebug() << "  Expected bytes:" << expectedBytes;

    // Validate sector/block ranges
    if (sector < 0 || sector > 15)
    {
        qDebug() << "ERROR: Invalid sector number:" << sector;
        return false;
    }

    int sectorFirstBlock = sector * 4;
    int sectorLastBlock = sectorFirstBlock + 2; // Last data block (trailer is +3)

    if (startBlock < sectorFirstBlock || endBlock > sectorLastBlock)
    {
        qDebug() << "ERROR: Block range" << startBlock << "-" << endBlock
                 << "is invalid for sector" << sector;
        qDebug() << "Valid range is" << sectorFirstBlock << "-" << sectorLastBlock;
        return false;
    }

    // Authenticate and read
    QByteArray cardData;
    if (!authenticateAndRead(coupler, keyA, sector, startBlock, endBlock, cardData))
    {
        qDebug() << "ERROR: Failed to authenticate and read card";
        return false;
    }

    // Verify we got the expected amount of data
    if (cardData.length() != expectedBytes)
    {
        qDebug() << "WARNING: Unexpected data length:" << cardData.length()
                 << "Expected:" << expectedBytes;
    }

    qDebug() << "\n=== Processing Card Data ===";
    qDebug() << "Raw bytes (hex):" << cardData.toHex();
    qDebug() << "Raw bytes (ascii):" << cardData;

    // Remove trailing null bytes (common in MIFARE Classic blocks)
    int originalLength = cardData.length();
    while (cardData.endsWith('\0'))
    {
        cardData.chop(1);
    }

    if (originalLength != cardData.length())
    {
        qDebug() << "Removed" << (originalLength - cardData.length())
                 << "trailing null bytes";
    }

    if (cardData.isEmpty())
    {
        qDebug() << "ERROR: Card data is empty after trimming nulls";
        return false;
    }

    qDebug() << "\n=== Base64 Decoding ===";
    qDebug() << "Base64 data:" << cardData;
    qDebug() << "Base64 length:" << cardData.length() << "bytes";

    // Decode Base64 data
    QByteArray decodedData = QByteArray::fromBase64(cardData);

    if (decodedData.isEmpty())
    {
        qDebug() << "ERROR: Base64 decoding produced empty result";
        qDebug() << "This could mean:";
        qDebug() << "  1. The data on the card is not valid Base64";
        qDebug() << "  2. The wrong sector/blocks were read";
        qDebug() << "  3. The card is empty or corrupted";
        return false;
    }

    qDebug() << "✓ Decoded successfully";
    qDebug() << "Decoded data (hex):" << decodedData.toHex();
    qDebug() << "Decoded data (ascii):" << decodedData;
    qDebug() << "Decoded length:" << decodedData.length() << "bytes";
    qDebug() << "=== Processing Complete ===\n";

    rawData = decodedData;
    return true;
}

bool CardReader::processMifareUL(Coupler *coupler, QByteArray &outData)
{
    CouplerMiFARE *mifareCoupler = (CouplerMiFARE *)coupler;
    uchar data[16], ucStatus;

    qDebug() << "=== Reading MIFARE Ultralight ===";

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

    qDebug() << "=== Ultralight read complete ===";
    return true;
}

CardReader::CardData CardReader::scanCard(unsigned int timeoutSeconds)
{
    CardData result;
    result.success = false;

    Config &config = Config::instance();

    qDebug() << "\n========================================";
    qDebug() << "Waiting for card... (timeout:" << timeoutSeconds << "seconds)";
    qDebug() << "========================================";
    emit readProgress(config.msgScanning);

    uint64_t t_max = Time::GetMilliSeconds() + timeoutSeconds * 1000;

    while (Time::GetMilliSeconds() < t_max)
    {
        sCARD_Search search;
        unsigned char com;
        uint8 options = SEARCH_OPT_MAX_SPEED;
        unsigned char atr[256];
        uint16 atrLen;

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

        // MIFARE Classic 1K
        if (com == 5 && atr[1] == 0x08)
        {
            qDebug() << ">>> Found MIFARE Classic 1K card <<<";
            result.cardType = "MIFARE Classic 1K";
            emit cardDetected(result.cardType);

            if (processMifareClassic(&_coupler, result.rawData))
            {
                result.success = true;
                emit scanComplete(true, config.msgSuccess);
            }
            else
            {
                result.errorMessage = config.msgAuthFailed;
                emit scanComplete(false, result.errorMessage);
            }
            return result;
        }
        // MIFARE Classic 4K
        else if (com == 5 && atr[1] == 0x09)
        {
            qDebug() << ">>> Found MIFARE Classic 4K card <<<";
            result.cardType = "MIFARE Classic 4K";
            emit cardDetected(result.cardType);

            if (processMifareClassic(&_coupler, result.rawData))
            {
                result.success = true;
                emit scanComplete(true, config.msgSuccess);
            }
            else
            {
                result.errorMessage = config.msgAuthFailed;
                emit scanComplete(false, result.errorMessage);
            }
            return result;
        }
        // MIFARE Ultralight
        else if (com == 5 && atr[1] == 0x04)
        {
            qDebug() << ">>> Found MIFARE Ultralight card <<<";
            result.cardType = "MIFARE Ultralight";
            emit cardDetected(result.cardType);

            if (processMifareUL(&_coupler, result.rawData))
            {
                result.success = true;
                emit scanComplete(true, config.msgSuccess);
            }
            else
            {
                result.errorMessage = config.msgReadFailed;
                emit scanComplete(false, result.errorMessage);
            }
            return result;
        }
        // ISO14443-4
        else if (com == 8)
        {
            qDebug() << ">>> Found ISO14443-4 card (Simulated read) <<<";
            result.cardType = "ISO14443-4";
            emit cardDetected(result.cardType);

            QByteArray fakeData = QByteArray::fromHex("112233445566778899AABBCCDDEEFF");
            result.rawData = fakeData;
            result.cardUid = "4645454545";
            result.success = true;
            emit scanComplete(true, "Card read successfully (simulated)");

            return result;
        }
        // ISO15693
        else if (com == 9)
        {
            qDebug() << ">>> Found ISO15693 card <<<";
            result.cardType = "ISO15693";
            emit cardDetected(result.cardType);

            result.errorMessage = "ISO15693 card processing not implemented";
            emit scanComplete(false, result.errorMessage);
            return result;
        }
        // Innovatron
        else if (com == 3 && atr[7] == 1)
        {
            qDebug() << ">>> Found Innovatron card <<<";
            result.cardType = "Innovatron";
            emit cardDetected(result.cardType);

            result.errorMessage = "Innovatron card processing not implemented";
            emit scanComplete(false, result.errorMessage);
            return result;
        }

        usleep(100000); // 100ms delay
    }

    // Timeout reached
    qDebug() << "========================================";
    qDebug() << "TIMEOUT: No card detected";
    qDebug() << "========================================";

    result.errorMessage = "Card scan timeout - no card detected";
    emit scanComplete(false, result.errorMessage);
    _coupler.Reset();

    return result;
}