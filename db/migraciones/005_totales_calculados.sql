-- Migración 005 — Tickets.total y Tickets.saldo_pendiente pasan de «guardadas» (STORED) a «calculadas al
-- leer» (VIRTUAL).
--
-- Problema: un nodo (MariaDB 13) sube sus tickets al central (MariaDB 10.11). El ticket se inserta con subtotal 0
-- y después el trigger actualiza subtotal y monto_pagado; el central NO recalcula las columnas STORED al aplicar
-- esas actualizaciones por filas, y total y saldo_pendiente quedaban en 0.00. Con VIRTUAL no se guardan ni
-- se replican: se calculan en cada lectura, siempre bien. Reproducido con 10.11.14 (central) y 13.0.2 (nodo).
--
-- Se aplica en TODOS los nodos y el central, SIN replicarse. Idempotente (si ya son VIRTUAL no hace nada).
-- Orden recomendado: primero el CENTRAL (corrige al instante lo ya subido), después cada nodo; cualquier
-- orden es seguro (probado en ambos sentidos: la réplica no se rompe).
--
--   sudo db/migraciones/aplicar-005.sh        (en el central y en cada equipo)
SET sql_log_bin = 0;
USE farmacia;

SET @pendiente = (SELECT COUNT(*) FROM information_schema.COLUMNS
                   WHERE TABLE_SCHEMA = 'farmacia' AND TABLE_NAME = 'Tickets'
                     AND COLUMN_NAME IN ('total', 'saldo_pendiente') AND EXTRA LIKE '%STORED%');

-- MariaDB no convierte STORED en VIRTUAL con MODIFY: se quitan y se vuelven a agregar en la MISMA posición
-- (el orden de las columnas debe ser igual en todos los nodos por la replicación por filas).
SET @quitar = IF(@pendiente > 0, 'ALTER TABLE Tickets DROP COLUMN total, DROP COLUMN saldo_pendiente', 'DO 0');
PREPARE paso FROM @quitar; EXECUTE paso; DEALLOCATE PREPARE paso;

SET @agregar = IF(@pendiente > 0,
    'ALTER TABLE Tickets
        ADD COLUMN total DECIMAL(10,2) AS (subtotal - descuento) VIRTUAL AFTER descuento,
        ADD COLUMN saldo_pendiente DECIMAL(10,2) AS (subtotal - descuento - monto_pagado) VIRTUAL AFTER monto_pagado',
    'DO 0');
PREPARE paso FROM @agregar; EXECUTE paso; DEALLOCATE PREPARE paso;
