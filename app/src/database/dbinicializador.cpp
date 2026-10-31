#include "dbinicializador.h"
#include "dbconfig.h"

#include <QFile>
#include <QRandomGenerator>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

namespace DbInicializador {

QStringList partirSentencias(const QString &sql) {
    QStringList sentencias;
    QString delimitador = ";";
    QString actual;

    for (const QString &linea : sql.split('\n')) {
        const QString recortada = linea.trimmed();
        if (recortada.startsWith("--"))
            continue;
        if (actual.trimmed().isEmpty() && recortada.startsWith("DELIMITER ", Qt::CaseInsensitive)) {
            delimitador = recortada.mid(10).trimmed();
            actual.clear();
            continue;
        }

        actual += linea + '\n';
        if (recortada.endsWith(delimitador)) {
            QString sentencia = actual.trimmed();
            sentencia.chop(delimitador.length());
            sentencia = sentencia.trimmed();
            if (!sentencia.isEmpty())
                sentencias << sentencia;
            actual.clear();
        }
    }
    return sentencias;
}

static QString contrasenaAleatoria() {
    const QString alfabeto = "abcdefghijkmnpqrstuvwxyzABCDEFGHJKLMNPQRSTUVWXYZ23456789";
    QString contrasena;
    for (int i = 0; i < 24; ++i)
        contrasena.append(alfabeto.at(QRandomGenerator::system()->bounded(alfabeto.length())));
    return contrasena;
}

static bool ejecutar(QSqlQuery &query, const QString &sql, QString &error) {
    if (query.exec(sql))
        return true;
    error = query.lastError().text();
    return false;
}

Resultado inicializar(const QString &host, int puerto, const QString &adminUsuario, const QString &adminContrasena) {
    Resultado resultado;
    const QString conexion = "farmacia_admin_setup";

    {
        QSqlDatabase admin = QSqlDatabase::addDatabase("QMYSQL", conexion);
        admin.setHostName(host);
        admin.setPort(puerto);
        admin.setUserName(adminUsuario);
        admin.setPassword(adminContrasena);
        if (!admin.open()) {
            resultado.error = admin.lastError().text();
            admin.close();
            QSqlDatabase::removeDatabase(conexion);
            return resultado;
        }

        QSqlQuery query(admin);
        QString error;
        bool ok = true;

        // El script crea la base y usa "USE farmacia"; si ya hay tablas se respeta lo existente.
        ok = ok && ejecutar(query, QString("CREATE DATABASE IF NOT EXISTS %1 CHARACTER SET utf8mb4 "
                                           "COLLATE utf8mb4_unicode_ci").arg(DbConfig::kDatabase), error);
        ok = ok && ejecutar(query, QString("USE %1").arg(DbConfig::kDatabase), error);
        if (ok) {
            query.exec("SHOW TABLES LIKE 'Usuarios'");
            if (!query.next()) {
                QFile archivo(":/db/schema.sql");
                if (!archivo.open(QFile::ReadOnly | QFile::Text)) {
                    error = "No se encontró el esquema embebido (schema.sql).";
                    ok = false;
                } else {
                    for (const QString &sentencia : partirSentencias(QString::fromUtf8(archivo.readAll()))) {
                        if (!ejecutar(query, sentencia, error)) {
                            ok = false;
                            break;
                        }
                    }
                }
            }
        }

        // Usuario de la app: contraseña nueva en cada ejecución (también si el usuario ya existía).
        const QString usuario = DbConfig::kUser;
        const QString contrasena = contrasenaAleatoria();
        const QStringList hostsCuenta = {"localhost", "127.0.0.1"};
        for (const QString &hostCuenta : hostsCuenta) {
            const QString cuenta = QString("'%1'@'%2'").arg(usuario, hostCuenta);
            ok = ok && ejecutar(query, QString("CREATE USER IF NOT EXISTS %1 IDENTIFIED BY '%2'").arg(cuenta, contrasena), error);
            ok = ok && ejecutar(query, QString("ALTER USER %1 IDENTIFIED BY '%2'").arg(cuenta, contrasena), error);
            ok = ok && ejecutar(query, QString("GRANT ALL PRIVILEGES ON %1.* TO %2").arg(DbConfig::kDatabase, cuenta), error);
        }
        ok = ok && ejecutar(query, "FLUSH PRIVILEGES", error);

        if (ok) {
            resultado.ok = true;
            resultado.usuario = usuario;
            resultado.contrasena = contrasena;
        } else {
            resultado.error = error;
        }
        admin.close();
    }
    QSqlDatabase::removeDatabase(conexion);
    return resultado;
}

}
