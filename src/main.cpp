#include <QApplication>
#include <QFile>
#include <QMessageBox>
#include <QDebug>
#include "gui/main_window.h"
#include "gui/setup_wizard.h"
#include "utils/system_checker.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    app.setApplicationName("LinuxDroid");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("TripleTech");
    app.setOrganizationDomain("tripletech.com");

    // Check if first run
    bool firstRun = QFile::exists("/opt/linuxdroid/.first_run");

    if (firstRun) {
        qDebug() << "First run detected - showing setup wizard";

        SetupWizard wizard;
        wizard.setWindowTitle("LinuxDroid Setup - First Run");

        if (wizard.exec() == QDialog::Accepted) {
            // Remove first run marker
            QFile::remove("/opt/linuxdroid/.first_run");
            qDebug() << "Setup completed successfully";

            // Launch main window
            MainWindow window;
            window.show();
            return app.exec();
        } else {
            qDebug() << "Setup cancelled by user";
            return 0;
        }
    } else {
        // Normal launch - check if Android image exists
        if (!SystemChecker::hasAndroidImage()) {
            auto reply = QMessageBox::question(
                nullptr,
                "No Android Image",
                "No Android system image found. Would you like to run the setup wizard to download one?",
                QMessageBox::Yes | QMessageBox::No
            );

            if (reply == QMessageBox::Yes) {
                SetupWizard wizard;
                if (wizard.exec() != QDialog::Accepted) {
                    return 0;
                }
            } else {
                QMessageBox::information(
                    nullptr,
                    "Image Required",
                    "LinuxDroid requires an Android system image to run.\n"
                    "You can manually place an Android x86 ISO in /opt/linuxdroid/images/"
                );
                return 0;
            }
        }

        // Launch main window
        MainWindow window;
        window.show();
        return app.exec();
    }
}
