#pragma once

#include <QString>

class SesionActual {
public:
    static SesionActual &instance();

    void iniciar(int idUsuario, const QString &nombreUsuario, const QString &rol,
                 int idEmpleado, int idSucursal, const QString &nombreSucursal);
    void cerrar();

    bool activa() const { return m_idUsuario >= 0; }
    bool esAdmin() const { return m_rol == "admin"; }

    int idUsuario() const { return m_idUsuario; }
    QString nombreUsuario() const { return m_nombreUsuario; }
    QString rol() const { return m_rol; }
    int idEmpleado() const { return m_idEmpleado; }
    int idSucursal() const { return m_idSucursal; }
    QString nombreSucursal() const { return m_nombreSucursal; }

private:
    SesionActual() = default;

    int m_idUsuario = -1;
    QString m_nombreUsuario;
    QString m_rol;
    int m_idEmpleado = -1;
    int m_idSucursal = -1;
    QString m_nombreSucursal;
};
