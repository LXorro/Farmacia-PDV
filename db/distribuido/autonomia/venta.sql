-- Una venta como la registra el Punto de Venta: Ticket + Venta + Pago.
-- Requiere @suc, @emp (empleado) y @cant (unidades del producto 1).
USE farmacia;

SET @cliente = (SELECT MIN(id_cliente) FROM Clientes WHERE id_sucursal = @suc);
SET @lote    = (SELECT MIN(id_lote) FROM Lotes WHERE id_sucursal = @suc AND id_producto = 1);

START TRANSACTION;
INSERT INTO Tickets (id_sucursal, id_empleado, id_cliente, modalidad_pago) VALUES (@suc, @emp, @cliente, 'contado');
SET @ticket = LAST_INSERT_ID();
INSERT INTO Ventas (id_ticket, id_lote, cantidad, precio_unitario) VALUES (@ticket, @lote, @cant, 28.00);
INSERT INTO Pagos (id_ticket, monto, metodo_pago) VALUES (@ticket, @cant * 28.00, 'efectivo');
COMMIT;
SELECT @ticket AS id_ticket;
