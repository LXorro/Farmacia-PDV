#include "database/databasemanager.h"
#include "database/sesionactual.h"
#include "dialogs/configurarbddialog.h"
#include "dialogs/logindialog.h"
#include "mainwindow.h"
#include "util/iconos.h"

#include <QApplication>
#include <QFile>
#include <QFontDatabase>
#include <QIcon>
#include <QTranslator>
#include <QMessageBox>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("Farmacia");
    QCoreApplication::setApplicationName("FarmaciaApp");
    QFontDatabase::addApplicationFont(":/fonts/SpaceMono-Regular.ttf");
    QFontDatabase::addApplicationFont(":/fonts/SpaceMono-Bold.ttf");
    // Textos estándar de Qt (Guardar/Cancelar, Sí/No, calendario) en español.
    QTranslator traductorQt;
    if (traductorQt.load(":/translations/qtbase_es.qm"))
        app.installTranslator(&traductorQt);

    QApplication::setStyle("Fusion");
    QApplication::setWindowIcon(Iconos::app());

    QFile hojaDeEstilos(":/style.qss");
    if (hojaDeEstilos.open(QFile::ReadOnly | QFile::Text))
        app.setStyleSheet(QString::fromUtf8(hojaDeEstilos.readAll()));

    auto resultado = DatabaseManager::instance().connect();
    if (resultado == DatabaseManager::Resultado::NoConfigurado) {
        ConfigurarBdDialog asistente;
        if (asistente.exec() != QDialog::Accepted)
            return 0;
        resultado = DatabaseManager::instance().connect();
    }
    if (resultado != DatabaseManager::Resultado::Ok) {
        QMessageBox::critical(nullptr, "Error de conexión",
                               "No se pudo conectar a la base de datos:\n" + DatabaseManager::instance().lastError() +
                                   "\n\nVerifica que MariaDB esté corriendo en este equipo.");
        return 1;
    }

    LoginDialog login;
    if (login.exec() != QDialog::Accepted || !SesionActual::instance().activa())
        return 0;

    MainWindow window;
    window.showFullScreen();

    return app.exec();
}
