#pragma once

#include <QSqlDatabase>
#include <QString>

class DatabaseManager {
public:
    static DatabaseManager &instance();

    bool connect();
    QString lastError() const { return m_lastError; }
    QSqlDatabase db() const { return QSqlDatabase::database(); }

private:
    DatabaseManager() = default;
    QString m_lastError;
};
