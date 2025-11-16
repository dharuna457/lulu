#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QSystemTrayIcon>
#include "../core/qemu_manager.h"
#include "../core/vm_config.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onNewInstance();
    void onStartInstance();
    void onStopInstance();
    void onDeleteInstance();
    void onSettings();
    void onAbout();
    void onInstanceSelected();
    void onVMStarted();
    void onVMStopped();
    void onVMError(const QString& error);

private:
    void setupUI();
    void setupMenuBar();
    void setupToolBar();
    void setupStatusBar();
    void setupTrayIcon();
    void loadInstances();
    void refreshInstanceList();

    // UI Components
    QListWidget *m_instanceList;
    QPushButton *m_startButton;
    QPushButton *m_stopButton;
    QPushButton *m_deleteButton;
    QLabel *m_statusLabel;
    QSystemTrayIcon *m_trayIcon;

    // Core
    QemuManager *m_qemuManager;
    QList<VMConfig> m_instances;
    VMConfig *m_currentInstance;
};

#endif // MAIN_WINDOW_H
