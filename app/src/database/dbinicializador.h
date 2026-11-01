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
    QString contrasenaRemota;  // solo en el central: contraseña de la cuenta de administración remota
};

// Parte un script SQL en sentencias; entiende "DELIMITER" y descarta comentarios "--".
QStringList partirSentencias(const QString &sql);

// Usa una cuenta administradora (p. ej. root) solo durante esta llamada.
// El rol decide qué puede escribir la cuenta de la app (cada tabla tiene un único nodo escritor):
//   Unico    — todo.
//   Central  — escribe usuarios, empleados, sucursales, asignaciones y catálogo; solo lee lo operativo.
//   Sucursal — escribe lo operativo (clientes, lotes, ventas...); solo lee lo que baja del central.
// En el central además crea la cuenta limitada con la que las sucursales administran Usuarios.
enum class Rol { Unico, Central, Sucursal };

Resultado inicializar(const QString &host, int puerto, const QString &adminUsuario, const QString &adminContrasena,
                      Rol rol = Rol::Unico);

// Tablas que se capturan en el central y bajan a las sucursales / tablas que cada sucursal escribe.
extern const QStringList kTablasDelCentral;
extern const QStringList kTablasDeSucursal;

}
