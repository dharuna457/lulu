#ifndef SYSTEM_CHECKER_H
#define SYSTEM_CHECKER_H

#include <QString>
#include <QMap>

class SystemChecker {
public:
    struct SystemInfo {
        bool kvmAvailable;
        bool kvmAccessible;
        bool qemuInstalled;
        int cpuCores;
        qint64 totalRamMB;
        qint64 availableRamMB;
        qint64 diskSpaceGB;
        bool virtualizationEnabled;
        QString cpuModel;
        QString errorMessage;
    };

    static SystemInfo checkSystem();
    static bool checkKVMSupport();
    static bool checkQEMUInstalled();
    static bool checkVirtualizationEnabled();
    static bool hasAndroidImage();
    static QString getAndroidImagePath();
    static bool checkDiskSpace(const QString& path, qint64 requiredGB);
    static int getCPUCores();
    static qint64 getTotalRAM();
    static qint64 getAvailableRAM();
    static QString getCPUModel();

    // Minimum requirements
    static constexpr int MIN_RAM_MB = 4096;      // 4GB
    static constexpr int MIN_DISK_GB = 20;        // 20GB
    static constexpr int MIN_CPU_CORES = 2;

    static bool meetsMinimumRequirements(const SystemInfo& info);
    static QStringList getWarnings(const SystemInfo& info);
};

#endif // SYSTEM_CHECKER_H
