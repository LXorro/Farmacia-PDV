#pragma once

#include <QString>

// SHA-256 + sal aleatoria. No es bcrypt (Qt no lo trae de fábrica y sumar
// una librería externa es más peso del que justifica este proyecto), pero
// evita guardar contraseñas en claro o sin sal.
namespace PasswordUtil {
QString generarSal();
QString hashear(const QString &contrasena, const QString &sal);
bool verificar(const QString &contrasena, const QString &sal, const QString &hashEsperado);
}
