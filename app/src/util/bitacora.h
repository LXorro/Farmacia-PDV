#pragma once

#include <QString>

// Registro de cambios hechos en una sucursal (precios, lotes): quién, cuándo y bajo qué autorización.
// Va en la misma transacción que el cambio: si la bitácora falla, el cambio no se guarda.
namespace Bitacora {
// Devuelve false (con `error`) si no pudo registrar. Usa la conexión por defecto.
bool registrar(const QString &accion, const QString &entidad, const QString &idRegistro, const QString &detalle,
               QString &error);
}
