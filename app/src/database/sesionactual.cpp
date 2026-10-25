#include "sesionactual.h"

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

void SesionActual::cerrar() {
    m_idUsuario = -1;
    m_nombreUsuario.clear();
    m_rol.clear();
    m_idEmpleado = -1;
    m_idSucursal = -1;
    m_nombreSucursal.clear();
}
