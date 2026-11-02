-- Migración 002 — precios y costos por sucursal.
--   Productos:  precio_venta -> precio_sugerido, se elimina precio_compra (el costo vive en el lote).
--   Lotes:      + costo_unitario.
--   Nueva tabla Productos_Sucursal (precio, activo y mínimo por sucursal; la escribe la sucursal).
-- Debe aplicarse en TODOS los nodos (central y cada sucursal) y SIN replicarse: cada nodo cambia su propio
-- esquema, y con replicación por filas los esquemas no pueden quedar distintos. Idempotente.
--
--   sudo db/migraciones/aplicar-002.sh nodo       (en cada equipo con la app)
--   sudo db/migraciones/aplicar-002.sh central    (en el central; luego preparar-central.sh)
SET sql_log_bin = 0;
USE farmacia;
SET FOREIGN_KEY_CHECKS = 0;

ALTER TABLE Productos CHANGE COLUMN IF EXISTS precio_venta precio_sugerido DECIMAL(10,2) NOT NULL DEFAULT 0;
ALTER TABLE Productos DROP COLUMN IF EXISTS precio_compra;
ALTER TABLE Lotes ADD COLUMN IF NOT EXISTS costo_unitario DECIMAL(10,2) NOT NULL DEFAULT 0 AFTER cantidad_disponible;

CREATE TABLE IF NOT EXISTS Productos_Sucursal (
    id_producto   INT NOT NULL,
    id_sucursal   INT NOT NULL,
    precio_venta  DECIMAL(10,2) NULL,
    activo        BOOLEAN NOT NULL DEFAULT TRUE,
    stock_minimo  INT NOT NULL DEFAULT 0,
    PRIMARY KEY (id_producto, id_sucursal),
    CONSTRAINT fk_prodsuc_producto FOREIGN KEY (id_producto) REFERENCES Productos(id_producto),
    CONSTRAINT fk_prodsuc_sucursal FOREIGN KEY (id_sucursal) REFERENCES Sucursales(id_sucursal)
) ENGINE=InnoDB;

SET FOREIGN_KEY_CHECKS = 1;

-- Los permisos de la tabla nueva los da aplicar-002.sh (nodo) o preparar-central.sh (central).
