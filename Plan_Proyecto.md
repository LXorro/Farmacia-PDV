# Plan del proyecto — Sistema de Farmacia (Bases de Datos Distribuidas)

## 1. Alcance (actualizado)

Por ser un entregable de curso, el alcance se reduce a **un solo sistema: Farmacia**. Se descarta el PDV Genérico y la migración desde Eleventa (`PDVDATA.FDB`/`PDVDATA.sql`) — esos archivos quedan en la carpeta solo como referencia, sin uso en el entregable.

`Farmacia_actualizado.md` sigue siendo el esquema base (Productos, Categorías, Clientes, Proveedores, Lotes, Inventario, Empleados, Tickets, Ventas, Pagos, Devoluciones, Detalle_Devolucion). Lo único que falta agregar es la capa de **multi-sucursal**, necesaria para poder demostrar los tres temas de BD distribuidas sobre este mismo esquema.

### Autenticación y roles (tabla `Usuarios`, separada de `Empleados`)

No todo empleado tiene acceso al sistema, así que el login vive en su propia tabla (`Usuarios`, FK 1:1 opcional a `Empleados`) en vez de agregarle columnas a `Empleados`. `rol` (`admin`/`empleado`) controla el alcance **en la aplicación**, reforzando la misma idea de fragmentación que ya está en la base de datos:

| Pantalla | Empleado | Admin |
|---|---|---|
| Punto de Venta, Clientes, Tickets, Inventario | solo su sucursal (combo bloqueado/filtrado) | todas |
| Productos, Categorías | solo lectura (catálogo maestro = central) | CRUD |
| Proveedores, Empleados, Sucursales, Usuarios | oculto | CRUD |

El primer usuario que se registra queda **admin** automáticamente (bootstrap); los siguientes quedan `empleado` por defecto, y un admin puede promoverlos después desde la pantalla de Usuarios.

**Importante:** la restricción por rol es solo de **alcance** (sucursal, catálogo maestro), no de **autoría**. Cualquier empleado de una sucursal puede ver, cobrar, abonar o devolver tickets/ventas que haya atendido **otro** empleado de esa misma sucursal — `TicketsPage` filtra por `id_sucursal`, no por `id_empleado`, y los combos de "empleado que atiende" en Punto de Venta y en devoluciones listan a todo el personal activo de la sucursal (preseleccionan al usuario logueado por comodidad, pero se puede cambiar). Esto refleja que el sistema modela personal compartiendo una caja/turno, no cuentas aisladas entre sí.

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

## 6. Pendiente (opcional)

- Pulir la app Qt: validaciones adicionales, reportes de venta, pantalla
  de inventario/lotes.
- Si se quiere entregar en PDF/Word en vez de Markdown, convertir
  `docs/Reporte.md` (los diagramas Mermaid se pueden exportar como imagen
  si el formato de entrega no soporta Mermaid).
