#pragma once

#include <QSqlDatabase>
#include <QString>

class DatabaseManager {
public:
    enum class Resultado {
        Ok,
        NoConfigurado,          // sin credenciales guardadas, o usuario/base/tablas inexistentes
        ServidorNoDisponible    // MariaDB no responde
    };

    static DatabaseManager &instance();

    Resultado connect();
    // Guarda las credenciales de la app (QSettings) para los siguientes arranques.
    void guardarCredenciales(const QString &host, int puerto, const QString &usuario, const QString &contrasena);
    QString lastError() const { return m_lastError; }
    QSqlDatabase db() const { return QSqlDatabase::database(); }

private:
    DatabaseManager() = default;
    QString m_lastError;
};
