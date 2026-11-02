#pragma once

#include <QSqlDatabase>
#include <QString>

class DatabaseManager {
public:
    enum class Resultado {
        Ok,
        NoConfigurado,          // sin credenciales guardadas, o usuario/base/tablas inexistentes
        ServidorNoDisponible    // MariaDB no responde
    };

    static DatabaseManager &instance();

    Resultado connect();
    // Guarda las credenciales de la app (QSettings) para los siguientes arranques.
    void guardarCredenciales(const QString &host, int puerto, const QString &usuario, const QString &contrasena);
    QString lastError() const { return m_lastError; }
    QSqlDatabase db() const { return QSqlDatabase::database(); }

    // Modo de la sesión (ya no es una propiedad del equipo):
    //   Unico    — equipo sin central (una sola base local).
    //   Sucursal — base local: empleados, y administradores mientras no haya conexión con el central.
    //   Central  — un administrador con el central a la vista: la conexión por defecto pasa a ser la
    //              del central (ve todas las sucursales y administra catálogo, cuentas y personal).
    enum class Funcion { Unico, Central, Sucursal };
    static Funcion funcion();
    static bool esSucursal() { return funcion() == Funcion::Sucursal; }

    // Central al que este equipo está enlazado (lo guarda el asistente); sin él, equipo único.
    static bool hayCentral();
    static QString hostCentral();
    static int puertoCentral();
    void guardarCentral(const QString &host, int puerto);

    // Sucursal a la que pertenece este equipo (solo en funciones "sucursal"); -1 si aún no se asignó.
    static int idSucursalNodo();
    void guardarSucursalNodo(int idSucursal);

    // Base donde se administran las cuentas: el central (sucursal) o la conexión por defecto
    // (equipo único, o sesión de administración ya conectada al central).
    QSqlDatabase dbUsuarios() const;
    // Abre (o comprueba) la conexión al central con la contraseña de la cuenta de administración
    // remota. Sin central configurado, o ya en modo Central, devuelve true.
    bool conectarCentral(const QString &contrasena);
    bool centralConectado() const;
    // Con la conexión al central abierta, la convierte en la conexión por defecto de la sesión
    // (modo Central). Debe llamarse sin consultas abiertas ni páginas creadas.
    bool activarModoCentral();

private:
    DatabaseManager() = default;
    QString m_lastError;
};
