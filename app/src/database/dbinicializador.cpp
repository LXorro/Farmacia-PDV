#include "dbinicializador.h"
#include "dbconfig.h"

#include <QFile>
#include <QRegularExpression>
#include <QRandomGenerator>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

namespace DbInicializador {

const QStringList kTablasDelCentral = {"Sucursales", "Empleados", "Usuarios", "Asignaciones_Temporales",
                                       "Categorias", "Proveedores", "Productos"};
const QStringList kTablasDeSucursal = {"Clientes", "Lotes", "Inventario", "Tickets", "Ventas", "Pagos",
                                       "Devoluciones", "Detalle_Devolucion"};

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

Resultado inicializar(const QString &host, int puerto, const QString &adminUsuario, const QString &adminContrasena,
                      Rol rol) {
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

        // Nada de esto debe viajar por replicación: cada nodo se prepara por su cuenta
        // (si no, el usuario/contraseña de un nodo pisaría el de los demás).
        ok = ok && ejecutar(query, "SET SESSION sql_log_bin = 0", error);

        // El script crea la base y usa "USE farmacia"; si ya hay tablas se respeta lo existente.
        ok = ok && ejecutar(query, QString("CREATE DATABASE IF NOT EXISTS %1 CHARACTER SET utf8mb4 "
                                           "COLLATE utf8mb4_unicode_ci").arg(DbConfig::kDatabase), error);
        ok = ok && ejecutar(query, QString("USE %1").arg(DbConfig::kDatabase), error);
        if (ok) {
            QFile archivo(":/db/schema.sql");
            if (!archivo.open(QFile::ReadOnly | QFile::Text)) {
                error = "No se encontró el esquema embebido (schema.sql).";
                ok = false;
            } else {
                query.exec("SHOW TABLES LIKE 'Usuarios'");
                const bool baseNueva = !query.next();
                static const QRegularExpression crearTabla("^CREATE TABLE\\s+(\\w+)", QRegularExpression::CaseInsensitiveOption);
                for (const QString &sentencia : partirSentencias(QString::fromUtf8(archivo.readAll()))) {
                    // Base nueva: todo el esquema. Base existente: solo las tablas que le falten
                    // (así una versión nueva del esquema no pierde ni toca los datos actuales).
                    if (!baseNueva) {
                        const auto coincidencia = crearTabla.match(sentencia);
                        if (!coincidencia.hasMatch())
                            continue;
                        query.exec(QString("SHOW TABLES LIKE '%1'").arg(coincidencia.captured(1)));
                        if (query.next())
                            continue;
                    }
                    if (!ejecutar(query, sentencia, error)) {
                        ok = false;
                        break;
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
            ok = ok && ejecutar(query, QString("REVOKE ALL PRIVILEGES, GRANT OPTION FROM %1").arg(cuenta), error);
            if (rol == Rol::Unico) {
                ok = ok && ejecutar(query, QString("GRANT ALL PRIVILEGES ON %1.* TO %2").arg(DbConfig::kDatabase, cuenta), error);
                continue;
            }
            const QStringList &escribe = rol == Rol::Central ? kTablasDelCentral : kTablasDeSucursal;
            const QStringList &lee = rol == Rol::Central ? kTablasDeSucursal : kTablasDelCentral;
            for (const QString &tabla : escribe)
                ok = ok && ejecutar(query, QString("GRANT SELECT, INSERT, UPDATE, DELETE ON %1.%2 TO %3")
                                               .arg(DbConfig::kDatabase, tabla, cuenta), error);
            for (const QString &tabla : lee)
                ok = ok && ejecutar(query, QString("GRANT SELECT ON %1.%2 TO %3").arg(DbConfig::kDatabase, tabla, cuenta), error);
        }

        // Cuenta remota (solo central): las sucursales la usan para crear/editar cuentas de la app.
        // No puede tocar nada más que Usuarios; leer Empleados/Sucursales es para armar el formulario.
        QString contrasenaRemota;
        if (rol == Rol::Central) {
            contrasenaRemota = contrasenaAleatoria();
            const QString cuenta = QString("'%1'@'%'").arg(DbConfig::kUsuarioRemoto);
            ok = ok && ejecutar(query, QString("CREATE USER IF NOT EXISTS %1 IDENTIFIED BY '%2'").arg(cuenta, contrasenaRemota), error);
            ok = ok && ejecutar(query, QString("ALTER USER %1 IDENTIFIED BY '%2'").arg(cuenta, contrasenaRemota), error);
            ok = ok && ejecutar(query, QString("REVOKE ALL PRIVILEGES, GRANT OPTION FROM %1").arg(cuenta), error);
            ok = ok && ejecutar(query, QString("GRANT SELECT, INSERT, UPDATE, DELETE ON %1.Usuarios TO %2").arg(DbConfig::kDatabase, cuenta), error);
            ok = ok && ejecutar(query, QString("GRANT SELECT ON %1.Empleados TO %2").arg(DbConfig::kDatabase, cuenta), error);
            ok = ok && ejecutar(query, QString("GRANT SELECT ON %1.Sucursales TO %2").arg(DbConfig::kDatabase, cuenta), error);
        }
        ok = ok && ejecutar(query, "FLUSH PRIVILEGES", error);

        if (ok) {
            resultado.ok = true;
            resultado.usuario = usuario;
            resultado.contrasena = contrasena;
            resultado.contrasenaRemota = contrasenaRemota;
        } else {
            resultado.error = error;
        }
        admin.close();
    }
    QSqlDatabase::removeDatabase(conexion);
    return resultado;
}

}
