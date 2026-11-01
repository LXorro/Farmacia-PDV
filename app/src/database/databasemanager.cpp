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

DatabaseManager::Funcion DatabaseManager::funcion() {
    const QString valor = QSettings().value("nodo/funcion", "unico").toString();
    if (valor == "central")
        return Funcion::Central;
    return valor == "sucursal" ? Funcion::Sucursal : Funcion::Unico;
}

void DatabaseManager::guardarFuncion(Funcion funcion, const QString &hostCentral, int puertoCentral) {
    QSettings ajustes;
    ajustes.setValue("nodo/funcion", funcion == Funcion::Central ? "central"
                                     : funcion == Funcion::Sucursal ? "sucursal" : "unico");
    ajustes.setValue("central/host", hostCentral);
    ajustes.setValue("central/puerto", puertoCentral);
}

int DatabaseManager::idSucursalNodo() {
    return QSettings().value("nodo/id_sucursal", -1).toInt();
}

void DatabaseManager::guardarSucursalNodo(int idSucursal) {
    QSettings().setValue("nodo/id_sucursal", idSucursal);
}

static const char *kConexionCentral = "central";

QSqlDatabase DatabaseManager::dbUsuarios() const {
    return esSucursal() ? QSqlDatabase::database(kConexionCentral, false) : QSqlDatabase::database();
}

bool DatabaseManager::centralConectado() const {
    if (!esSucursal())
        return true;
    QSqlDatabase base = QSqlDatabase::database(kConexionCentral, false);
    if (!base.isValid() || !base.isOpen())
        return false;
    QSqlQuery ping(base);
    return ping.exec("SELECT 1");
}

bool DatabaseManager::conectarCentral(const QString &contrasena) {
    if (!esSucursal())
        return true;
    if (centralConectado())
        return true;

    QSettings ajustes;
    QSqlDatabase base = QSqlDatabase::contains(kConexionCentral) ? QSqlDatabase::database(kConexionCentral, false)
                                                                  : QSqlDatabase::addDatabase("QMYSQL", kConexionCentral);
    if (base.isOpen())
        base.close();
    base.setHostName(ajustes.value("central/host").toString());
    base.setPort(ajustes.value("central/puerto", DbConfig::kPort).toInt());
    base.setDatabaseName(DbConfig::kDatabase);
    base.setUserName(DbConfig::kUsuarioRemoto);
    base.setPassword(contrasena);
    // Sin conexión no debe congelar la ventana.
    base.setConnectOptions("MYSQL_OPT_CONNECT_TIMEOUT=5");
    if (!base.open()) {
        m_lastError = base.lastError().text();
        return false;
    }
    return true;
}
