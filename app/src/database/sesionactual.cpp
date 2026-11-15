#include "sesionactual.h"
#include "databasemanager.h"

#include <QSqlQuery>

SesionActual &SesionActual::instance() {
    static SesionActual sesion;
    return sesion;
}

void SesionActual::iniciar(int idUsuario, const QString &nombreUsuario, const QString &rol,
                            int idEmpleado, int idSucursal, const QString &nombreSucursal) {
    m_idUsuario = idUsuario;
    m_nombreUsuario = nombreUsuario;
    m_rol = rol;
    m_idEmpleado = idEmpleado;
    m_idSucursal = idSucursal;
    m_nombreSucursal = nombreSucursal;
}

bool SesionActual::veTodasLasSucursales() const {
    return esAdmin() && !DatabaseManager::esSucursal();
}

bool SesionActual::tienePermisoTemporal() const {
    if (m_idUsuario < 0)
        return false;
    QSqlQuery query;
    query.prepare("SELECT 1 FROM Permisos_Temporales WHERE id_usuario = :u AND estado = 'vigente' "
                  "AND UTC_TIMESTAMP() BETWEEN fecha_inicio AND fecha_fin LIMIT 1");
    query.bindValue(":u", m_idUsuario);
    return query.exec() && query.next();
}

bool SesionActual::puedeGestionarSucursal() const {
    return esAdmin() || tienePermisoTemporal();
}

void SesionActual::cerrar() {
    m_idUsuario = -1;
    m_nombreUsuario.clear();
    m_rol.clear();
    m_idEmpleado = -1;
    m_idSucursal = -1;
    m_nombreSucursal.clear();
}
