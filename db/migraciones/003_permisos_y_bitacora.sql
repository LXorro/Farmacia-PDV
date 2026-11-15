-- Migración 003 — permisos temporales («encargado», 1-3 días) y bitácora de cambios.
--   Permisos_Temporales: la escribe el central (baja a las sucursales).
--   Bitacora_Cambios:    la escribe la sucursal (sube al central).
-- Se aplica en TODOS los nodos y SIN replicarse (cada nodo crea su propia tabla). Idempotente.
--
--   sudo db/migraciones/aplicar-003.sh nodo       (en cada equipo con la app)
--   sudo db/migraciones/aplicar-003.sh central    (en el central; luego preparar-central.sh)
SET sql_log_bin = 0;
USE farmacia;
SET FOREIGN_KEY_CHECKS = 0;

CREATE TABLE IF NOT EXISTS Permisos_Temporales (
    id_permiso    INT AUTO_INCREMENT PRIMARY KEY,
    id_usuario    INT NOT NULL,
    rol_otorgado  ENUM('encargado') NOT NULL DEFAULT 'encargado',
    fecha_inicio  DATETIME NOT NULL,
    fecha_fin     DATETIME NOT NULL,
    id_autoriza   INT NULL,
    estado        ENUM('vigente', 'revocado') NOT NULL DEFAULT 'vigente',
    INDEX idx_permisos_usuario (id_usuario, estado, fecha_fin),
    CONSTRAINT ck_permisos_duracion CHECK (fecha_fin > fecha_inicio AND TIMESTAMPDIFF(HOUR, fecha_inicio, fecha_fin) <= 72),
    CONSTRAINT fk_permisos_usuario  FOREIGN KEY (id_usuario)  REFERENCES Usuarios(id_usuario),
    CONSTRAINT fk_permisos_autoriza FOREIGN KEY (id_autoriza) REFERENCES Usuarios(id_usuario) ON DELETE SET NULL
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS Bitacora_Cambios (
    id_bitacora     INT AUTO_INCREMENT PRIMARY KEY,
    id_sucursal     INT NOT NULL,
    id_usuario      INT NULL,
    nombre_usuario  VARCHAR(50) NOT NULL,
    fecha           DATETIME NOT NULL,
    accion          VARCHAR(30) NOT NULL,
    entidad         VARCHAR(30) NOT NULL,
    id_registro     VARCHAR(40) NOT NULL,
    detalle         TEXT,
    con_permiso     BOOLEAN NOT NULL DEFAULT FALSE,
    INDEX idx_bitacora_fecha (id_sucursal, fecha),
    CONSTRAINT fk_bitacora_sucursal FOREIGN KEY (id_sucursal) REFERENCES Sucursales(id_sucursal)
) ENGINE=InnoDB;

SET FOREIGN_KEY_CHECKS = 1;
-- Los permisos de las tablas nuevas los da aplicar-003.sh (nodo) o preparar-central.sh (central).
