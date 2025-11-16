#include "download_manager.h"
#include <QFileInfo>
#include <QDir>
#include <QNetworkRequest>
#include <QDebug>
#include <QElapsedTimer>

DownloadManager::DownloadManager(QObject *parent)
    : QObject(parent),
      m_networkManager(new QNetworkAccessManager(this)),
      m_reply(nullptr),
      m_file(nullptr),
      m_isDownloading(false),
      m_bytesReceived(0),
      m_totalBytes(0),
      m_previousBytes(0),
      m_downloadSpeed(0.0),
      m_retryCount(0) {

    m_speedTimer = new QTimer(this);
    connect(m_speedTimer, &QTimer::timeout, this, &DownloadManager::updateSpeed);
}

DownloadManager::~DownloadManager() {
    if (m_isDownloading) {
        cancelDownload();
    }
}

void DownloadManager::startDownload(const QString& url, const QString& destination) {
    if (m_isDownloading) {
        emit downloadError("Download already in progress");
        return;
    }

    m_url = url;
    m_destination = destination;
    m_bytesReceived = 0;
    m_totalBytes = 0;
    m_previousBytes = 0;
    m_retryCount = 0;

    // Create directory if it doesn't exist
    QFileInfo fileInfo(destination);
    QDir dir = fileInfo.dir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    // Check if file already exists (resume capability)
    m_file = new QFile(destination + ".part", this);
    QIODevice::OpenMode mode = QIODevice::WriteOnly | QIODevice::Append;

    if (m_file->exists()) {
        m_bytesReceived = m_file->size();
        qDebug() << "Resuming download from" << m_bytesReceived << "bytes";
    }

    if (!m_file->open(mode)) {
        emit downloadError("Cannot open file for writing: " + destination);
        delete m_file;
        m_file = nullptr;
        return;
    }

    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", "LinuxDroid/1.0");

    // Resume support
    if (m_bytesReceived > 0) {
        QByteArray rangeHeader = "bytes=" + QByteArray::number(m_bytesReceived) + "-";
        request.setRawHeader("Range", rangeHeader);
    }

    m_reply = m_networkManager->get(request);

    connect(m_reply, &QNetworkReply::downloadProgress,
            this, &DownloadManager::onDownloadProgress);
    connect(m_reply, &QNetworkReply::finished,
            this, &DownloadManager::onFinished);
    connect(m_reply, &QNetworkReply::readyRead,
            this, &DownloadManager::onReadyRead);
    connect(m_reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::errorOccurred),
            this, &DownloadManager::onError);

    m_isDownloading = true;
    m_downloadTime.start();
    m_speedTimer->start(1000); // Update speed every second

    qDebug() << "Download started:" << url;
}

void DownloadManager::pauseDownload() {
    if (!m_isDownloading) {
        return;
    }

    if (m_reply) {
        m_reply->abort();
    }

    m_isDownloading = false;
    m_speedTimer->stop();
}

void DownloadManager::resumeDownload() {
    if (m_isDownloading) {
        return;
    }

    startDownload(m_url, m_destination);
}

void DownloadManager::cancelDownload() {
    if (m_reply) {
        m_reply->abort();
        m_reply->deleteLater();
        m_reply = nullptr;
    }

    if (m_file) {
        m_file->close();
        m_file->remove();
        delete m_file;
        m_file = nullptr;
    }

    m_isDownloading = false;
    m_speedTimer->stop();
}

void DownloadManager::onDownloadProgress(qint64 bytesReceived, qint64 totalBytes) {
    m_bytesReceived += bytesReceived;

    if (totalBytes > 0) {
        m_totalBytes = totalBytes + (m_file ? m_file->size() - bytesReceived : 0);
    }

    emit downloadProgress(m_bytesReceived, m_totalBytes);
}

void DownloadManager::onReadyRead() {
    if (m_file) {
        m_file->write(m_reply->readAll());
    }
}

void DownloadManager::onFinished() {
    m_speedTimer->stop();
    m_isDownloading = false;

    if (m_reply->error() != QNetworkReply::NoError) {
        qWarning() << "Download error:" << m_reply->errorString();

        if (m_file) {
            m_file->close();
            delete m_file;
            m_file = nullptr;
        }

        // Retry logic
        if (m_retryCount < MAX_RETRIES) {
            m_retryCount++;
            qDebug() << "Retrying download, attempt" << m_retryCount;
            QTimer::singleShot(2000, this, &DownloadManager::resumeDownload);
        } else {
            emit downloadError("Download failed after " + QString::number(MAX_RETRIES) + " retries");
        }

        m_reply->deleteLater();
        m_reply = nullptr;
        return;
    }

    // Write remaining data
    if (m_file) {
        m_file->write(m_reply->readAll());
        m_file->close();

        // Rename .part file to final name
        QString finalName = m_destination;
        QFile::remove(finalName); // Remove if exists
        m_file->rename(finalName);

        delete m_file;
        m_file = nullptr;
    }

    m_reply->deleteLater();
    m_reply = nullptr;

    qDebug() << "Download completed:" << m_destination;
    emit downloadFinished(m_destination);
}

void DownloadManager::onError(QNetworkReply::NetworkError error) {
    qWarning() << "Network error:" << error << m_reply->errorString();
}

void DownloadManager::updateSpeed() {
    calculateSpeed();
    emit downloadSpeedUpdated(m_downloadSpeed);
}

void DownloadManager::calculateSpeed() {
    qint64 currentBytes = m_bytesReceived;
    qint64 bytesDiff = currentBytes - m_previousBytes;
    m_downloadSpeed = bytesDiff; // Bytes per second
    m_previousBytes = currentBytes;
}

int DownloadManager::progressPercentage() const {
    if (m_totalBytes == 0) {
        return 0;
    }
    return static_cast<int>((m_bytesReceived * 100) / m_totalBytes);
}

QString DownloadManager::estimatedTimeRemaining() const {
    if (m_downloadSpeed <= 0 || m_totalBytes <= 0) {
        return "Calculating...";
    }

    qint64 remainingBytes = m_totalBytes - m_bytesReceived;
    int secondsRemaining = static_cast<int>(remainingBytes / m_downloadSpeed);

    int hours = secondsRemaining / 3600;
    int minutes = (secondsRemaining % 3600) / 60;
    int seconds = secondsRemaining % 60;

    if (hours > 0) {
        return QString("%1h %2m").arg(hours).arg(minutes);
    } else if (minutes > 0) {
        return QString("%1m %2s").arg(minutes).arg(seconds);
    } else {
        return QString("%1s").arg(seconds);
    }
}

void DownloadManager::setExpectedChecksum(const QString& sha256) {
    m_expectedChecksum = sha256;
}

bool DownloadManager::verifyChecksum() {
    if (m_expectedChecksum.isEmpty()) {
        qWarning() << "No expected checksum set";
        return true; // Skip verification if not set
    }

    QFile file(m_destination);
    if (!file.open(QIODevice::ReadOnly)) {
        emit checksumVerified(false);
        return false;
    }

    QCryptographicHash hash(QCryptographicHash::Sha256);
    if (hash.addData(&file)) {
        QString calculatedHash = QString(hash.result().toHex());
        bool matches = (calculatedHash.toLower() == m_expectedChecksum.toLower());

        qDebug() << "Checksum verification:" << (matches ? "SUCCESS" : "FAILED");
        qDebug() << "Expected:" << m_expectedChecksum;
        qDebug() << "Calculated:" << calculatedHash;

        emit checksumVerified(matches);
        return matches;
    }

    emit checksumVerified(false);
    return false;
}
