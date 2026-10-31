#include "databasemanager.h"
#include "dbconfig.h"

#include <QSettings>
#include <QSqlError>
#include <QSqlQuery>

DatabaseManager &DatabaseManager::instance() {
    static DatabaseManager mgr;
    return mgr;
}

DatabaseManager::Resultado DatabaseManager::connect() {
    QSettings ajustes;
    const QString contrasena = ajustes.value("db/contrasena").toString();
    if (contrasena.isEmpty())
        return Resultado::NoConfigurado;

    QSqlDatabase database = QSqlDatabase::contains(QSqlDatabase::defaultConnection)
                                ? QSqlDatabase::database(QSqlDatabase::defaultConnection, false)
                                : QSqlDatabase::addDatabase("QMYSQL");
    if (database.isOpen())
        database.close();
    database.setHostName(ajustes.value("db/host", DbConfig::kHost).toString());
    database.setPort(ajustes.value("db/puerto", DbConfig::kPort).toInt());
    database.setDatabaseName(DbConfig::kDatabase);
    database.setUserName(ajustes.value("db/usuario", DbConfig::kUser).toString());
    database.setPassword(contrasena);

    if (!database.open()) {
        m_lastError = database.lastError().text();
        // 1045/1044: acceso denegado; 1049: la base no existe.
        const QString codigo = database.lastError().nativeErrorCode();
        if (codigo == "1045" || codigo == "1044" || codigo == "1049")
            return Resultado::NoConfigurado;
        return Resultado::ServidorNoDisponible;
    }

    // Conecta pero sin tablas (base creada a medias): también requiere el asistente.
    QSqlQuery tablas("SHOW TABLES LIKE 'Usuarios'");
    if (!tablas.next())
        return Resultado::NoConfigurado;
    return Resultado::Ok;
}

void DatabaseManager::guardarCredenciales(const QString &host, int puerto, const QString &usuario,
                                          const QString &contrasena) {
    QSettings ajustes;
    ajustes.setValue("db/host", host);
    ajustes.setValue("db/puerto", puerto);
    ajustes.setValue("db/usuario", usuario);
    ajustes.setValue("db/contrasena", contrasena);
}
