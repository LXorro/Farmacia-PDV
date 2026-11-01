-- Datos locales de una sucursal (clientes y lotes recibidos). Se ejecuta
-- precedido de "SET @suc = <id_sucursal>;". Los IDs son auto_increment: el
-- desfase por nodo evita que choquen al subir al central.
USE farmacia;

INSERT INTO Clientes (id_sucursal, nombre, apellido_paterno, telefono) VALUES
    (@suc, 'Cliente', CONCAT('Sucursal ', @suc), '555000');

INSERT INTO Lotes (id_producto, id_sucursal, numero_lote, fecha_caducidad, cantidad_inicial, cantidad_disponible) VALUES
    (1, @suc, CONCAT('P-', @suc), DATE_ADD(CURRENT_DATE, INTERVAL 18 MONTH), 100, 100),
    (3, @suc, CONCAT('J-', @suc), DATE_ADD(CURRENT_DATE, INTERVAL 24 MONTH),  50,  50);
