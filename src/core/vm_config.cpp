#include "vm_config.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QThread>
#include <QSysInfo>
#include <QStorageInfo>
#include <QDir>
#include <QRegularExpression>

VMConfig::VMConfig()
    : m_cpuCores(2),
      m_ramMB(4096),
      m_resolution(1920, 1080),
      m_rootEnabled(false) {
}

VMConfig::VMConfig(const QString& configPath) : VMConfig() {
    loadFromFile(configPath);
}

bool VMConfig::loadFromFile(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        m_lastError = "Cannot open config file: " + filePath;
        return false;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (doc.isNull() || !doc.isObject()) {
        m_lastError = "Invalid JSON in config file";
        return false;
    }

    fromJson(doc.object());
    return true;
}

bool VMConfig::saveToFile(const QString& filePath) const {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    QJsonDocument doc(toJson());
    file.write(doc.toJson(QJsonDocument::Indented));
    return true;
}

QJsonObject VMConfig::toJson() const {
    QJsonObject json;
    json["name"] = m_name;
    json["imagePath"] = m_imagePath;
    json["diskPath"] = m_diskPath;
    json["instancePath"] = m_instancePath;
    json["cpuCores"] = m_cpuCores;
    json["ramMB"] = m_ramMB;
    json["resolutionWidth"] = m_resolution.width();
    json["resolutionHeight"] = m_resolution.height();
    json["rootEnabled"] = m_rootEnabled;
    return json;
}

void VMConfig::fromJson(const QJsonObject& json) {
    m_name = json["name"].toString();
    m_imagePath = json["imagePath"].toString();
    m_diskPath = json["diskPath"].toString();
    m_instancePath = json["instancePath"].toString();
    m_cpuCores = json["cpuCores"].toInt(2);
    m_ramMB = json["ramMB"].toInt(4096);

    int width = json["resolutionWidth"].toInt(1920);
    int height = json["resolutionHeight"].toInt(1080);
    m_resolution = QSize(width, height);

    m_rootEnabled = json["rootEnabled"].toBool(false);
}

bool VMConfig::isValid() const {
    if (m_name.isEmpty()) {
        return false;
    }

    if (!QFile::exists(m_imagePath)) {
        return false;
    }

    if (m_cpuCores < 1 || m_cpuCores > getMaxCpuCores()) {
        return false;
    }

    if (m_ramMB < 512 || m_ramMB > getMaxRamMB()) {
        return false;
    }

    return true;
}

QString VMConfig::validationError() const {
    if (m_name.isEmpty()) {
        return "Instance name is required";
    }

    if (!QFile::exists(m_imagePath)) {
        return "Android image not found: " + m_imagePath;
    }

    if (m_cpuCores < 1) {
        return "At least 1 CPU core required";
    }

    if (m_ramMB < 512) {
        return "At least 512MB RAM required";
    }

    return QString();
}

VMConfig VMConfig::defaultConfig() {
    VMConfig config;
    config.setName("My Android");
    config.setCpuCores(qMin(4, getMaxCpuCores()));
    config.setRamMB(4096);
    config.setResolution(QSize(1920, 1080));
    config.setRootEnabled(false);
    return config;
}

int VMConfig::getMaxCpuCores() {
    return QThread::idealThreadCount();
}

int VMConfig::getMaxRamMB() {
    // Get total system RAM and reserve 2GB for system
    QFile meminfo("/proc/meminfo");
    if (meminfo.open(QIODevice::ReadOnly)) {
        QString content = QString::fromUtf8(meminfo.readAll());
        QStringList lines = content.split('\n');
        for (const QString& line : lines) {
            if (line.startsWith("MemTotal:")) {
                QStringList parts = line.split(QRegularExpression("\\s+"));
                if (parts.size() >= 2) {
                    qint64 totalKB = parts[1].toLongLong();
                    qint64 maxMB = (totalKB / 1024) - 2048; // Reserve 2GB
                    return qMax(2048LL, maxMB); // Minimum 2GB
                }
            }
        }
    }

    return 8192; // Default to 8GB if can't detect
}
