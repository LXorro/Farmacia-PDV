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
Resultado inicializar(const QString &host, int puerto, const QString &adminUsuario, const QString &adminContrasena);

}
