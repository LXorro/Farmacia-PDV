# Demo de BD distribuidas (fragmentación, replicación, transacción XA)

Entorno autocontenido con dos instancias MariaDB desechables — **no** usa ni
modifica el servicio MariaDB del sistema donde vive la base `farmacia` real
de la app. Vive enteramente en esta carpeta (`central/`, `sucursal/`) y se
puede borrar sin afectar nada más.

- **central** — puerto 13306, `server-id=1`, maestro de replicación.
- **sucursal** — puerto 13307, `server-id=2`, esclavo filtrado.

## Arrancar / apagar

```
./start.sh   # levanta ambas instancias y reinicia la réplica
./stop.sh    # las apaga limpiamente
```

## 1. Fragmentación horizontal (física)

`fragmentacion_demo.sql` crea `Ventas_Particionadas` con
`PARTITION BY LIST (id_sucursal)` sobre una copia desnormalizada de
Ventas+Tickets (sin FK, porque MySQL/MariaDB no permite `FOREIGN KEY` en
tablas particionadas — ver nota en `db/schema.sql`).

```
mysql --socket=central/mysql.sock -u root farmacia < fragmentacion_demo.sql
mysql --socket=central/mysql.sock -u root farmacia -e \
  "EXPLAIN PARTITIONS SELECT * FROM Ventas_Particionadas WHERE id_sucursal = 2;"
```

Verificado: la consulta filtrada por `id_sucursal` solo escanea la
partición correspondiente (`partitions: p_norte`), no las demás — eso es
la poda de particiones. La vista `Ventas_Consolidadas` reconstruye el
agregado global uniendo todas las particiones.

## 2. Replicación maestro → sucursal (catálogo, filtrada)

`sucursal/my.cnf` declara `replicate-do-table` solo para
`Categorias`, `Proveedores`, `Productos`. El resto (Clientes, Tickets,
Ventas, Lotes, etc.) es dato local de cada sucursal y no se replica.

```
mysql --socket=sucursal/mysql.sock -u root -e "SHOW SLAVE STATUS\G"
```

Verificado: un `INSERT` en `Proveedores` en central aparece en sucursal en
~1 segundo; un `INSERT` en `Clientes` en central **no** llega a sucursal
(no está en el filtro).

## 3. Transacción distribuida (XA / 2PC)

Escenario: `Stock_Centralizado` (en central) es la autoridad de inventario
de un producto de alta rotación que **no** se fragmenta por sucursal. Una
venta de ese producto en cualquier sucursal debe descontar el stock central
y registrar la venta local de forma atómica — o ambas escrituras se
confirman, o ninguna.

```
./setup_xa_demo.sql   # crea Stock_Centralizado (central) y Ventas_Centralizadas (sucursal)
./xa_venta.sh <id_producto> <cantidad> <id_sucursal>
```

`xa_venta.sh` hace de coordinador manual: usa el mismo `gtrid` (id global
de transacción) con un `bqual` (calificador de rama) distinto por nodo —
`XA START 'gtrid','central'` / `XA START 'gtrid','sucursal'` — que es el
formato estándar de XA para transacciones multi-recurso. Fases:

1. `XA START` / `XA END` / `XA PREPARE` en **ambos** nodos.
2. Si ambos prepararon: `XA COMMIT` en ambos.
3. Si alguno falló: `XA ROLLBACK` en el/los que sí prepararon.

Verificado con dos casos:
- **Venta válida** (2 de 5 disponibles): ambos nodos preparan, se
  confirma en ambos, stock queda en 3 y la venta aparece en sucursal.
- **Venta que excede el stock** (10 de 3 disponibles): central falla el
  prepare (0 filas afectadas); aunque sucursal sí había preparado, el
  coordinador hace rollback en ambos — stock y tabla de ventas quedan
  exactamente como antes. Así se evita la sobreventa entre sucursales.

### Nota técnica importante (para el reporte)

Al combinar replicación nativa con XA manual entre los mismos dos nodos,
el `XA PREPARE` de central se replica como evento de control (no de fila)
hacia sucursal — por eso, si se usa el mismo xid plano en ambos nodos,
sucursal lo rechaza con `XAER_DUPID` (el xid ya existe, registrado por el
hilo de replicación). Usar `gtrid` compartido + `bqual` distinto por nodo
evita la colisión y es además la forma correcta de nombrar las ramas de
una transacción XA multi-recurso.

## Limpieza de transacciones XA colgadas

Una transacción en estado `PREPARED` es durable y sobrevive la
desconexión del cliente — hay que resolverla explícitamente:

```
mysql --socket=central/mysql.sock -u root -e "XA RECOVER;"
mysql --socket=central/mysql.sock -u root -e "XA ROLLBACK 'gtrid','bqual';"
```
