#pragma once

// Valores por defecto de la conexión. El usuario y la contraseña de la app
// no viven aquí: los genera el asistente de primer arranque y se guardan con
// QSettings (ver DatabaseManager).
namespace DbConfig {
constexpr auto kHost     = "127.0.0.1";
constexpr int  kPort     = 3306;
constexpr auto kDatabase = "farmacia";
constexpr auto kUser     = "farmacia_app";
}
