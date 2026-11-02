#include "dbinicializador.h"
#include "dbconfig.h"

#include <QFile>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QRegularExpression>
#include <QRandomGenerator>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>

namespace DbInicializador {

const QStringList kTablasDelCentral = {"Sucursales", "Empleados", "Usuarios", "Asignaciones_Temporales",
                                       "Categorias", "Proveedores", "Productos"};
const QStringList kTablasDeSucursal = {"Clientes", "Lotes", "Inventario", "Tickets", "Ventas", "Pagos",
                                       "Devoluciones", "Detalle_Devolucion", "Productos_Sucursal"};

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
            // Solo para el indicador de sincronización: ver el estado de la replicación, nada más.
            if (rol != Rol::Unico)
                ok = ok && ejecutar(query, QString("GRANT REPLICA MONITOR ON *.* TO %1").arg(cuenta), error);
            if (rol == Rol::Unico) {
                ok = ok && ejecutar(query, QString("GRANT ALL PRIVILEGES ON %1.* TO %2").arg(DbConfig::kDatabase, cuenta), error);
                continue;
            }
            const QStringList &escribe = kTablasDeSucursal;
            const QStringList &lee = kTablasDelCentral;
            for (const QString &tabla : escribe)
                ok = ok && ejecutar(query, QString("GRANT SELECT, INSERT, UPDATE, DELETE ON %1.%2 TO %3")
                                               .arg(DbConfig::kDatabase, tabla, cuenta), error);
            for (const QString &tabla : lee)
                ok = ok && ejecutar(query, QString("GRANT SELECT ON %1.%2 TO %3").arg(DbConfig::kDatabase, tabla, cuenta), error);
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


// ---- Enlace con el central -------------------------------------------------------------------

static QString q(const QString &texto) {   // literal SQL entre comillas simples
    QString t = texto;
    t.replace('\\', "\\\\").replace('\'', "\\'");
    return "'" + t + "'";
}

// Abre una conexión con nombre y la cierra/quita al salir.
struct Conexion {
    QString nombre;
    QSqlDatabase db;
    Conexion(const QString &nombre, const QString &host, int puerto, const QString &usuario, const QString &contrasena,
             QString &error)
        : nombre(nombre) {
        db = QSqlDatabase::addDatabase("QMYSQL", nombre);
        db.setHostName(host);
        db.setPort(puerto);
        db.setUserName(usuario);
        db.setPassword(contrasena);
        db.setConnectOptions("MYSQL_OPT_CONNECT_TIMEOUT=8");
        if (!db.open())
            error = db.lastError().text();
    }
    bool abierta() const { return db.isOpen(); }
    ~Conexion() {
        db.close();
        db = QSqlDatabase();
        QSqlDatabase::removeDatabase(nombre);
    }
};

bool listarSucursales(const Central &central, QList<SucursalInfo> &sucursales, QString &error) {
    Conexion con("farmacia_listar", central.host, central.puerto, central.usuario, central.contrasena, error);
    if (!con.abierta())
        return false;
    QSqlQuery query(con.db);
    if (!query.exec("SELECT id_sucursal, nombre FROM farmacia.Sucursales WHERE rol = 'sucursal' AND estado = 'activa' "
                    "ORDER BY nombre")) {
        error = query.lastError().text();
        return false;
    }
    while (query.next())
        sucursales.append({query.value(0).toInt(), query.value(1).toString()});
    return true;
}

int ranuraDeSucursal(int idSucursal) {
    // Los ids los genera el central con incremento 10 y offset 1 (1, 11, 21...).
    return 2 + (qMax(idSucursal, 1) - 1) / DbConfig::kIncrementoAutoinc % (DbConfig::kIncrementoAutoinc - 1);
}

QString configuracionNodo(int ranura) {
    QString cnf = "[mysqld]\n";
    cnf += QString("server-id=%1\ngtid_domain_id=%1\n").arg(DbConfig::kServerIdBase + ranura);
    cnf += "log-bin=farmacia-bin\nbinlog_format=ROW\n";
    cnf += QString("auto_increment_increment=%1\nauto_increment_offset=%2\n").arg(DbConfig::kIncrementoAutoinc).arg(ranura);
    cnf += "bind-address=0.0.0.0\n";   // el central tiene que alcanzar este equipo (Tailscale)
    for (const QString &tabla : kTablasDelCentral)
        cnf += QString("replicate-do-table=%1.%2\n").arg(DbConfig::kDatabase, tabla);
    return cnf;
}

bool verificarNodoLocal(const QString &host, int puerto, const QString &adminUsuario, const QString &adminContrasena,
                        int ranura, QString &instrucciones) {
    QString error;
    Conexion con("farmacia_verificar", host, puerto, adminUsuario, adminContrasena, error);
    if (!con.abierta()) {
        instrucciones = error;
        return false;
    }
    QSqlQuery query(con.db);
    if (!query.exec("SELECT @@log_bin, @@server_id, @@auto_increment_offset, @@auto_increment_increment, @@binlog_format")
        || !query.next()) {
        instrucciones = query.lastError().text();
        return false;
    }
    const bool listo = query.value(0).toInt() == 1
                       && query.value(1).toInt() == DbConfig::kServerIdBase + ranura
                       && query.value(2).toInt() == ranura
                       && query.value(3).toInt() == DbConfig::kIncrementoAutoinc
                       && query.value(4).toString() == "ROW";
    if (listo)
        return true;
    instrucciones =
        "El MariaDB de este equipo todavía no está preparado para replicar (necesita registro binario, un "
        "server-id propio y escuchar por la red, y MariaDB solo lo acepta en su archivo de configuración).\n\n"
        "Una sola vez, como administrador:\n"
        "  1. Guarda esto en /etc/my.cnf.d/farmacia-nodo.cnf (o /etc/mysql/mariadb.conf.d/ en Debian):\n\n"
        + configuracionNodo(ranura) +
        "\n  2. Reinicia MariaDB (sudo systemctl restart mariadb).\n"
        "  3. Vuelve a pulsar «Configurar».\n\n"
        "(db/distribuido/real/preparar-nodo.sh " + QString::number(ranura) + " hace los pasos 1 y 2.)";
    return false;
}

QString ipTailscale() {
    for (const QHostAddress &direccion : QNetworkInterface::allAddresses()) {
        if (direccion.protocol() != QAbstractSocket::IPv4Protocol)
            continue;
        if (direccion.isInSubnet(QHostAddress("100.64.0.0"), 10))
            return direccion.toString();
    }
    return QString();
}

static bool copiarTabla(QSqlDatabase &origen, QSqlDatabase &destino, const QString &tabla, QString &error) {
    QSqlQuery lectura(origen);
    if (!lectura.exec(QString("SELECT * FROM %1.%2").arg(DbConfig::kDatabase, tabla))) {
        error = tabla + ": " + lectura.lastError().text();
        return false;
    }
    const QSqlRecord registro = lectura.record();
    QStringList columnas, marcas;
    for (int i = 0; i < registro.count(); ++i) {
        columnas << "`" + registro.fieldName(i) + "`";
        marcas << "?";
    }
    QSqlQuery vaciar(destino);
    if (!vaciar.exec(QString("DELETE FROM %1.%2").arg(DbConfig::kDatabase, tabla))) {
        error = tabla + ": " + vaciar.lastError().text();
        return false;
    }
    QSqlQuery insercion(destino);
    insercion.prepare(QString("INSERT INTO %1.%2 (%3) VALUES (%4)")
                          .arg(DbConfig::kDatabase, tabla, columnas.join(','), marcas.join(',')));
    while (lectura.next()) {
        for (int i = 0; i < registro.count(); ++i)
            insercion.bindValue(i, lectura.value(i));
        if (!insercion.exec()) {
            error = tabla + ": " + insercion.lastError().text();
            return false;
        }
    }
    return true;
}

bool enlazarConCentral(const QString &host, int puerto, const QString &adminUsuario, const QString &adminContrasena,
                       const Central &central, int idSucursal, QString &error) {
    const QString miIp = ipTailscale();
    if (miIp.isEmpty()) {
        error = "Este equipo no tiene dirección de Tailscale (100.x.x.x): el central no podría alcanzarlo. "
                "Conéctate a Tailscale e inténtalo de nuevo.";
        return false;
    }
    const QString nombreConexion = QString("sucursal_%1").arg(idSucursal);
    const QString cuentaBaja = QString("repl_s%1").arg(idSucursal);   // en el central: la usa esta sucursal
    const QString cuentaSube = "repl_central";                        // aquí: la usa el central
    const QString hostsRed = "100.64.0.0/255.192.0.0";                // todo el tailnet
    const QString contrasenaBaja = contrasenaAleatoria();
    const QString contrasenaSube = contrasenaAleatoria();

    Conexion cen("farmacia_enlace_central", central.host, central.puerto, central.usuario, central.contrasena, error);
    if (!cen.abierta())
        return false;
    Conexion loc("farmacia_enlace_local", host, puerto, adminUsuario, adminContrasena, error);
    if (!loc.abierta())
        return false;

    QSqlQuery c(cen.db), l(loc.db);
    auto fallar = [&](QSqlQuery &consulta, const QString &sql) {
        error = consulta.lastError().text() + "\n(" + sql.left(80) + ")";
        return false;
    };
    auto ej = [&](QSqlQuery &consulta, const QString &sql) { return consulta.exec(sql) || fallar(consulta, sql); };

    // Nada de esto debe viajar por replicación.
    if (!ej(c, "SET SESSION sql_log_bin = 0") || !ej(l, "SET SESSION sql_log_bin = 0")
        || !ej(l, "SET SESSION FOREIGN_KEY_CHECKS = 0"))
        return false;

    // 1. Cuenta con la que esta sucursal lee el binlog del central.
    const QString cuentaBajaSql = QString("%1@%2").arg(q(cuentaBaja), q(hostsRed));
    if (!ej(c, "CREATE USER IF NOT EXISTS " + cuentaBajaSql + " IDENTIFIED BY " + q(contrasenaBaja))
        || !ej(c, "ALTER USER " + cuentaBajaSql + " IDENTIFIED BY " + q(contrasenaBaja))
        || !ej(c, "GRANT REPLICATION SLAVE ON *.* TO " + cuentaBajaSql))
        return false;

    // 2. Cuenta con la que el central lee el binlog de esta sucursal.
    const QString cuentaSubeSql = QString("%1@%2").arg(q(cuentaSube), q(hostsRed));
    if (!ej(l, "CREATE USER IF NOT EXISTS " + cuentaSubeSql + " IDENTIFIED BY " + q(contrasenaSube))
        || !ej(l, "ALTER USER " + cuentaSubeSql + " IDENTIFIED BY " + q(contrasenaSube))
        || !ej(l, "GRANT REPLICATION SLAVE ON *.* TO " + cuentaSubeSql))
        return false;

    // 3. Copia inicial de lo que baja del central, con la posición exacta del binlog para que la
    //    replicación continúe justo desde ahí (como mysqldump --single-transaction --master-data).
    l.exec("STOP SLAVE");
    l.exec("RESET SLAVE ALL");
    if (!ej(c, "FLUSH TABLES WITH READ LOCK") || !ej(c, "START TRANSACTION WITH CONSISTENT SNAPSHOT"))
        return false;
    QString posicion;
    if (c.exec("SELECT @@gtid_binlog_pos") && c.next())
        posicion = c.value(0).toString();
    c.exec("UNLOCK TABLES");
    for (const QString &tabla : kTablasDelCentral) {
        if (!copiarTabla(cen.db, loc.db, tabla, error)) {
            c.exec("ROLLBACK");
            return false;
        }
    }
    c.exec("COMMIT");

    // 4. Esta sucursal replica desde el central.
    if (!ej(l, "SET GLOBAL gtid_slave_pos = " + q(posicion))
        || !ej(l, QString("CHANGE MASTER TO MASTER_HOST=%1, MASTER_PORT=%2, MASTER_USER=%3, MASTER_PASSWORD=%4, "
                          "MASTER_USE_GTID=slave_pos, MASTER_CONNECT_RETRY=5")
                       .arg(q(central.host)).arg(central.puerto).arg(q(cuentaBaja), q(contrasenaBaja)))
        || !ej(l, "START SLAVE"))
        return false;

    // 5. El central replica desde esta sucursal (conexión con nombre: una por sucursal).
    //    La primera vez arranca desde la posición ACTUAL del binlog local: lo anterior al enlace (p. ej.
    //    un DROP DATABASE hecho a mano) no debe subir al central. Si la conexión ya existía, solo se
    //    actualizan la dirección y la contraseña, conservando por dónde iba.
    bool existe = false;
    if (c.exec("SHOW ALL SLAVES STATUS")) {
        while (c.next())
            existe = existe || c.value("Connection_name").toString() == nombreConexion;
    }
    c.exec(QString("STOP SLAVE %1").arg(q(nombreConexion)));
    QString posicion5;
    if (existe) {
        posicion5 = "";
    } else {
        if (!l.exec("SHOW MASTER STATUS") || !l.next()) {
            error = "El MariaDB local no tiene registro binario (log-bin) activo.";
            return false;
        }
        posicion5 = QString(", MASTER_LOG_FILE=%1, MASTER_LOG_POS=%2")
                        .arg(q(l.value("File").toString())).arg(l.value("Position").toLongLong());
    }
    if (!ej(c, QString("CHANGE MASTER %1 TO MASTER_HOST=%2, MASTER_PORT=%3, MASTER_USER=%4, MASTER_PASSWORD=%5, "
                       "MASTER_CONNECT_RETRY=5%6")
                    .arg(q(nombreConexion), q(miIp)).arg(puerto).arg(q(cuentaSube), q(contrasenaSube), posicion5))
        || !ej(c, QString("START SLAVE %1").arg(q(nombreConexion))))
        return false;

    return true;
}

}
