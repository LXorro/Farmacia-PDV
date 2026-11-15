#include "bitacora.h"
#include "../database/sesionactual.h"

#include <QSqlError>
#include <QSqlQuery>

namespace Bitacora {

bool registrar(const QString &accion, const QString &entidad, const QString &idRegistro, const QString &detalle,
               QString &error) {
    const SesionActual &sesion = SesionActual::instance();
    QSqlQuery query;
    query.prepare("INSERT INTO Bitacora_Cambios (id_sucursal, id_usuario, nombre_usuario, fecha, accion, entidad, "
                  "id_registro, detalle, con_permiso) "
                  "VALUES (:suc, :usr, :nombre, UTC_TIMESTAMP(), :accion, :entidad, :reg, :detalle, :permiso)");
    query.bindValue(":suc", sesion.idSucursal());
    query.bindValue(":usr", sesion.idUsuario());
    query.bindValue(":nombre", sesion.nombreUsuario());
    query.bindValue(":accion", accion);
    query.bindValue(":entidad", entidad);
    query.bindValue(":reg", idRegistro);
    query.bindValue(":detalle", detalle);
    // «Con permiso» = lo hizo un empleado amparado por un permiso temporal (un admin no lo necesita).
    query.bindValue(":permiso", !sesion.esAdmin());
    if (!query.exec()) {
        error = query.lastError().text();
        return false;
    }
    return true;
}

}
