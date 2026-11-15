-- Migración 004 — pagos y reembolsos con tarjeta.
--   Pagos:        + ultimos_digitos (la autorización va en numero_referencia, que ya existía).
--   Devoluciones: tipo_reembolso admite 'tarjeta'; + modalidad_tarjeta y referencia_reembolso.
-- La app no cobra ni reembolsa nada: registra lo que hace la terminal. Un reembolso a tarjeta queda
-- 'solicitada' hasta capturar el folio que imprime la terminal.
-- Se aplica en TODOS los nodos (central y sucursales) y SIN replicarse; con replicación por filas los
-- esquemas no pueden diferir, y ningún nodo debe guardar 'tarjeta' antes de que todos lo admitan. Idempotente.
--
--   sudo db/migraciones/aplicar-004.sh        (en cada equipo y en el central)
SET sql_log_bin = 0;
USE farmacia;

ALTER TABLE Pagos ADD COLUMN IF NOT EXISTS ultimos_digitos CHAR(4) NULL;

ALTER TABLE Devoluciones
    MODIFY COLUMN tipo_reembolso ENUM('efectivo', 'transferencia', 'saldo_a_favor', 'cambio', 'tarjeta') NOT NULL,
    ADD COLUMN IF NOT EXISTS modalidad_tarjeta ENUM('cancelacion', 'devolucion') NULL,
    ADD COLUMN IF NOT EXISTS referencia_reembolso VARCHAR(100) NULL;
