# Base de Datos: Farmacia (esquema actualizado)

## Tabla: Productos

| Campo | Tipo de dato | Descripción |
|---|---|---|
| id_producto | INT (PK) | Identificador único del producto |
| nombre | VARCHAR(100) | Nombre del producto |
| marca | VARCHAR(100) | Marca comercial del producto |
| descripcion | TEXT | Descripción del producto |
| codigo_barras | VARCHAR(50) | Código de barras |
| id_categoria | INT (FK) | Categoría del producto |
| id_proveedor | INT (FK) | Proveedor del producto |
| precio_compra | DECIMAL(10,2) | Precio de compra |
| precio_venta | DECIMAL(10,2) | Precio de venta |
| requiere_receta | BOOLEAN | Indica si el producto requiere receta médica |
| estado | VARCHAR(20) | Estado del producto: disponible, agotado, suspendido o caducado |

> `stock` y `fecha_caducidad` ya no viven en Productos: un mismo producto puede tener varios lotes con cantidades y caducidades distintas. Ver tabla **Lotes**.

## Tabla: Categorias

| Campo | Tipo de dato | Descripción |
|---|---|---|
| id_categoria | INT (PK) | Identificador de la categoría |
| nombre | VARCHAR(50) | Nombre de la categoría |
| tipo | VARCHAR(50) | Tipo de categoría, como medicamento, higiene, suplemento o equipo médico |
| descripcion | TEXT | Descripción de la categoría |

## Tabla: Clientes

Registra a las personas a las que se les puede vender, en particular necesaria para poder cobrar los saldos de ventas a pago diferido.

| Campo | Tipo de dato | Descripción |
|---|---|---|
| id_cliente | INT (PK) | Identificador único del cliente |
| nombre | VARCHAR(50) | Nombre |
| apellido_paterno | VARCHAR(50) | Apellido paterno |
| apellido_materno | VARCHAR(50) | Apellido materno |
| telefono | VARCHAR(20) | Número telefónico |
| correo | VARCHAR(100) | Correo electrónico |
| direccion | VARCHAR(200) | Dirección |
| fecha_registro | DATE | Fecha en la que se registró al cliente |
| estado | VARCHAR(20) | Estado del cliente: activo o inactivo |

## Tabla: Proveedores

| Campo | Tipo de dato | Descripción |
|---|---|---|
| id_proveedor | INT (PK) | Identificador del proveedor |
| nombre_empresa | VARCHAR(100) | Nombre del proveedor |
| telefono | VARCHAR(20) | Número telefónico |
| correo | VARCHAR(100) | Correo electrónico |
| direccion | VARCHAR(200) | Dirección |
| fecha_registro | DATE | Fecha en la que se registró al proveedor |
| fecha_ultima_compra | DATE | Fecha de la compra más reciente realizada al proveedor |
| estado | VARCHAR(20) | Estado del proveedor: activo, inactivo o suspendido |

## Tabla: Lotes

Controla el inventario por lote de cada producto, en lugar de un único stock y una única fecha de caducidad por producto. Permite manejo FEFO (first-expired-first-out) y rastrear caducidades de forma independiente entre reabastecimientos.

| Campo | Tipo de dato | Descripción |
|---|---|---|
| id_lote | INT (PK) | Identificador único del lote |
| id_producto | INT (FK) | Producto al que pertenece el lote |
| numero_lote | VARCHAR(50) | Número de lote asignado por el proveedor o la farmacia |
| fecha_ingreso | DATE | Fecha en la que el lote ingresó al inventario |
| fecha_caducidad | DATE | Fecha de caducidad de ese lote específico |
| cantidad_inicial | INT | Cantidad recibida originalmente |
| cantidad_disponible | INT | Cantidad disponible actualmente de ese lote |
| estado | VARCHAR(20) | Estado del lote: disponible, agotado o caducado |

## Tabla: Inventario

| Campo | Tipo de dato | Descripción |
|---|---|---|
| id_movimiento | INT (PK) | Identificador del movimiento |
| id_lote | INT (FK) | Lote relacionado |
| tipo_movimiento | VARCHAR(20) | Tipo de movimiento: entrada o salida |
| cantidad | INT | Cantidad de productos involucrados |
| fecha_movimiento | DATETIME | Fecha y hora del movimiento |
| motivo | VARCHAR(20) | Motivo del movimiento: compra, venta, devolución, daño, caducidad o ajuste |

## Tabla: Empleados

| Campo | Tipo de dato | Descripción |
|---|---|---|
| id_empleado | INT (PK) | Identificador del empleado |
| nombre | VARCHAR(50) | Nombre |
| apellido_paterno | VARCHAR(50) | Apellido paterno |
| apellido_materno | VARCHAR(50) | Apellido materno |
| puesto | VARCHAR(50) | Puesto que desempeña |
| telefono | VARCHAR(20) | Número telefónico |
| correo | VARCHAR(100) | Correo electrónico |
| fecha_contratacion | DATE | Fecha en la que fue contratado |
| estado | VARCHAR(20) | Estado del empleado: activo o inactivo |

## Tabla: Tickets

Esta tabla representa la transacción completa realizada durante una compra.

| Campo | Tipo de dato | Descripción |
|---|---|---|
| id_ticket | INT (PK) | Identificador único del ticket |
| id_empleado | INT (FK) | Empleado que realizó la transacción |
| id_cliente | INT (FK, nullable) | Cliente al que se le vendió (nulo si fue venta de mostrador sin registrar) |
| fecha_venta | DATETIME | Fecha y hora de emisión del ticket |
| subtotal | DECIMAL(10,2) | Importe antes de aplicar descuentos (sincronizado automáticamente desde Ventas) |
| descuento | DECIMAL(10,2) | Descuento aplicado a la transacción |
| total | DECIMAL(10,2) | Importe total del ticket (columna calculada: subtotal − descuento) |
| modalidad_pago | VARCHAR(20) | Modalidad de pago: contado o diferido |
| monto_pagado | DECIMAL(10,2) | Cantidad pagada hasta el momento (sincronizada automáticamente desde Pagos) |
| saldo_pendiente | DECIMAL(10,2) | Cantidad que todavía debe pagarse (columna calculada: total − monto_pagado) |
| fecha_limite_pago | DATE | Fecha límite para liquidar un pago diferido |
| estado_pago | VARCHAR(20) | Estado del pago: pagado, parcial, pendiente o vencido |
| estado_ticket | VARCHAR(20) | Estado del ticket: completado, cancelado o devuelto |

> `metodo_pago` se eliminó de esta tabla: con pago diferido un ticket puede recibir abonos con métodos distintos, así que el método vive únicamente en **Pagos**, por abono.

## Tabla: Ventas

Cada registro representa un producto (de un lote específico) incluido dentro de un ticket.

| Campo | Tipo de dato | Descripción |
|---|---|---|
| id_venta | INT (PK) | Identificador único del registro de venta |
| id_ticket | INT (FK) | Ticket al que pertenece el producto vendido |
| id_lote | INT (FK) | Lote del producto del que se descontó la venta |
| cantidad | INT | Cantidad de unidades vendidas |
| precio_unitario | DECIMAL(10,2) | Precio del producto al momento de la venta |
| subtotal | DECIMAL(10,2) | Columna calculada: cantidad × precio_unitario |

## Tabla: Pagos

Esta tabla registra los pagos y abonos correspondientes a los tickets, especialmente cuando se utiliza pago diferido.

| Campo | Tipo de dato | Descripción |
|---|---|---|
| id_pago | INT (PK) | Identificador único del pago |
| id_ticket | INT (FK) | Ticket al que se aplica el pago |
| monto | DECIMAL(10,2) | Cantidad pagada o abonada |
| fecha_pago | DATETIME | Fecha y hora en la que se realizó el pago |
| metodo_pago | VARCHAR(30) | Método utilizado: efectivo, tarjeta o transferencia |
| numero_referencia | VARCHAR(100) | Número de referencia de la operación |
| estado | VARCHAR(20) | Estado del pago: aplicado, pendiente o cancelado |

## Tabla: Devoluciones

Esta tabla registra la información general de cada devolución.

| Campo | Tipo de dato | Descripción |
|---|---|---|
| id_devolucion | INT (PK) | Identificador único de la devolución |
| id_ticket | INT (FK) | Ticket relacionado con la devolución |
| id_empleado | INT (FK) | Empleado que registró la devolución |
| fecha_devolucion | DATETIME | Fecha y hora de la devolución |
| motivo | VARCHAR(200) | Motivo general de la devolución |
| total_devuelto | DECIMAL(10,2) | Importe total devuelto |
| tipo_reembolso | VARCHAR(30) | Tipo de reembolso: efectivo, transferencia, saldo a favor o cambio |
| estado | VARCHAR(20) | Estado: solicitada, aprobada, rechazada o completada |

## Tabla: Detalle_Devolucion

Esta tabla especifica cuáles productos de una venta fueron devueltos.

| Campo | Tipo de dato | Descripción |
|---|---|---|
| id_detalle_devolucion | INT (PK) | Identificador único del detalle |
| id_devolucion | INT (FK) | Devolución a la que pertenece |
| id_venta | INT (FK) | Registro del producto vendido |
| cantidad | INT | Cantidad de unidades devueltas |
| importe | DECIMAL(10,2) | Cantidad monetaria que será reembolsada |
| estado_producto | VARCHAR(30) | Condición del producto: sellado, abierto, dañado o caducado |
| reintegrar_stock | BOOLEAN | Indica si el producto puede regresar al inventario |

## Relaciones entre las tablas

| Tipo de relación | Tabla de origen | Tabla de destino | Justificación |
|---|---|---|---|
| 1:N | Categorias | Productos | Una categoría puede incluir muchos productos, pero cada producto pertenece a una sola categoría. |
| 1:N | Proveedores | Productos | Un proveedor puede suministrar varios productos, pero cada producto tiene asignado un proveedor. |
| 1:N | Productos | Lotes | Un producto puede tener varios lotes con distintas fechas de caducidad. |
| 1:N | Lotes | Inventario | Un lote puede generar muchos movimientos de inventario. |
| 1:N | Lotes | Ventas | Un lote puede aparecer en diferentes registros de venta hasta agotarse. |
| 1:N | Empleados | Tickets | Un empleado puede generar muchos tickets, pero cada ticket es generado por un solo empleado. |
| 1:N | Clientes | Tickets | Un cliente puede tener muchos tickets asociados (incluyendo saldos por pago diferido). |
| 1:N | Tickets | Ventas | Un ticket puede contener varios registros de productos vendidos. |
| 1:N | Tickets | Pagos | Un ticket puede recibir varios pagos o abonos. |
| 1:N | Tickets | Devoluciones | Un ticket puede generar varias devoluciones parciales. |
| 1:N | Empleados | Devoluciones | Un empleado puede registrar varias devoluciones. |
| 1:N | Devoluciones | Detalle_Devolucion | Una devolución puede contener varios productos devueltos. |
| 1:N | Ventas | Detalle_Devolucion | Un producto vendido puede devolverse total o parcialmente. |

## Funcionamiento general

- **Clientes** permite identificar a quién se le vendió, indispensable para cobrar saldos de pago diferido.
- **Lotes** controla el inventario por lote de cada producto, permitiendo manejo FEFO y caducidades independientes entre reabastecimientos.
- **Tickets** almacena la transacción completa.
- **Ventas** almacena individualmente los productos (por lote) incluidos en cada ticket.
- **Pagos** almacena los pagos y abonos realizados sobre un ticket.
- **Devoluciones** contiene la información general de cada devolución.
- **Detalle_Devolucion** indica exactamente qué productos fueron devueltos.
- **Inventario** registra las entradas y salidas de productos por lote.
- Al vender, se descuenta automáticamente la cantidad del lote correspondiente y se registra una salida en Inventario.
- Si un producto devuelto puede volver a venderse (`reintegrar_stock = true`), se registra automáticamente una entrada en Inventario sobre el mismo lote de origen.
- `Tickets.subtotal` se recalcula automáticamente a partir de la suma de `Ventas.subtotal` del mismo ticket.
- `Tickets.total` y `Tickets.saldo_pendiente` son columnas calculadas, no se guardan manualmente.
- Cuando el pago es de contado, el saldo pendiente debe ser cero.
- Cuando el pago es diferido, el ticket puede recibir varios abonos (cada uno con su propio método de pago) hasta liquidar el saldo.
- `Tickets.monto_pagado` se recalcula automáticamente a partir de la suma de `Pagos.monto` aplicados al ticket.
- Cuando el saldo pendiente llega a cero, el estado del pago cambia automáticamente a `pagado`.
