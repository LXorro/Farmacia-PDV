#!/usr/bin/env bash
# Coordinador manual de una transacción distribuida (2PC vía XA) para la
# venta de un producto de stock centralizado (ver setup_xa_demo.sql).
#
# Uso: ./xa_venta.sh <id_producto> <cantidad> <id_sucursal>
#
# Protocolo:
#   1. FASE DE PREPARACIÓN: XA START/END/PREPARE en CENTRAL (descuenta stock,
#      solo si hay suficiente) y en SUCURSAL (registra la venta local).
#   2. DECISIÓN: si ambos PREPARE tuvieron éxito -> XA COMMIT en ambos.
#      Si alguno falló -> XA ROLLBACK en el que sí haya preparado.
#
# Esto demuestra la propiedad atómica de una transacción distribuida: la
# venta del producto centralizado nunca queda registrada en un solo nodo.

set -u

ID_PRODUCTO="${1:?uso: xa_venta.sh <id_producto> <cantidad> <id_sucursal>}"
CANTIDAD="${2:?uso: xa_venta.sh <id_producto> <cantidad> <id_sucursal>}"
ID_SUCURSAL="${3:?uso: xa_venta.sh <id_producto> <cantidad> <id_sucursal>}"

CENTRAL_SOCK=/home/xrr/Projects/PDV/db/distribuido/central/mysql.sock
SUCURSAL_SOCK=/home/xrr/Projects/PDV/db/distribuido/sucursal/mysql.sock
GTRID="venta_$(date +%s%N)"
# XID = gtrid (id global de transacción, compartido) + bqual (calificador de
# rama, único por nodo/resource manager). Es el formato estándar de XA para
# que cada participante tenga su propia rama dentro de la misma transacción
# distribuida — además evita que el PREPARE de un nodo, al replicarse como
# evento de control, choque con el XID usado por el otro nodo.
XID_CENTRAL="'$GTRID','central'"
XID_SUCURSAL="'$GTRID','sucursal'"

echo "== Transacción distribuida (gtrid=$GTRID) =="
echo "Vender $CANTIDAD unidades del producto $ID_PRODUCTO en la sucursal $ID_SUCURSAL"

# --- Fase 1a: preparar en CENTRAL (descuenta stock centralizado) ---
central_out=$(mysql --socket="$CENTRAL_SOCK" -u root farmacia 2>&1 <<SQL
XA START $XID_CENTRAL;
UPDATE Stock_Centralizado
   SET cantidad_disponible = cantidad_disponible - $CANTIDAD
 WHERE id_producto = $ID_PRODUCTO AND cantidad_disponible >= $CANTIDAD;
SELECT ROW_COUNT() AS filas_afectadas;
XA END $XID_CENTRAL;
XA PREPARE $XID_CENTRAL;
SQL
)
central_filas=$(echo "$central_out" | grep -A1 "filas_afectadas" | tail -1)
central_ok=0
if echo "$central_out" | grep -qi "error"; then
    central_ok=1
elif [ "$central_filas" != "1" ]; then
    central_ok=1
fi

echo "--- CENTRAL (prepare) ---"
echo "$central_out"
echo "resultado: $([ $central_ok -eq 0 ] && echo PREPARADO || echo FALLÓ)"

# --- Fase 1b: preparar en SUCURSAL (registra la venta local) ---
sucursal_out=$(mysql --socket="$SUCURSAL_SOCK" -u root farmacia 2>&1 <<SQL
XA START $XID_SUCURSAL;
INSERT INTO Ventas_Centralizadas (id_producto, id_sucursal, cantidad) VALUES ($ID_PRODUCTO, $ID_SUCURSAL, $CANTIDAD);
XA END $XID_SUCURSAL;
XA PREPARE $XID_SUCURSAL;
SQL
)
sucursal_ok=0
if echo "$sucursal_out" | grep -qi "error"; then
    sucursal_ok=1
fi

echo "--- SUCURSAL (prepare) ---"
echo "$sucursal_out"
echo "resultado: $([ $sucursal_ok -eq 0 ] && echo PREPARADO || echo FALLÓ)"

# --- Fase 2: decisión ---
if [ $central_ok -eq 0 ] && [ $sucursal_ok -eq 0 ]; then
    echo "== Ambos nodos prepararon correctamente: XA COMMIT =="
    mysql --socket="$CENTRAL_SOCK" -u root farmacia -e "XA COMMIT $XID_CENTRAL;" 2>&1
    mysql --socket="$SUCURSAL_SOCK" -u root farmacia -e "XA COMMIT $XID_SUCURSAL;" 2>&1
    echo "VENTA CONFIRMADA en ambos nodos."
    exit 0
else
    echo "== Al menos un nodo falló: XA ROLLBACK en el/los que sí prepararon =="
    [ $central_ok -eq 0 ] && mysql --socket="$CENTRAL_SOCK" -u root farmacia -e "XA ROLLBACK $XID_CENTRAL;" 2>&1
    [ $sucursal_ok -eq 0 ] && mysql --socket="$SUCURSAL_SOCK" -u root farmacia -e "XA ROLLBACK $XID_SUCURSAL;" 2>&1
    echo "VENTA CANCELADA en ambos nodos (ninguno quedó con cambios parciales)."
    exit 1
fi
