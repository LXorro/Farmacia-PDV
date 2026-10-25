#include "databasemanager.h"
#include "dbconfig.h"

#include <QSqlError>

DatabaseManager &DatabaseManager::instance() {
    static DatabaseManager mgr;
    return mgr;
}

bool DatabaseManager::connect() {
    QSqlDatabase database = QSqlDatabase::addDatabase("QMYSQL");
    database.setHostName(DbConfig::kHost);
    database.setPort(DbConfig::kPort);
    database.setDatabaseName(DbConfig::kDatabase);
    database.setUserName(DbConfig::kUser);
    database.setPassword(DbConfig::kPassword);

    if (!database.open()) {
        m_lastError = database.lastError().text();
        return false;
    }
    return true;
}
