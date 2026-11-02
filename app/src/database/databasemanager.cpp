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

static DatabaseManager::Funcion g_modoSesion = DatabaseManager::Funcion::Sucursal;

DatabaseManager::Funcion DatabaseManager::funcion() {
    return hayCentral() ? g_modoSesion : Funcion::Unico;
}

bool DatabaseManager::hayCentral() {
    return !hostCentral().isEmpty();
}

QString DatabaseManager::hostCentral() {
    return QSettings().value("central/host").toString().trimmed();
}

int DatabaseManager::puertoCentral() {
    return QSettings().value("central/puerto", DbConfig::kPort).toInt();
}

void DatabaseManager::guardarCentral(const QString &host, int puerto) {
    QSettings ajustes;
    ajustes.setValue("central/host", host);
    ajustes.setValue("central/puerto", puerto);
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
        return true;  // Único/Central: la conexión por defecto ya es la que cuenta
    QSqlDatabase base = QSqlDatabase::database(kConexionCentral, false);
    if (!base.isValid() || !base.isOpen())
        return false;
    QSqlQuery ping(base);
    return ping.exec("SELECT 1");
}

bool DatabaseManager::conectarCentral(const QString &contrasena) {
    if (!hayCentral() || funcion() == Funcion::Central)
        return true;
    if (centralConectado())
        return true;

    QSettings ajustes;
    QSqlDatabase base = QSqlDatabase::contains(kConexionCentral) ? QSqlDatabase::database(kConexionCentral, false)
                                                                  : QSqlDatabase::addDatabase("QMYSQL", kConexionCentral);
    if (base.isOpen())
        base.close();
    base.setHostName(hostCentral());
    base.setPort(puertoCentral());
    base.setDatabaseName(DbConfig::kDatabase);
    base.setUserName(DbConfig::kUsuarioRemoto);
    base.setPassword(contrasena);
    // Sin conexión no debe congelar la ventana. Va sin TLS a propósito: el central solo acepta
    // conexiones en la interfaz de Tailscale, que ya cifra el tráfico a nivel de red.
    base.setConnectOptions(QStringLiteral("MYSQL_OPT_CONNECT_TIMEOUT=5"));
    if (!base.open()) {
        m_lastError = base.lastError().text();
        return false;
    }
    return true;
}

bool DatabaseManager::activarModoCentral() {
    if (funcion() == Funcion::Central)
        return true;
    if (!hayCentral() || !centralConectado())
        return false;

    // Se conserva la local como "local" por si hay que volver atrás.
    {
        QSqlDatabase local = QSqlDatabase::cloneDatabase(QSqlDatabase::database(), "local");
        if (!local.open()) {
            m_lastError = local.lastError().text();
            return false;
        }
    }
    QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);
    bool ok = false;
    {
        QSqlDatabase nueva = QSqlDatabase::cloneDatabase(QSqlDatabase::database(kConexionCentral, false),
                                                         QSqlDatabase::defaultConnection);
        ok = nueva.open();
        if (!ok)
            m_lastError = nueva.lastError().text();
    }
    if (!ok) {
        QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);
        QSqlDatabase restaurada = QSqlDatabase::cloneDatabase(QSqlDatabase::database("local", false),
                                                              QSqlDatabase::defaultConnection);
        restaurada.open();
        return false;
    }
    g_modoSesion = Funcion::Central;
    return true;
}
