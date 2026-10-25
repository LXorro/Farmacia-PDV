#include "database/databasemanager.h"
#include "database/sesionactual.h"
#include "dialogs/logindialog.h"
#include "mainwindow.h"
#include "util/iconos.h"

#include <QApplication>
#include <QFile>
#include <QFontDatabase>
#include <QIcon>
#include <QMessageBox>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QFontDatabase::addApplicationFont(":/fonts/SpaceMono-Regular.ttf");
    QFontDatabase::addApplicationFont(":/fonts/SpaceMono-Bold.ttf");
    QApplication::setStyle("Fusion");
    QApplication::setWindowIcon(Iconos::app());

    QFile hojaDeEstilos(":/style.qss");
    if (hojaDeEstilos.open(QFile::ReadOnly | QFile::Text))
        app.setStyleSheet(QString::fromUtf8(hojaDeEstilos.readAll()));

    if (!DatabaseManager::instance().connect()) {
        QMessageBox::critical(nullptr, "Error de conexión",
                               "No se pudo conectar a la base de datos:\n" + DatabaseManager::instance().lastError() +
                                   "\n\nRevisa src/database/dbconfig.h y que MariaDB esté corriendo "
                                   "(ver db/create_user.sql, db/schema.sql, db/seed.sql).");
        return 1;
    }

    LoginDialog login;
    if (login.exec() != QDialog::Accepted || !SesionActual::instance().activa())
        return 0;

    MainWindow window;
    window.showFullScreen();

    return app.exec();
}
