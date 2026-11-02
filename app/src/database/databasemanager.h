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

    // Función de este equipo: una sucursal no escribe Usuarios en su base local (los recibe del
    // central por replicación); los administra conectándose al central.
    enum class Funcion { Unico, Central, Sucursal };
    static Funcion funcion();
    static bool esSucursal() { return funcion() == Funcion::Sucursal; }
    // certificadoCentral: contenido del .pem a confiar para TLS con el central (vacío = sin TLS
    // explícito). Se guarda en QSettings, no en el binario: así se puede cambiar/rotar sin recompilar.
    void guardarFuncion(Funcion funcion, const QString &hostCentral = QString(), int puertoCentral = 0,
                         const QString &certificadoCentral = QString());

    // Sucursal a la que pertenece este equipo (solo en funciones "sucursal"); -1 si aún no se asignó.
    static int idSucursalNodo();
    void guardarSucursalNodo(int idSucursal);

    // Base donde se administran las cuentas: el central (sucursal) o la local (equipo único/central).
    QSqlDatabase dbUsuarios() const;
    // Abre (o comprueba) la conexión al central con la contraseña dada; en un equipo que no es
    // sucursal siempre devuelve true.
    bool conectarCentral(const QString &contrasena);
    bool centralConectado() const;

private:
    DatabaseManager() = default;
    // Vuelca el certificado guardado en QSettings a un archivo real en disco (el driver MySQL
    // necesita una ruta de archivo, no puede leer directo de QSettings); vacío si no hay ninguno.
    static QString rutaCertificadoCentral();
    QString m_lastError;
};
