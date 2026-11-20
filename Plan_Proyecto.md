# Plan del proyecto — Sistema de Farmacia (Bases de Datos Distribuidas)

## 1. Alcance (actualizado)

Por ser un entregable de curso, el alcance se reduce a **un solo sistema: Farmacia**. Se descarta el PDV Genérico y la migración desde Eleventa (`PDVDATA.FDB`/`PDVDATA.sql`) — esos archivos quedan en la carpeta solo como referencia, sin uso en el entregable.

`Farmacia_actualizado.md` sigue siendo el esquema base (Productos, Categorías, Clientes, Proveedores, Lotes, Inventario, Empleados, Tickets, Ventas, Pagos, Devoluciones, Detalle_Devolucion). Lo único que falta agregar es la capa de **multi-sucursal**, necesaria para poder demostrar los tres temas de BD distribuidas sobre este mismo esquema.

### Autenticación y roles (tabla `Usuarios`, separada de `Empleados`)

No todo empleado tiene acceso al sistema, así que el login vive en su propia tabla (`Usuarios`, FK 1:1 opcional a `Empleados`) en vez de agregarle columnas a `Empleados`. `rol` (`admin`/`empleado`) controla el alcance **en la aplicación**, reforzando la misma idea de fragmentación que ya está en la base de datos:

| Pantalla | Empleado | Admin |
|---|---|---|
| Punto de Venta, Clientes, Tickets, Inventario | solo su sucursal (combo bloqueado/filtrado) | todas (el equipo central no vende: solo consulta) |
| Productos, Categorías | solo lectura (catálogo maestro = central) | CRUD |
| Proveedores, Empleados, Sucursales, Usuarios | oculto | CRUD |

No hay registro libre: solo si la tabla `Usuarios` está vacía, el login ofrece crear el **administrador inicial** (sin empleado ligado). Las demás cuentas las crea un admin desde la pantalla de Usuarios; desde una sucursal, conectándose al central (cuenta `farmacia_remoto`, ver `db/distribuido/autonomia/README.md`).

**Importante:** la restricción por rol es solo de **alcance** (sucursal, catálogo maestro), no de **autoría**. Cualquier empleado de una sucursal puede ver, cobrar, abonar o devolver tickets/ventas que haya atendido **otro** empleado de esa misma sucursal — `TicketsPage` filtra por `id_sucursal`, no por `id_empleado`, y los combos de "empleado que atiende" en Punto de Venta y en devoluciones listan a todo el personal activo de la sucursal (preseleccionan al usuario logueado por comodidad, pero se puede cambiar). Esto refleja que el sistema modela personal compartiendo una caja/turno, no cuentas aisladas entre sí.

**Autonomía y propiedad de datos:** el modo ya no es una propiedad del equipo sino de la sesión (un empleado trabaja sobre la base local de su sucursal; un administrador, al iniciar sesión, se conecta al central y ve todas las sucursales; sin central el equipo es único) y la base solo permite escribir cada tabla desde un nodo (ver `db/distribuido/autonomia/README.md`). Un empleado puede operar en otra sucursal con una *asignación temporal* autorizada por un admin, pero solo iniciando sesión en un equipo de esa sucursal.

- **Motor:** MySQL/MariaDB. Cubre igual los tres temas: particionamiento nativo (`PARTITION BY LIST/RANGE/HASH`), replicación integrada (master-slave nativa; Galera Cluster en MariaDB si se quiere multi-maestro), y transacciones distribuidas vía **XA Transactions** (`XA START`/`XA END`/`XA PREPARE`/`XA COMMIT`) como equivalente de 2PC.
- **Cliente/UI:** Qt (C++), conectado a la base vía `QSqlDatabase` (driver `QMYSQL`).
- **Temas a demostrar:** fragmentación (horizontal y vertical), replicación entre sitios/sucursales, transacciones distribuidas.

## 2. Qué se le agrega a Farmacia para volverla distribuida

Farmacia, tal como está en `Farmacia_actualizado.md`, es un esquema de una sola sede. Para el curso necesita una capa de sucursales:

- **Tabla nueva: `Sucursales`** — id_sucursal, nombre, dirección, rol (`central` o `sucursal`).
- **Tablas que se fragmentan horizontalmente por sucursal** (cada sucursal solo tiene sus propias filas, el central las reconstruye):
  - `Lotes` e `Inventario` (el stock físico vive donde está la sucursal).
  - `Tickets`, `Ventas`, `Pagos`, `Devoluciones`, `Detalle_Devolucion` (las transacciones ocurren en una sucursal específica).
  - Todas ganan una columna `id_sucursal`.
- **Fragmentación vertical:** `Clientes` — datos de contacto (nombre, teléfono, dirección) se capturan y viven en la sucursal donde se registró el cliente; el saldo consolidado de pago diferido (útil para cobranza) se mantiene en el nodo central.
- **Replicación (maestro → sucursales):** `Productos`, `Categorias`, `Proveedores` — catálogo maestro que se define en el central y se replica de solo lectura hacia cada sucursal (replicación nativa MySQL/MariaDB, central como master, sucursales como slaves). El central siempre es la fuente de verdad.
- **Transacción distribuida:** escenario de un producto de alta rotación cuyo inventario se controla de forma centralizada (no fragmentado) — dos sucursales intentan vender el último lote disponible casi al mismo tiempo. Se resuelve con XA Transactions (`XA START`/`XA PREPARE` en el nodo sucursal y en el nodo central, `XA COMMIT` solo si ambos confirman) para evitar sobreventa.

## 3. Estado de implementación

- **`db/schema.sql`** — DDL completo en MariaDB con `Sucursales` + `id_sucursal` en Clientes/Empleados/Lotes/Tickets/Devoluciones, triggers que automatizan lo descrito en "Funcionamiento general" de `Farmacia_actualizado.md` (descuento de stock al vender, resincronización de `subtotal`/`monto_pagado`/`estado_pago`, reintegro de stock en devoluciones).
  - Nota técnica: MySQL/MariaDB no permiten `FOREIGN KEY` en tablas particionadas (ni que sean referenciadas por FK). El esquema en uso por la app prioriza integridad referencial (`id_sucursal` como columna normal); el `PARTITION BY` físico para el entregable de fragmentación se demuestra aparte, sobre copias desnormalizadas, para no romper las FK que la app necesita.
- **`db/seed.sql`** — datos de ejemplo (2 sucursales, categorías, proveedores, productos, empleados, clientes, lotes).
- **`db/create_user.sql`** — alternativa manual (obsoleta): el asistente de primer arranque de la app ya crea la base, el usuario `farmacia_app` y el esquema. Si se usa a mano, la contraseña fija de este script no coincide con la que guarda la app.
- **`app/`** — aplicación Qt6 (CMake + Widgets + Sql), con interfaces en `.ui` editables desde Qt Designer. Una pantalla por cada entidad del esquema:
  - `src/mainwindow.ui/.h/.cpp` — ventana principal con menú lateral (Inicio, Punto de Venta, Productos, Clientes, Tickets, Inventario, Categorías, Proveedores, Empleados, Sucursales, Usuarios).
  - `src/pages/iniciopage.ui` + `.h/.cpp` — pestaña de inicio (primer ítem del menú): saludo con usuario/rol/sucursal, 3 indicadores rápidos (ventas de hoy, tickets con saldo pendiente, lotes por caducar en 30 días — todos filtrados por sucursal si el usuario es `empleado`), y una cuadrícula de accesos directos a cada pantalla (los de administración solo se muestran si el usuario es `admin`). Emite `abrirPagina(nombre)`, que `MainWindow` conecta para mover el `listMenu` a la pantalla correspondiente.
  - `src/pages/puntoventapage` — venta funcional: selección de sucursal/empleado/cliente, carrito con selección de lote por FEFO, checkout transaccional (`Tickets`+`Ventas`+`Pagos`) con verificación de stock (`SELECT ... FOR UPDATE`) antes de comprometer la venta.
  - `src/pages/productospage`, `clientespage`, `sucursalespage`, `categoriaspage`, `proveedorespage`, `empleadospage` — CRUD completo (alta/edición/eliminación) de cada catálogo, cada uno con su diálogo (`src/dialogs/*dialog.ui`).
  - `src/pages/lotespage` + `src/dialogs/lotedialog.ui` — inventario: recepción de lotes nuevos y edición de cantidad disponible/caducidad/estado de lotes existentes.
  - `src/pages/ticketspage` — historial de ventas (join Sucursal/Empleado/Cliente); doble clic o "Ver detalle" abre `src/dialogs/ticketdetalledialog.ui`, que muestra líneas vendidas y pagos de un ticket, y permite:
    - **Registrar abono** (pago diferido): valida saldo pendiente, inserta en `Pagos` — los triggers resincronizan `monto_pagado`/`estado_pago`.
    - **Registrar devolución** de una línea seleccionada vía `src/dialogs/devoluciondialog.ui`: inserta `Devoluciones`+`Detalle_Devolucion` en una transacción; si `reintegrar_stock` está marcado, el trigger regresa la cantidad al lote de origen.
  - **Login y cuentas** (2026-10-04): `src/dialogs/logindialog.ui`, `registrodialog.ui`, `usuariodialog.ui`, `src/pages/usuariospage` (admin) y `src/database/sesionactual.{h,cpp}` (singleton con la sesión activa). `src/util/passwordutil.{h,cpp}` hashea con SHA-256 + sal (no bcrypt — Qt no lo trae de fábrica). `main.cpp` pide login antes de abrir `MainWindow`; `MainWindow` arma el menú según `rol` (ver matriz en la sección 1); `ProductosPage`/`CategoriasPage` quedan de solo lectura para `empleado`; `PuntoVentaPage`/`ClientesPage`/`LotesPage`/`TicketsPage` se filtran/bloquean a la sucursal del usuario logueado.
  - `src/database/databasemanager.{h,cpp}` + `src/database/dbconfig.h` — conexión a MariaDB vía `QMYSQL`. `dbconfig.h` solo trae host/puerto/base por defecto; el usuario y la contraseña (aleatoria) los genera el asistente de primer arranque (`src/dialogs/configurarbddialog.*` + `src/database/dbinicializador.*`) con una cuenta administradora usada una sola vez, y se guardan con `QSettings`. El asistente corre el `db/schema.sql` embebido en los recursos del ejecutable y no toca los datos si las tablas ya existen.
  - Compila limpio con `cmake -B build && cmake --build build` (Qt6, verificado en este entorno).
- **Cargado y probado en vivo** (2026-10-01 a 2026-10-04): esquema y datos de ejemplo cargados en MariaDB; la app Qt arrancó contra la base real y se registró una venta genuina desde la interfaz (Ticket #2, sucursal Norte) con subtotal/stock/estado de pago sincronizados correctamente por los triggers. El flujo de devolución (inserción + reintegro de stock) se verificó con la consulta exacta del diálogo. El bootstrap de login (primer registro → admin, siguientes → empleado) se verificó replicando la consulta exacta de `RegistroDialog`/`LoginDialog` contra la base real.

### Reparto de datos: qué decide el central y qué la sucursal

- **Central (admin, baja a las sucursales):** Sucursales, Empleados, Usuarios, Asignaciones, Categorías, Proveedores y Productos (identidad del producto + **precio sugerido**).
- **Sucursal (empleados, sube al central):** Clientes, Lotes (con su **costo unitario**: cada compra cuesta distinto), Inventario, Tickets, Ventas, Pagos, Devoluciones y **`Productos_Sucursal`** (precio propio, si la sucursal maneja el producto y su mínimo de existencias). Sin fila, se vende al precio sugerido. `Ventas.precio_unitario` guarda lo cobrado, así que cambiar un precio no altera tickets viejos.
- **Permisos temporales:** `Permisos_Temporales` (central → sucursales) concede a un empleado el rol «encargado» por 1-3 días; sin él solo hace lo esencial de un vendedor. `Bitacora_Cambios` (sucursal → central) registra quién cambió precios y lotes. Detalle en `ESTADO_PROYECTO.md`; migración `db/migraciones/003_permisos_y_bitacora.sql`.
- El catálogo es único porque `codigo_barras` es único: con varios escritores dos sucursales podrían crear el mismo código a la vez. Migración de bases existentes: `db/migraciones/002_precios_por_sucursal.sql`.

## 4. Demo de BD distribuidas — implementada y verificada (`db/distribuido/`)

Entorno aparte con dos instancias MariaDB desechables (`central:13306`,
`sucursal:13307`), que no tocan el servicio del sistema donde vive la app.
Ver `db/distribuido/README.md` para el detalle completo y cómo
levantarlo/apagarlo (`start.sh`/`stop.sh`). Los tres temas ya están
implementados y probados:

1. **Fragmentación horizontal física** (`fragmentacion_demo.sql`) — tabla
   `Ventas_Particionadas` con `PARTITION BY LIST (id_sucursal)`. Verificado
   con `EXPLAIN PARTITIONS`: una consulta filtrada por sucursal solo
   escanea su partición (poda de particiones). Vista `Ventas_Consolidadas`
   reconstruye el agregado global.
2. **Replicación maestro → sucursal, filtrada** — central como master,
   sucursal como slave con `replicate-do-table` solo para
   `Productos`/`Categorias`/`Proveedores`. Verificado: un proveedor nuevo
   en central llega a sucursal; un cliente nuevo en central (dato local)
   no llega.
3. **Transacción distribuida XA (2PC)** (`setup_xa_demo.sql` + `xa_venta.sh`)
   — `Stock_Centralizado` en central como autoridad de un producto de alta
   rotación no fragmentado; cada venta en sucursal debe confirmarse en
   ambos nodos o en ninguno. Implementado con `gtrid` compartido + `bqual`
   distinto por nodo (formato estándar XA multi-recurso — necesario porque
   el PREPARE de central se replica como evento de control hacia sucursal
   y colisiona si se usa el mismo xid plano en ambos). Verificado con caso
   de venta válida (ambos preparan y confirman) y caso de sobreventa
   (central rechaza el prepare, se hace rollback en ambos nodos, sin
   cambios parciales).

## 5. Entregables del reporte — completos (`docs/`)

1. ✅ `docs/diagrama_er.md` — diagrama ER (Mermaid) del esquema de Farmacia
   con la capa de sucursales.
2. ✅ `docs/diagrama_arquitectura.md` — diagrama (Mermaid) de la demo
   distribuida: fragmentación + replicación + XA, con tabla comparativa.
3. ✅ `docs/Reporte.md` — reporte escrito completo: introducción,
   objetivos, alcance, diseño de BD, arquitectura distribuida (incluyendo
   el hallazgo técnico de la colisión XA/replicación), pruebas realizadas
   y conclusiones.

## 6. Siguiente cambio planeado: impresión de tickets y pago mixto

Pedido del 2026-11-18. **Fase 1 implementada el 2026-11-20** (ver 6.7); faltan la fase 2 (ESC/POS) y probar con una impresora real. Decisiones tomadas: impresora térmica de **80 mm**, texto **alineado y de buen tamaño, sin saltos de línea inesperados**, con el formato sencillo de un ticket de texto (no un diseño gráfico), y la información que pide un ticket de venta en México. Lo que dice la ley está marcado como **por validar con un contador** (ver 6.4).

### 6.1 Impresión (fases)

1. **Fase 1 — por el driver de Windows.** `Qt6::PrintSupport` (`QPrinter`, `QPrintPreviewDialog`, `QPrinterInfo`). El ticket se arma como **texto monoespaciado por columnas** y se dibuja con una fuente de ancho fijo, de modo que cada línea mide exactamente lo que se calculó y no puede partirse sola. Sirve con cualquier impresora instalada en Windows.
2. **Fase 2 (opcional) — ESC/POS directo.** Por TCP al puerto 9100 (red, con `QTcpSocket`; `Qt6::Network` ya está enlazado) o por el spooler en modo RAW (USB, código solo de Windows). Da corte de papel y apertura de cajón. Requiere página de códigos para acentos y `ñ` (CP858 o WPC1252).

**Ancho de 80 mm:** ~72 mm imprimibles = 576 puntos a 203 dpi. La fuente A de 12 puntos da **48 columnas**; se usará **42 por defecto** (más margen y letra más grande) y es configurable de 40 a 48. El motor de layout (`util/ticketimpreso.*`) trabaja siempre en columnas, así que otro ancho (58 mm = 32 columnas, A4) solo cambia ese número.

- **Configuración por equipo** (`QSettings`): impresora, ancho de papel (80 mm por defecto, 58 mm y personalizado), columnas, márgenes, tamaño de letra, copias, imprimir al cobrar (sí/no/preguntar), corte y cajón (fase 2). Pantalla «Configurar impresora» con **impresión de prueba que incluye una regla de columnas** para calibrar.
- **Vista previa y pruebas sin impresora:** el mismo texto se envía a un PDF del mismo ancho.
- **Dónde se imprime:** al cobrar, «Reimprimir» en el detalle del ticket, comprobante de devolución (con el estado «pendiente en terminal» del reembolso a tarjeta) y, opcional, el corte del día en formato angosto.
- **Reglas del layout:** nombres de producto partidos por palabras (nunca a mitad de palabra salvo que no quepa), importes alineados a la derecha, columnas fijas `CANT  DESCRIPCIÓN … P.UNIT  IMPORTE`, acentos y `ñ` correctos, líneas separadoras de ancho exacto.

### 6.2 Contenido del ticket

Establecimiento: nombre o razón social, **RFC**, domicilio del establecimiento (calle, colonia, CP, municipio, estado), teléfono, sucursal. Venta: **folio**, fecha y hora, cajero, cliente (si se registró). Líneas: cantidad, descripción, precio unitario e importe. Totales: subtotal, **IVA desglosado por tasa** (0 % y 16 %), descuento si lo hay y total. Pago: método(s); si fue tarjeta, **autorización y últimos 4 dígitos**; efectivo recibido y **cambio**; en venta a crédito, abonos, **saldo y fecha límite**. Pie: leyenda de que no es factura y cómo solicitarla, política de devoluciones, aviso de receta retenida para antibióticos y medicamentos controlados.

**Datos que hoy no existen en la base y habría que agregar** (los escribe el central y bajan a las sucursales, como el resto del catálogo):
- `Sucursales`: razón social, RFC, domicilio completo y régimen fiscal.
- `Productos`: **tasa de IVA** (0 % para medicina de patente con registro sanitario; 16 % para el resto, p. ej. higiene personal y suplementos) y, si se quiere, marca de «requiere receta retenida».
- Hipótesis a confirmar: que los precios de venta **ya incluyen IVA**, de modo que el desglose se calcula hacia atrás.
- Folio: hoy `id_ticket` avanza de 10 en 10 por sucursal (12, 22, 32…); decidir si se imprime tal cual o con prefijo de sucursal.
- Estos cambios van en una migración nueva, **posterior a la corrección de totales** de la réplica (ver 6.5), y siguen la regla de siempre: aplicarla en todos los nodos antes de usar la versión nueva de la app.

### 6.3 Pago mixto (efectivo + tarjeta / transferencia)

El esquema **ya lo permite**: `Pagos` guarda una fila por pago y un ticket puede tener varias; los triggers suman `monto_pagado`. Lo que falta es la pantalla y las reglas.

- **Cobrar:** nueva opción «Mixto» con una lista de pagos (método + monto). La suma de los pagos debe igualar el total en una venta de contado.
- **Cambio:** solo sale de la parte en efectivo; en `Pagos` se registra el efectivo **aplicado** (sin el cambio) para que el corte de efectivo siga cuadrando.
- **Tarjeta dentro de un mixto:** pide autorización (4-20 caracteres) y, opcional, los últimos 4 dígitos, igual que hoy.
- **Devoluciones:** ya calculan por vía de pago (`m_tarjetaPagada` y `m_otrosDisponible`): a la tarjeta solo hasta lo pagado con tarjeta, y efectivo o transferencia solo por el resto. Hay que **probarlo** con tickets mixtos.
- **Reporte diario:** «cobros por método» ya separa por fila de pago; los tickets mixtos se reparten solos.
- **Ticket impreso:** una línea por cada método, con su monto.
- **Abonos de ventas a crédito:** decidir si también pueden ser mixtos (hoy es un solo método por abono).

### 6.4 Lo legal: por validar

La app **no emite CFDI** (factura electrónica con timbrado de un PAC) y eso queda **fuera de alcance**; el ticket no sustituye a la factura. Conviene que un contador confirme la lista de datos mínimos, la tasa de IVA de cada producto y la leyenda de facturación. Lo averiguado en fuentes públicas (blogs y despachos, no el texto oficial) coincide en: medicina de patente con registro sanitario a **tasa 0 %** (no exenta); suplementos, cosméticos e higiene personal a 16 %; ambas tasas pueden ir en el mismo ticket; hay discrepancia sobre el artículo del Reglamento de la LIVA que define «medicina de patente» (6 o 7). Antes de afirmar algo de cara a un cliente, verificar en la LIVA y en la Resolución Miscelánea Fiscal vigentes.

### 6.5 Dependencia con otro pendiente

**Resuelto en código, pendiente de aplicar:** migración 005 (`Tickets.total` y `saldo_pendiente` pasan a `VIRTUAL`; reproducido y probado con MariaDB 10.11.14 + 13.0.2). Texto original del problema: las columnas `total` y `saldo_pendiente` de `Tickets` llegan en **0.00** al central real (MariaDB 10.11.14, con un nodo 13.x); no se reproduce con ambos lados en 13.0.2. Un ticket impreso en una sucursal sale bien (calcula en local), pero cualquier pantalla de administrador sobre el central mostrará ventas y saldos en cero hasta corregirlo. Va **antes** de esta mejora.

### 6.7 Estado de la implementación (2026-11-20)

**Hecho** (código en `app/src/util/ticketimpreso.*` y `app/src/dialogs/{pagomixto,vistaticket,configurarticket}dialog.*`):
- Ticket en texto por columnas (42 por defecto, configurable de 32 a 64; 32 = 58 mm), con encabezado del establecimiento, folio, fecha, cajero y cliente, líneas, **IVA desglosado por tasa**, forma de pago (autorización y últimos 4 dígitos de la tarjeta, referencia de la transferencia, efectivo recibido y cambio), saldo y fecha límite de las ventas a crédito, devoluciones registradas y pie configurable.
- **Vista en pantalla** con la misma fuente y columnas que el papel, **Guardar PDF** de 80 mm (fuente incrustada) e **Imprimir** por el driver de Windows (`QPrinter`). Sirve sin impresora.
- Se abre al cobrar («Ver / imprimir ticket») y desde el detalle de cualquier ticket (también para un admin sobre el central).
- **Pago mixto** (contado): tarjeta, transferencia y efectivo; el efectivo es lo que falta y se calcula el cambio. Se guarda una fila de `Pagos` por método (el efectivo, sin el cambio). Las devoluciones de un ticket mixto respetan la vía de pago (verificado).
- Configuración por equipo en `QSettings` (impresora, ancho, columnas, razón social, RFC, domicilio, teléfono, pie, regla de IVA). **No hay cambios de esquema**: no requiere migración.

**Pendiente / decisiones tomadas por defecto**
- **Fase 2 (ESC/POS directo):** corte de papel y cajón. No hecha.
- **Datos fiscales por equipo, no por sucursal en la base**; el domicilio y el teléfono salen de la sucursal si no se configuran. Si se quiere que viajen con el catálogo, hay que agregarlos a `Sucursales` (migración y orden de despliegue de siempre).
- **Tasa de IVA según la categoría** del producto (`medicamento` a 0 %, el resto a 16 %). Es una aproximación a validar con un contador; lo correcto es una tasa por producto en `Productos`, pero esa columna nueva rompería la réplica de un nodo sin migrar.
- **Folio = `id_ticket`** (avanza de 10 en 10 por sucursal). Sin prefijo de sucursal.
- El **efectivo recibido y el cambio** solo se imprimen al cobrar (no se guardan en la base); una reimpresión no los muestra. El efectivo simple no pide el monto recibido.
- **Comprobante de devolución aparte:** hoy las devoluciones salen como sección del ticket.
- **Abonos mixtos** y el pago mixto en ventas a crédito: no incluidos.
- **No probado con una impresora térmica real.** Verificado: texto (2,400 tickets aleatorios, ninguna línea fuera del ancho, en 6 anchos), PDF de 80 mm, impresión a una «impresora PDF» de Qt, flujo completo del cobro mixto contra MariaDB desechable y sus validaciones.

### 6.6 Pruebas a agregar al manual

Ticket de 80 mm: ninguna línea se parte, importes alineados, acentos correctos, nombre muy largo, 1 y 40 líneas, ticket a crédito, con tarjeta, mixto, devolución y reimpresión. Mixto: suma distinta del total, cambio, tarjeta sin autorización, devolución de un ticket mixto y corte de efectivo. Impresora apagada, sin papel, o sin impresora configurada: mensaje claro y la venta **no se pierde**.

## 7. Pendiente (opcional)

- Pulir la app Qt: validaciones adicionales, reportes de venta, pantalla
  de inventario/lotes.
- Si se quiere entregar en PDF/Word en vez de Markdown, convertir
  `docs/Reporte.md` (los diagramas Mermaid se pueden exportar como imagen
  si el formato de entrega no soporta Mermaid).
