#include "signature_helper.hpp"
#include <QFile>
#include <QDebug>
#include <openssl/sha.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/x509.h>
#include <cstring>

EVP_PKEY *privateKey;
EVP_PKEY *publicKey;
SignatureHelper::SignatureHelper() : privateKey(nullptr), publicKey(nullptr)
{
}

SignatureHelper::~SignatureHelper()
{
    if (privateKey)
        EVP_PKEY_free(privateKey);
    if (publicKey)
        EVP_PKEY_free(publicKey);
}

EVP_PKEY *SignatureHelper::loadPKCS12(const QString &pfxPath, const QString &password)
{
    QFile file(pfxPath);
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "Failed to open certificate file:" << pfxPath;
        return nullptr;
    }

    QByteArray pfxData = file.readAll();
    file.close();

    BIO *bio = BIO_new_mem_buf(pfxData.data(), pfxData.size());
    PKCS12 *p12 = d2i_PKCS12_bio(bio, nullptr);
    BIO_free(bio);

    if (!p12)
    {
        qDebug() << "Failed to read PKCS12 structure";
        return nullptr;
    }

    EVP_PKEY *pkey = nullptr;
    X509 *cert = nullptr;
    STACK_OF(X509) *ca = nullptr;

    if (!PKCS12_parse(p12, password.toStdString().c_str(), &pkey, &cert, &ca))
    {
        qDebug() << "Failed to parse PKCS12";
        PKCS12_free(p12);
        return nullptr;
    }

    PKCS12_free(p12);
    if (cert)
        X509_free(cert);
    if (ca)
        sk_X509_pop_free(ca, X509_free); // Fixed: use sk_X509_pop_free

    return pkey;
}

EVP_PKEY *SignatureHelper::loadPublicKeyFromPEM(const QString &pemPath)
{
    QFile file(pemPath);
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "Failed to open PEM file:" << pemPath;
        return nullptr;
    }

    QByteArray pemData = file.readAll();
    file.close();

    BIO *bio = BIO_new_mem_buf(pemData.data(), pemData.size());
    X509 *cert = PEM_read_bio_X509(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);

    if (!cert)
    {
        qDebug() << "Failed to read X509 certificate from PEM";
        return nullptr;
    }

    EVP_PKEY *pkey = X509_get_pubkey(cert);
    X509_free(cert);

    if (!pkey)
    {
        qDebug() << "Failed to extract public key from PEM certificate";
        return nullptr;
    }

    return pkey;
}

bool SignatureHelper::loadPrivateCertificate(const QString &pfxPath, const QString &password)
{
    EVP_PKEY *pkey = loadPKCS12(pfxPath, password);
    if (!pkey)
    {
        qDebug() << "Failed to load private certificate";
        return false;
    }
    if (privateKey)
        EVP_PKEY_free(privateKey);
    privateKey = pkey;
    qDebug() << "Private certificate loaded successfully";
    return true;
}

bool SignatureHelper::loadPublicCertificate(const QString &pemPath)
{
    EVP_PKEY *pkey = loadPublicKeyFromPEM(pemPath);
    if (!pkey)
    {
        qDebug() << "Failed to load public certificate";
        return false;
    }

    if (publicKey)
        EVP_PKEY_free(publicKey);

    publicKey = pkey;
    qDebug() << "Public certificate loaded successfully";
    return true;
}

QString SignatureHelper::toBase64(const unsigned char *data, int length)
{
    BIO *b64 = BIO_new(BIO_f_base64());
    BIO *bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);

    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(bio, data, length);
    BIO_flush(bio);

    BUF_MEM *bufferPtr;
    BIO_get_mem_ptr(bio, &bufferPtr);

    QString result = QString::fromLatin1(bufferPtr->data, bufferPtr->length);
    BIO_free_all(bio);

    return result;
}

QByteArray SignatureHelper::fromBase64(const QString &base64)
{
    QByteArray input = base64.toLatin1();

    BIO *b64 = BIO_new(BIO_f_base64());
    BIO *bio = BIO_new_mem_buf(input.data(), input.length());
    bio = BIO_push(b64, bio);

    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);

    QByteArray output;
    output.resize(input.length());

    int decodedLength = BIO_read(bio, output.data(), output.size());
    BIO_free_all(bio);

    output.resize(decodedLength);
    return output;
}

QString SignatureHelper::signData(const QString &data, const QString &algorithm)
{
    if (!privateKey)
    {
        qDebug() << "Private key not loaded";
        return QString();
    }

    // Determine the digest algorithm
    const EVP_MD *md = nullptr;
    if (algorithm.compare("SHA1withRSA", Qt::CaseInsensitive) == 0)
        md = EVP_sha1();
    else if (algorithm.compare("SHA256withRSA", Qt::CaseInsensitive) == 0)
        md = EVP_sha256();
    else
    {
        qDebug() << "Unsupported signing algorithm:" << algorithm;
        return QString();
    }

    QByteArray dataBytes = data.toUtf8();
    unsigned char *sig = nullptr;
    size_t sigLen = 0;

    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    if (!mdctx)
    {
        qDebug() << "EVP_MD_CTX_new failed";
        return QString();
    }

    if (EVP_DigestSignInit(mdctx, nullptr, md, nullptr, privateKey) != 1)
    {
        qDebug() << "EVP_DigestSignInit failed";
        EVP_MD_CTX_free(mdctx);
        return QString();
    }

    if (EVP_DigestSignUpdate(mdctx, dataBytes.data(), dataBytes.size()) != 1)
    {
        qDebug() << "EVP_DigestSignUpdate failed";
        EVP_MD_CTX_free(mdctx);
        return QString();
    }

    // Get required signature length
    if (EVP_DigestSignFinal(mdctx, nullptr, &sigLen) != 1)
    {
        qDebug() << "EVP_DigestSignFinal (length) failed";
        EVP_MD_CTX_free(mdctx);
        return QString();
    }

    sig = (unsigned char *)OPENSSL_malloc(sigLen);
    if (!sig)
    {
        qDebug() << "OPENSSL_malloc failed";
        EVP_MD_CTX_free(mdctx);
        return QString();
    }

    if (EVP_DigestSignFinal(mdctx, sig, &sigLen) != 1)
    {
        qDebug() << "EVP_DigestSignFinal failed";
        OPENSSL_free(sig);
        EVP_MD_CTX_free(mdctx);
        return QString();
    }

    QString signature = toBase64(sig, (int)sigLen);

    OPENSSL_free(sig);
    EVP_MD_CTX_free(mdctx);

    return signature;
}

bool SignatureHelper::verifySignature(const QString &data, const QString &signature)
{
    if (!publicKey)
    {
        qDebug() << "Public key not loaded";
        return false;
    }

    qDebug() << "=== RESPONSE SIGNATURE VERIFICATION DEBUG ===";
    QByteArray dataBytes = data.toUtf8();
    qDebug() << "Data bytes length:" << dataBytes.length();
    qDebug() << "First 100 chars:" << data.left(100);
    qDebug() << "Last 100 chars:" << data.right(100);

    // Clean Base64 signature
    QString cleanSignature = signature;
    cleanSignature.remove('\n').remove('\r').remove(' ');
    QByteArray sigBytes = fromBase64(cleanSignature);
    qDebug() << "Signature bytes length:" << sigBytes.size();

    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    if (!mdctx)
    {
        qDebug() << "EVP_MD_CTX_new failed";
        return false;
    }

    EVP_PKEY_CTX *pctx = nullptr;
    // Use SHA-1 because server signs with SHA1withRSA
    if (EVP_DigestVerifyInit(mdctx, &pctx, EVP_sha1(), nullptr, publicKey) != 1)
    {
        qDebug() << "EVP_DigestVerifyInit failed";
        EVP_MD_CTX_free(mdctx);
        return false;
    }

    // Use PKCS#1 v1.5 padding (default for SHA1withRSA)
    EVP_PKEY_CTX_set_rsa_padding(pctx, RSA_PKCS1_PADDING);

    if (EVP_DigestVerifyUpdate(mdctx, dataBytes.data(), dataBytes.size()) != 1)
    {
        qDebug() << "EVP_DigestVerifyUpdate failed";
        EVP_MD_CTX_free(mdctx);
        return false;
    }

    int ret = EVP_DigestVerifyFinal(mdctx, (const unsigned char *)sigBytes.data(), sigBytes.size());

    if (ret != 1)
    {
        unsigned long err = ERR_get_error();
        char errBuf[256];
        ERR_error_string_n(err, errBuf, sizeof(errBuf));
        qDebug() << "EVP_DigestVerifyFinal failed with return code:" << ret;
        qDebug() << "OpenSSL error:" << errBuf;
    }

    EVP_MD_CTX_free(mdctx);
    qDebug() << "Signature verification result:" << (ret == 1 ? "VALID" : "INVALID");

    return (ret == 1);
}