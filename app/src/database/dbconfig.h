#pragma once

// Valores por defecto de la conexión. El usuario y la contraseña de la app
// no viven aquí: los genera el asistente de primer arranque y se guardan con
// QSettings (ver DatabaseManager).
namespace DbConfig {
constexpr auto kHost     = "127.0.0.1";
constexpr int  kPort     = 3306;
constexpr auto kDatabase = "farmacia";
constexpr auto kUser     = "farmacia_app";
// Cuenta del central con la que un admin de una sucursal administra Usuarios (sin guardar su contraseña).
constexpr auto kUsuarioRemoto = "farmacia_remoto";
// Cuenta del central solo para dar de alta una sucursal (replicación); la contraseña se pide en el
// asistente y no se guarda.
constexpr auto kUsuarioRegistro = "farmacia_registro";
// Replicación: el central usa server-id 1 y offset 1; cada sucursal ocupa una «ranura» 2..10 (de ahí
// salen su server-id, su gtid_domain_id y su auto_increment_offset). Con incremento 10 caben 9.
constexpr int kIncrementoAutoinc = 10;
constexpr int kServerIdBase = 10;
}
