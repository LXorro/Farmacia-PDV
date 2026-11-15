#include "sesionactual.h"
#include "databasemanager.h"

#include <QSqlQuery>
#include <QTimeZone>

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

QDateTime SesionActual::vencePermisoTemporal() const {
    if (m_idUsuario < 0)
        return QDateTime();
    QSqlQuery query;
    query.prepare("SELECT MAX(fecha_fin) FROM Permisos_Temporales WHERE id_usuario = :u AND estado = 'vigente' "
                  "AND UTC_TIMESTAMP() BETWEEN fecha_inicio AND fecha_fin");
    query.bindValue(":u", m_idUsuario);
    if (!query.exec() || !query.next() || query.value(0).isNull())
        return QDateTime();
    QDateTime fin = query.value(0).toDateTime();   // guardado en UTC, sin zona
    fin.setTimeZone(QTimeZone::utc());
    return fin.toLocalTime();
}

bool SesionActual::tienePermisoTemporal() const {
    return vencePermisoTemporal().isValid();
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
