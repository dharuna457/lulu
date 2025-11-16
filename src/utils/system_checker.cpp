#include "system_checker.h"
#include <QFile>
#include <QDir>
#include <QProcess>
#include <QStorageInfo>
#include <QDebug>
#include <QRegularExpression>

SystemChecker::SystemInfo SystemChecker::checkSystem() {
    SystemInfo info;

    info.kvmAvailable = checkKVMSupport();
    info.kvmAccessible = QFile::exists("/dev/kvm");
    info.qemuInstalled = checkQEMUInstalled();
    info.cpuCores = getCPUCores();
    info.totalRamMB = getTotalRAM();
    info.availableRamMB = getAvailableRAM();
    info.virtualizationEnabled = checkVirtualizationEnabled();
    info.cpuModel = getCPUModel();

    QStorageInfo storage("/opt/linuxdroid");
    info.diskSpaceGB = storage.bytesAvailable() / (1024 * 1024 * 1024);

    return info;
}

bool SystemChecker::checkKVMSupport() {
    QProcess process;
    process.start("kvm-ok");
    process.waitForFinished();

    QString output = QString::fromUtf8(process.readAllStandardOutput());
    QString error = QString::fromUtf8(process.readAllStandardError());

    return output.contains("KVM acceleration can be used") ||
           QFile::exists("/dev/kvm");
}

bool SystemChecker::checkQEMUInstalled() {
    QProcess process;
    process.start("which", QStringList() << "qemu-system-x86_64");
    process.waitForFinished();
    return process.exitCode() == 0;
}

bool SystemChecker::checkVirtualizationEnabled() {
    QFile cpuinfo("/proc/cpuinfo");
    if (!cpuinfo.open(QIODevice::ReadOnly)) {
        return false;
    }

    QString content = QString::fromUtf8(cpuinfo.readAll());
    return content.contains("vmx") || content.contains("svm");
}

bool SystemChecker::hasAndroidImage() {
    QDir imageDir("/opt/linuxdroid/images");
    if (!imageDir.exists()) {
        return false;
    }

    QStringList filters;
    filters << "*.iso" << "*.img";
    QStringList images = imageDir.entryList(filters, QDir::Files);

    return !images.isEmpty();
}

QString SystemChecker::getAndroidImagePath() {
    QDir imageDir("/opt/linuxdroid/images");
    if (!imageDir.exists()) {
        return QString();
    }

    QStringList filters;
    filters << "*.iso" << "*.img";
    QStringList images = imageDir.entryList(filters, QDir::Files);

    if (images.isEmpty()) {
        return QString();
    }

    return imageDir.absoluteFilePath(images.first());
}

bool SystemChecker::checkDiskSpace(const QString& path, qint64 requiredGB) {
    QStorageInfo storage(path);
    qint64 availableGB = storage.bytesAvailable() / (1024 * 1024 * 1024);
    return availableGB >= requiredGB;
}

int SystemChecker::getCPUCores() {
    QFile cpuinfo("/proc/cpuinfo");
    if (!cpuinfo.open(QIODevice::ReadOnly)) {
        return 1;
    }

    QString content = QString::fromUtf8(cpuinfo.readAll());
    int count = content.count("processor");

    return qMax(1, count);
}

qint64 SystemChecker::getTotalRAM() {
    QFile meminfo("/proc/meminfo");
    if (!meminfo.open(QIODevice::ReadOnly)) {
        return 0;
    }

    QString content = QString::fromUtf8(meminfo.readAll());
    QStringList lines = content.split('\n');

    for (const QString& line : lines) {
        if (line.startsWith("MemTotal:")) {
            QRegularExpression re("\\d+");
            QRegularExpressionMatch match = re.match(line);
            if (match.hasMatch()) {
                qint64 kb = match.captured(0).toLongLong();
                return kb / 1024; // Convert to MB
            }
        }
    }

    return 0;
}

qint64 SystemChecker::getAvailableRAM() {
    QFile meminfo("/proc/meminfo");
    if (!meminfo.open(QIODevice::ReadOnly)) {
        return 0;
    }

    QString content = QString::fromUtf8(meminfo.readAll());
    QStringList lines = content.split('\n');

    for (const QString& line : lines) {
        if (line.startsWith("MemAvailable:")) {
            QRegularExpression re("\\d+");
            QRegularExpressionMatch match = re.match(line);
            if (match.hasMatch()) {
                qint64 kb = match.captured(0).toLongLong();
                return kb / 1024; // Convert to MB
            }
        }
    }

    return 0;
}

QString SystemChecker::getCPUModel() {
    QFile cpuinfo("/proc/cpuinfo");
    if (!cpuinfo.open(QIODevice::ReadOnly)) {
        return "Unknown";
    }

    QString content = QString::fromUtf8(cpuinfo.readAll());
    QStringList lines = content.split('\n');

    for (const QString& line : lines) {
        if (line.startsWith("model name")) {
            QStringList parts = line.split(':');
            if (parts.size() >= 2) {
                return parts[1].trimmed();
            }
        }
    }

    return "Unknown";
}

bool SystemChecker::meetsMinimumRequirements(const SystemInfo& info) {
    return info.cpuCores >= MIN_CPU_CORES &&
           info.totalRamMB >= MIN_RAM_MB &&
           info.diskSpaceGB >= MIN_DISK_GB &&
           info.qemuInstalled;
}

QStringList SystemChecker::getWarnings(const SystemInfo& info) {
    QStringList warnings;

    if (!info.qemuInstalled) {
        warnings << "QEMU is not installed. Please install qemu-system-x86.";
    }

    if (!info.kvmAvailable || !info.virtualizationEnabled) {
        warnings << "KVM virtualization is not available. Enable virtualization in BIOS for better performance.";
    }

    if (info.totalRamMB < MIN_RAM_MB) {
        warnings << QString("Insufficient RAM: %1 MB available, %2 MB required.")
                       .arg(info.totalRamMB).arg(MIN_RAM_MB);
    }

    if (info.cpuCores < MIN_CPU_CORES) {
        warnings << QString("Insufficient CPU cores: %1 available, %2 required.")
                       .arg(info.cpuCores).arg(MIN_CPU_CORES);
    }

    if (info.diskSpaceGB < MIN_DISK_GB) {
        warnings << QString("Insufficient disk space: %1 GB available, %2 GB required.")
                       .arg(info.diskSpaceGB).arg(MIN_DISK_GB);
    }

    return warnings;
}
