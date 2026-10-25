-- Demostración de fragmentación horizontal física (PARTITION BY).
-- Se ejecuta sobre una copia desnormalizada de Ventas+Tickets, porque
-- MySQL/MariaDB no permiten FOREIGN KEY en tablas particionadas (ver nota
-- en db/schema.sql). El esquema operativo que usa la app NO usa esta
-- tabla; esta es solo para el entregable de fragmentación del curso.

CREATE TABLE IF NOT EXISTS Ventas_Particionadas (
    id_venta         INT NOT NULL,
    id_sucursal      INT NOT NULL,
    id_ticket        INT NOT NULL,
    fecha_venta      DATETIME NOT NULL,
    id_producto      INT NOT NULL,
    cantidad         INT NOT NULL,
    precio_unitario  DECIMAL(10,2) NOT NULL,
    subtotal         DECIMAL(10,2) NOT NULL,
    PRIMARY KEY (id_venta, id_sucursal)  -- la columna de partición debe
                                          -- estar en toda clave única
) ENGINE=InnoDB
  PARTITION BY LIST (id_sucursal) (
    PARTITION p_central  VALUES IN (1),
    PARTITION p_norte    VALUES IN (2)
  );

-- Datos de ejemplo (equivalentes a los que generó la venta real del Ticket #2)
INSERT INTO Ventas_Particionadas VALUES
    (1, 1, 1, '2026-09-27 10:00:00', 1, 5, 28.00, 140.00),
    (2, 1, 2, '2026-09-27 10:15:00', 3, 2, 18.00, 36.00),
    (3, 2, 3, '2026-09-27 11:00:00', 1, 2, 28.00, 56.00),
    (4, 2, 4, '2026-09-27 11:30:00', 4, 1, 110.00, 110.00);

-- Poda de particiones: una consulta filtrada por id_sucursal solo toca la
-- partición de esa sucursal (no escanea las demás). Verificar con EXPLAIN:
--   EXPLAIN PARTITIONS SELECT * FROM Ventas_Particionadas WHERE id_sucursal = 2;
-- La columna `partitions` del resultado debe mostrar únicamente `p_norte`.

-- La vista consolidada en el central reconstruye el total fragmentado:
CREATE OR REPLACE VIEW Ventas_Consolidadas AS
SELECT id_sucursal, COUNT(*) AS num_ventas, SUM(subtotal) AS total_vendido
FROM Ventas_Particionadas
GROUP BY id_sucursal;
