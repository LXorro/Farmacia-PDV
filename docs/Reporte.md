# Reporte — Sistema de administración de Farmacia con capa distribuida

**Materia:** Bases de Datos Distribuidas
**Proyecto:** Sistema de administración de farmacia (PDV)

## 1. Introducción

Este proyecto implementa un sistema de administración de farmacia
(catálogo, inventario por lotes, ventas, pagos, devoluciones) extendido
con una capa de multi-sucursal, usada como vehículo para aplicar tres
técnicas centrales de bases de datos distribuidas: **fragmentación**,
**replicación** y **transacciones distribuidas**.

## 2. Objetivos

- Diseñar un esquema relacional completo y normalizado para una farmacia
  con manejo de inventario por lote (FEFO) y pago diferido.
- Extender ese esquema con una capa de sucursales que permita razonar
  sobre datos distribuidos geográficamente.
- Implementar y verificar, sobre motor real (MariaDB), los tres mecanismos
  de distribución: fragmentación horizontal, replicación maestro-esclavo
  y transacciones distribuidas (2PC vía XA).
- Construir una aplicación cliente funcional (Qt/C++) que opere sobre el
  esquema, demostrando que el diseño sostiene un flujo real de negocio.

## 3. Alcance

El proyecto se limita a un único sistema (Farmacia). Se evaluó y
descartó extender el alcance a un "PDV genérico" de marca blanca con
migración de datos desde Eleventa (`PDVDATA.FDB`); esos archivos quedan
en el repositorio solo como referencia histórica, sin participar en el
entregable. Ver `Plan_Proyecto.md` para el detalle de esa decisión.

## 4. Diseño de la base de datos

El esquema base (`Farmacia_actualizado.md`) cubre: `Productos`,
`Categorias`, `Proveedores`, `Clientes`, `Empleados`, `Lotes`,
`Inventario`, `Tickets`, `Ventas`, `Pagos`, `Devoluciones` y
`Detalle_Devolucion`. Decisiones clave del diseño original:

- El inventario se controla por **lote** (no por producto), permitiendo
  manejo FEFO (first-expired-first-out) y caducidades independientes
  entre reabastecimientos.
- `Tickets` separa la transacción comercial de sus líneas (`Ventas`),
  permitiendo pago diferido con múltiples abonos (`Pagos`), cada uno con
  su propio método de pago.
- Columnas calculadas (`Tickets.total`, `Tickets.saldo_pendiente`,
  `Ventas.subtotal`) evitan inconsistencia entre el dato almacenado y su
  fórmula.

Ver el diagrama ER completo en `docs/diagrama_er.md`.

### 4.1 Capa de sucursales

Para sostener los tres temas distribuidos, se agregó una tabla
`Sucursales` y una columna `id_sucursal` en las tablas cuyo dato es
intrínsecamente local a una sede física (`Clientes`, `Empleados`,
`Lotes`, `Tickets`, `Devoluciones`). El catálogo (`Productos`,
`Categorias`, `Proveedores`) se mantiene sin esa columna: es un dato
maestro definido una sola vez y replicado de solo lectura hacia cada
sucursal.

### 4.2 Automatización a nivel de base de datos

`db/schema.sql` implementa vía **triggers** las reglas de negocio
descritas en "Funcionamiento general" de `Farmacia_actualizado.md`:

- Al insertar una venta: se descuenta la cantidad del lote, se registra
  el movimiento de salida en `Inventario`, y se resincroniza
  `Tickets.subtotal`.
- Al insertar un pago: se resincroniza `Tickets.monto_pagado` y se
  recalcula `estado_pago` (`pendiente` → `parcial` → `pagado`).
- Al aprobar una devolución con `reintegrar_stock = true`: se regresan
  las unidades al lote de origen y se registra la entrada en
  `Inventario`.

Esto se verificó end-to-end (sección 6).

## 5. Arquitectura distribuida

Ver `docs/diagrama_arquitectura.md` para el diagrama completo. Resumen
de los tres mecanismos:

| Tema | Mecanismo técnico | Qué tablas/datos involucra |
|---|---|---|
| Fragmentación horizontal | `PARTITION BY LIST (id_sucursal)` | Copia desnormalizada de Ventas (`Ventas_Particionadas`) |
| Replicación | Master-slave nativo MariaDB, filtrado con `replicate-do-table` | `Productos`, `Categorias`, `Proveedores` (central → sucursales) |
| Transacción distribuida | XA / 2PC (`XA START/END/PREPARE/COMMIT/ROLLBACK`) | `Stock_Centralizado` (central) + `Ventas_Centralizadas` (sucursal) |

### 5.1 Por qué MySQL/MariaDB no permite fragmentación física + FK simultáneamente

MySQL/MariaDB no permiten `FOREIGN KEY` en tablas particionadas ni que
una tabla particionada sea referenciada por FK desde otra tabla. Por eso
el esquema **operativo** (el que usa la app, en `db/schema.sql`) prioriza
integridad referencial (`id_sucursal` como columna normal, sin
`PARTITION BY`), y la fragmentación física se demuestra aparte, sobre una
copia desnormalizada sin FK (`db/distribuido/fragmentacion_demo.sql`).
Esta es una decisión de diseño documentada, no una limitación no
resuelta.

### 5.2 Un hallazgo técnico: colisión entre replicación y XA

Al implementar la transacción distribuida sobre los mismos dos nodos que
ya tenían replicación activa, se encontró que el `XA PREPARE` del nodo
central se replica hacia la sucursal como evento de control (no de fila),
independientemente de los filtros `replicate-do-table` (que solo aplican
a eventos de fila). Si ambos nodos usan el mismo xid plano para su propia
rama de la transacción distribuida, la sucursal rechaza su propio
`XA START` con `XAER_DUPID` (el xid ya fue registrado por el hilo de
replicación). La solución — que además es la forma estándar de nombrar
transacciones XA multi-recurso — es usar un `gtrid` (id global) compartido
con un `bqual` (calificador de rama) distinto por nodo:
`XA START 'gtrid','central'` vs. `XA START 'gtrid','sucursal'`.

## 6. Pruebas realizadas

### 6.1 Flujo operativo completo (app Qt + esquema real)

Se cargó el esquema y datos de ejemplo en MariaDB, se compiló y ejecutó
la aplicación Qt, y se registró una venta real desde la interfaz
(Punto de Venta → selección de sucursal/empleado/cliente → carrito con
selección de lote por FEFO → cobro). Resultado verificado directamente en
base de datos:

- Ticket creado con `subtotal`/`total` correctos (sincronizados por
  trigger a partir de las líneas de `Ventas`).
- Stock del lote correspondiente descontado correctamente.
- Pago aplicado, `saldo_pendiente = 0`, `estado_pago = 'pagado'`.

### 6.2 Fragmentación

`EXPLAIN PARTITIONS` sobre `Ventas_Particionadas` filtrando por
`id_sucursal = 2` mostró poda de particiones (`partitions: p_norte`
únicamente, no escaneó `p_central`).

### 6.3 Replicación

- Un `INSERT` en `Proveedores` en el nodo central apareció en el nodo
  sucursal en ~1 segundo (`Slave_IO_Running` / `Slave_SQL_Running` = Yes).
- Un `INSERT` en `Clientes` en central (tabla fuera del filtro) **no**
  llegó a sucursal, confirmando que solo el catálogo se replica.

### 6.4 Transacción distribuida (XA)

- **Caso válido**: venta de 2 unidades de un producto con 5 disponibles
  en `Stock_Centralizado`. Ambos nodos prepararon correctamente → se
  confirmó en ambos → stock quedó en 3, venta registrada en sucursal.
- **Caso de sobreventa**: venta de 10 unidades con solo 3 disponibles. El
  `PREPARE` en central falló (0 filas afectadas por la condición
  `cantidad_disponible >= cantidad`); aunque sucursal sí había preparado
  su parte, el coordinador emitió `XA ROLLBACK` en ambos nodos. Verificado
  que ni el stock ni la tabla de ventas de sucursal quedaron con cambios
  parciales — la propiedad atómica de la transacción distribuida evitó la
  sobreventa.

## 7. Conclusiones

El diseño separa claramente qué datos son intrínsecamente locales a una
sucursal (fragmentación), qué datos son maestros y deben verse igual en
todos lados (replicación), y qué recursos son compartidos y requieren
coordinación explícita entre nodos (transacción distribuida). Los tres
mecanismos se implementaron sobre motor real y se verificaron con pruebas
concretas, incluyendo un caso de falla deliberada (sobreventa) para
demostrar que la atomicidad de la transacción distribuida efectivamente
previene inconsistencias entre nodos.

## Anexos

- `Farmacia_actualizado.md` — esquema base aprobado.
- `Plan_Proyecto.md` — historial de decisiones de alcance y arquitectura.
- `db/schema.sql`, `db/seed.sql` — esquema operativo y datos de ejemplo.
- `db/distribuido/` — entorno de demo distribuida (fragmentación,
  replicación, XA) con su propio README.
- `app/` — aplicación cliente Qt6.
- `docs/diagrama_er.md`, `docs/diagrama_arquitectura.md` — diagramas.
