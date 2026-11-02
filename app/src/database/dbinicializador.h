#pragma once

#include <QString>
#include <QStringList>

// Prepara el MariaDB local la primera vez: base, usuario de la app y esquema.
namespace DbInicializador {

struct Resultado {
    bool ok = false;
    QString error;
    QString usuario;      // usuario de la app creado
    QString contrasena;   // contraseña aleatoria generada para ese usuario
};

// Parte un script SQL en sentencias; entiende "DELIMITER" y descarta comentarios "--".
QStringList partirSentencias(const QString &sql);

// Usa una cuenta administradora (p. ej. root) solo durante esta llamada.
// El rol decide qué puede escribir la cuenta de la app (cada tabla tiene un único nodo escritor):
//   Unico    — todo.
//   Sucursal — escribe lo operativo (clientes, lotes, ventas...); solo lee lo que baja del central.
// El central no lo prepara la app: ver db/distribuido/real/preparar-central.sh.
enum class Rol { Unico, Sucursal };

Resultado inicializar(const QString &host, int puerto, const QString &adminUsuario, const QString &adminContrasena,
                      Rol rol = Rol::Unico);

// Tablas que se capturan en el central y bajan a las sucursales / tablas que cada sucursal escribe.
extern const QStringList kTablasDelCentral;
extern const QStringList kTablasDeSucursal;


// ---- Enlace con el central (replicación automática) -----------------------------------------

struct Central {
    QString host;
    int puerto = 3306;
    QString usuario;      // cuenta de registro (kUsuarioRegistro); solo se usa durante el asistente
    QString contrasena;
};

struct SucursalInfo {
    int id = 0;
    QString nombre;
};

// Sucursales activas que el central tiene dadas de alta.
bool listarSucursales(const Central &central, QList<SucursalInfo> &sucursales, QString &error);

// «Ranura» de una sucursal (2..10): de ella salen server-id, gtid_domain_id y auto_increment_offset.
int ranuraDeSucursal(int idSucursal);

// Contenido de my.cnf que el MariaDB local necesita para replicar (MariaDB no deja activar log-bin en caliente).
QString configuracionNodo(int ranura);

// Comprueba que el MariaDB local ya tiene esa configuración. Si falta algo, `instrucciones` dice qué hacer.
bool verificarNodoLocal(const QString &host, int puerto, const QString &adminUsuario, const QString &adminContrasena,
                        int ranura, QString &instrucciones);

// Dirección de este equipo en Tailscale (100.64.0.0/10); vacía si no hay.
QString ipTailscale();

// Deja la replicación andando en ambos sentidos: copia inicial de lo que baja del central, cuentas de
// replicación en cada lado y conexiones (central → sucursal para lo operativo, sucursal → central para el catálogo).
// Idempotente: se puede repetir. Las contraseñas de replicación se generan y no se conservan fuera de MariaDB.
bool enlazarConCentral(const QString &host, int puerto, const QString &adminUsuario, const QString &adminContrasena,
                       const Central &central, int idSucursal, QString &error);

}
