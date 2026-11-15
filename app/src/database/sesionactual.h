#pragma once

#include <QDateTime>
#include <QString>

class SesionActual {
public:
    static SesionActual &instance();

    void iniciar(int idUsuario, const QString &nombreUsuario, const QString &rol,
                 int idEmpleado, int idSucursal, const QString &nombreSucursal);
    void cerrar();

    bool activa() const { return m_idUsuario >= 0; }
    bool esAdmin() const { return m_rol == "admin"; }
    // Alcance de datos: un admin ve todas las sucursales salvo en el equipo de una sucursal,
    // donde solo existe (y se opera) la propia.
    bool veTodasLasSucursales() const;

    // Permiso temporal «encargado» (1-3 días, lo concede un admin): habilita fijar precios de la
    // sucursal y dar de alta/editar lotes. Se consulta a la base cada vez (vence solo, aunque no
    // haya conexión con el central, porque la tabla ya está replicada en el equipo).
    bool tienePermisoTemporal() const;
    // Hasta cuándo (hora local) dura el permiso temporal vigente; fecha inválida si no hay.
    QDateTime vencePermisoTemporal() const;
    // Un admin siempre puede; un empleado, solo con el permiso temporal vigente.
    bool puedeGestionarSucursal() const;

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
