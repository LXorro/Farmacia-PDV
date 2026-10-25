-- Escenario: un producto de alta rotación/escasez cuyo stock NO se fragmenta
-- por sucursal, sino que se controla de forma centralizada. Cada venta de
-- este producto en cualquier sucursal debe descontar el stock central de
-- forma atómica junto con el registro local de la venta: o se confirman
-- ambas escrituras (central + sucursal), o ninguna. Esto es exactamente
-- el caso de uso de una transacción distribuida (XA / 2PC).

-- === En CENTRAL (puerto 13306): autoridad del stock compartido ===
CREATE TABLE IF NOT EXISTS farmacia.Stock_Centralizado (
    id_producto         INT PRIMARY KEY,
    nombre               VARCHAR(100) NOT NULL,
    cantidad_disponible  INT NOT NULL
) ENGINE=InnoDB;

INSERT INTO farmacia.Stock_Centralizado (id_producto, nombre, cantidad_disponible)
VALUES (1, 'Antiviral importado (stock único nacional)', 5)
ON DUPLICATE KEY UPDATE cantidad_disponible = VALUES(cantidad_disponible);

-- === En SUCURSAL (puerto 13307): registro local de ventas de ese producto ===
CREATE TABLE IF NOT EXISTS farmacia.Ventas_Centralizadas (
    id_venta_centralizada  INT AUTO_INCREMENT PRIMARY KEY,
    id_producto            INT NOT NULL,
    id_sucursal            INT NOT NULL,
    cantidad                INT NOT NULL,
    fecha                   DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB;
