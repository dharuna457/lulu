#ifndef VM_CONFIG_H
#define VM_CONFIG_H

#include <QString>
#include <QSize>
#include <QJsonObject>

class VMConfig {
public:
    VMConfig();
    explicit VMConfig(const QString& configPath);

    // Getters
    QString name() const { return m_name; }
    QString imagePath() const { return m_imagePath; }
    QString diskPath() const { return m_diskPath; }
    int cpuCores() const { return m_cpuCores; }
    int ramMB() const { return m_ramMB; }
    QSize resolution() const { return m_resolution; }
    bool rootEnabled() const { return m_rootEnabled; }
    QString instancePath() const { return m_instancePath; }

    // Setters
    void setName(const QString& name) { m_name = name; }
    void setImagePath(const QString& path) { m_imagePath = path; }
    void setDiskPath(const QString& path) { m_diskPath = path; }
    void setCpuCores(int cores) { m_cpuCores = cores; }
    void setRamMB(int mb) { m_ramMB = mb; }
    void setResolution(const QSize& res) { m_resolution = res; }
    void setRootEnabled(bool enabled) { m_rootEnabled = enabled; }
    void setInstancePath(const QString& path) { m_instancePath = path; }

    // Serialization
    bool loadFromFile(const QString& filePath);
    bool saveToFile(const QString& filePath) const;
    QJsonObject toJson() const;
    void fromJson(const QJsonObject& json);

    // Validation
    bool isValid() const;
    QString validationError() const;

    // Defaults
    static VMConfig defaultConfig();
    static int getMaxCpuCores();
    static int getMaxRamMB();

private:
    QString m_name;
    QString m_imagePath;
    QString m_diskPath;
    QString m_instancePath;
    int m_cpuCores;
    int m_ramMB;
    QSize m_resolution;
    bool m_rootEnabled;
    QString m_lastError;
};

#endif // VM_CONFIG_H
