#include "estadosync.h"
#include "databasemanager.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QStringList>

namespace EstadoSync {

static constexpr int kRetrasoAvisoSegundos = 30;

struct Hilo {
    QString nombre;
    bool corriendo = false;
    int retraso = 0;
    QString error;
};

static Hilo leerHilo(const QSqlRecord &fila) {
    Hilo hilo;
    hilo.nombre = fila.value("Connection_name").toString();
    hilo.corriendo = fila.value("Slave_IO_Running").toString() == "Yes"
                     && fila.value("Slave_SQL_Running").toString() == "Yes";
    hilo.retraso = fila.value("Seconds_Behind_Master").toInt();
    hilo.error = fila.value("Last_IO_Error").toString();
    if (hilo.error.isEmpty())
        hilo.error = fila.value("Last_SQL_Error").toString();
    return hilo;
}

Estado consultar() {
    Estado estado;
    const auto funcion = DatabaseManager::funcion();
    if (funcion == DatabaseManager::Funcion::Unico)
        return estado;

    QSqlQuery query(DatabaseManager::instance().db());
    if (!query.exec("SHOW ALL REPLICAS STATUS")) {
        estado.texto = "Sincronización: sin permiso";
        estado.detalle = query.lastError().text();
        return estado;
    }

    QList<Hilo> hilos;
    while (query.next())
        hilos << leerHilo(query.record());

    if (hilos.isEmpty()) {
        estado.texto = "Sin replicación configurada";
        estado.detalle = "Este equipo no tiene ninguna conexión de replicación.";
        return estado;
    }

    QStringList caidos, detalle;
    int retraso = 0;
    for (const Hilo &hilo : hilos) {
        if (!hilo.corriendo)
            caidos << (hilo.nombre.isEmpty() ? "central" : hilo.nombre);
        retraso = qMax(retraso, hilo.retraso);
        detalle << QString("%1: %2%3").arg(hilo.nombre.isEmpty() ? "central" : hilo.nombre,
                                          hilo.corriendo ? QString("al día (retraso %1 s)").arg(hilo.retraso)
                                                         : "detenida",
                                          hilo.error.isEmpty() || hilo.corriendo ? "" : " — " + hilo.error);
    }
    estado.detalle = detalle.join('\n');

    const bool sucursal = funcion == DatabaseManager::Funcion::Sucursal;
    if (!caidos.isEmpty()) {
        estado.nivel = Nivel::SinConexion;
        estado.texto = sucursal ? "Sin conexión con el central (trabajando en local)"
                                : "Sin conexión con: " + caidos.join(", ");
    } else if (retraso > kRetrasoAvisoSegundos) {
        estado.nivel = Nivel::Retraso;
        estado.texto = QString("Sincronizando (retraso %1 s)").arg(retraso);
    } else {
        estado.nivel = Nivel::Bien;
        estado.texto = "Sincronizado";
    }
    return estado;
}

}
