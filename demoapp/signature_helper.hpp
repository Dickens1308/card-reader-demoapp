#ifndef SIGNATURE_HELPER_HPP
#define SIGNATURE_HELPER_HPP

#include <QString>
#include <QByteArray>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/pkcs12.h>

class SignatureHelper
{
public:
    SignatureHelper();
    ~SignatureHelper();

    bool loadPrivateCertificate(const QString &pfxPath, const QString &password);
    bool loadPublicCertificate(const QString &pfxPath);

    QString signData(const QString &data, const QString &algorithm);
    bool verifySignature(const QString &data, const QString &signature);

private:
    EVP_PKEY *privateKey;
    EVP_PKEY *publicKey;

    EVP_PKEY *loadPKCS12(const QString &pfxPath, const QString &password);
    EVP_PKEY *loadPublicKeyFromPEM(const QString &pemPath);
    QString toBase64(const unsigned char *data, int length);
    QByteArray fromBase64(const QString &base64);
};

#endif // SIGNATURE_HELPER_HPP