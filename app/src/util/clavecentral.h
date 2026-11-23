#pragma once

#include <QString>

class QWidget;

// Contraseña de administración del central («farmacia_remoto»): pedirla con la casilla «Recordar 24 h»
// y conservarla hasta 24 horas si el usuario lo pide y la contraseña resultó correcta.
// Se guarda en QSettings ofuscada con el identificador de la máquina (no es cifrado fuerte: evita
// verla a simple vista, pero quien tenga acceso a este equipo y a su configuración podría recuperarla).
namespace ClaveCentral {
// Contraseña recordada y vigente; vacía si no hay o ya venció (en ese caso la borra).
QString recordada();
void recordar(const QString &contrasena);
void olvidar();

// Diálogo con campo de contraseña y casilla. Devuelve false si se cancela.
bool pedir(QWidget *parent, const QString &titulo, const QString &texto, QString &contrasena, bool &recordarClave);
}
