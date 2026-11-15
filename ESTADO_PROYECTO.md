# Estado del proyecto — Farmacia PDV (BD distribuidas)

Actualizado: 2026-11-15 · Última versión subida: `fb75bcc` (los permisos temporales y la bitácora están en el árbol de trabajo, sin commitear) · Complementa a `Plan_Proyecto.md` (alcance y diseño original).

## 1. Qué es

Sistema de punto de venta para farmacias, hecho como entregable de **Bases de Datos Distribuidas**. Una app de escritorio (Qt 6 / C++, ~7.400 líneas) trabaja sobre MariaDB y demuestra **fragmentación**, **replicación** y **transacciones distribuidas** (XA) con un esquema multi-sucursal real.

- Stack: Qt 6 (Widgets, Sql, Svg, Network), driver `QMYSQL`, MariaDB 10.11, CMake.
- Compilación en Linux: `cmake -S app -B app/build && cmake --build app/build -j"$(nproc)"` → `app/build/FarmaciaApp`.
- Windows: `.github/workflows/windows.yml` (solo manual, `workflow_dispatch`).

## 2. Arquitectura

```
                 AWS (EC2, Ubuntu)                       Tailscale (red privada, 100.64.0.0/10)
          ┌────────────────────────────┐
          │  CENTRAL — MariaDB          │◄──── replicación de bajada (catálogo, personal, cuentas)
          │  server-id 1, log-bin, ROW  │────► replicación de subida (ventas, lotes, clientes…)
          └────────────▲───────────────┘
                       │ conexión de administración (farmacia_remoto, TLS)
        ┌──────────────┴──────────────┐
   Sucursal A (equipo + MariaDB local)   Sucursal B (equipo + MariaDB local) …
```

- **Central:** servidor en AWS con MariaDB. Consolida las ventas de todas las sucursales y es la fuente de verdad del catálogo, el personal y las cuentas.
- **Sucursales:** cada equipo conserva su MariaDB local, así que vende sin conexión y se sincroniza al volver.
- **Red:** el puerto 3306 del central solo es alcanzable por Tailscale (no se abre a internet). El tráfico va cifrado en el túnel y la conexión de la app usa TLS con un certificado de Let's Encrypt emitido para el nombre `ip-172-31-41-253.tail560a17.ts.net`.
- **Modo por sesión (no por equipo):**
  - **Empleado:** trabaja sobre la base local de su sucursal.
  - **Administrador:** al iniciar sesión se conecta al central y esa conexión pasa a ser la de toda la sesión: ve todas las sucursales y administra catálogo, personal, sucursales y cuentas. Si no hay conexión, puede entrar en modo local de solo lectura.
  - **Equipo único:** sin central configurado, una sola base local con todos los permisos.

## 3. Reparto de datos (un solo escritor por tabla, lo hacen cumplir los permisos de MariaDB)

| Escribe el **central** (admin) → baja a las sucursales | Escribe la **sucursal** (empleados) → sube al central |
|---|---|
| Sucursales, Empleados, Usuarios, Asignaciones_Temporales | Clientes, Lotes (con `costo_unitario`), Inventario |
| **Permisos_Temporales** | **Bitacora_Cambios** |
| Categorías, Proveedores | Tickets, Ventas, Pagos |
| Productos (identidad + `precio_sugerido`) | Devoluciones, Detalle_Devolucion |
| | **Productos_Sucursal** (precio propio, activo, stock mínimo) |

- Sin fila en `Productos_Sucursal`, la sucursal vende al precio sugerido. `Ventas.precio_unitario` guarda lo cobrado, así que cambiar un precio no altera tickets viejos.
- El catálogo es único porque `codigo_barras` es único: con varios escritores dos sucursales podrían crear el mismo código a la vez.
- El central **no vende**: no tiene Punto de Venta y las pantallas operativas son de consulta.

## 4. Replicación

- **Bajada (central → sucursal):** la sucursal replica las 7 tablas del central (`replicate-do-table` en su `my.cnf`).
- **Subida (sucursal → central):** el central tiene una conexión con nombre por sucursal (`sucursal_<id>`), multi-origen.
- **Sin conflictos de ID:** `auto_increment_increment=10` y un `offset` distinto por nodo. El central usa offset 1; cada sucursal una *ranura* de 2 a 10 (caben 9). Los ids de sucursal los genera el central (1, 11, 21…): id 1 → ranura 2, id 11 → ranura 3. De la ranura salen `server_id = 10 + ranura`, `gtid_domain_id` y el offset.
- `binlog-do-db=farmacia` en cada nodo: con `log-bin` activo el binlog recoge **todas** las bases del MariaDB local (con ROW el filtro es por la base de la tabla). Sin esto, otra base del equipo hizo que el central intentara aplicar tablas que no existen y su réplica se detuvo; además se filtraban datos ajenos al proyecto. El asistente también fija `replicate_do_db=farmacia` en la conexión del central.
- `binlog_format=ROW`: necesario para que los triggers (stock, subtotal, estado de pago) no se disparen dos veces.
- El esquema y las cuentas se crean con `sql_log_bin=0`: la replicación transporta solo datos.
- **Asistente de primer arranque** (automático): comprueba el MariaDB local, crea la base y la cuenta `farmacia_app`, copia el catálogo del central con la posición GTID exacta, crea las cuentas de replicación en ambos lados y arranca las dos réplicas.
- **Único paso manual:** MariaDB no activa `log-bin` ni `server-id` en caliente. Si faltan, el asistente muestra el `my.cnf` necesario; en Linux lo escribe `db/distribuido/real/preparar-nodo.sh <ranura>`.

## 5. Cuentas y contraseñas (qué es cada una; los valores no se guardan aquí)

| Cuenta | Tipo | Cuándo se pide |
|---|---|---|
| Usuario de la app (`Usuarios`) | Aplicación, hash SHA-256 + sal | En cada inicio de sesión |
| `farmacia_remoto` | MariaDB del central | En cada inicio de sesión de **admin**; no se guarda |
| `farmacia_registro` | MariaDB del central | Solo en el asistente (una vez por equipo); no se guarda |
| `farmacia_app` | MariaDB local | La genera el asistente (aleatoria) y se guarda en `QSettings` |
| `repl_s<id>` / `repl_central` | Replicación | Las genera el asistente; viven solo dentro de MariaDB |

Los usuarios de ejemplo del central están en `db/distribuido/real/datos-ejemplo-central.sql`.

## 6. Pantallas y roles

**Cada sucursal es un equipo:** la sucursal de un equipo se fija la primera vez (asistente) y no depende de quién inicie sesión. Un empleado de otra sucursal solo entra con una *asignación temporal* y opera en la sucursal **del equipo**. Es una decisión tomada (no se implementará «el usuario decide la sucursal»).

| Pantalla | Empleado | Admin (sobre el central) |
|---|---|---|
| Inicio, Punto de Venta, Clientes, Tickets, Inventario | Solo su sucursal | Consulta de todas (no vende) |
| Productos, Categorías | Lectura; el empleado fija **su precio** (botón «Precio en mi sucursal») | CRUD del catálogo |
| Proveedores, Empleados, Sucursales, Usuarios, Asignaciones | Ocultas | CRUD |
| Permisos (temporales) · Bitácora | Ocultas | Conceder/revocar · consulta |

### Permiso temporal «encargado» (nuevo)

Para evitar cambios no autorizados, un empleado **sin permiso solo hace lo esencial de un vendedor**: vender y cobrar, consultar productos y existencias, ver tickets y clientes, registrar abonos y devoluciones. Un admin puede concederle el rol «encargado» por **1, 2 o 3 días** (24/48/72 h exactas; lo exigen la app y un `CHECK` de la base), y mientras esté vigente puede:
- fijar el precio, si maneja el producto y el mínimo de existencias en su sucursal (`Productos_Sucursal`);
- dar de alta y editar lotes (con su costo).

Detalles: el permiso se guarda en `Permisos_Temporales` (la escribe el central y baja a las sucursales), vence solo aunque no haya conexión (las fechas van en **UTC**, independiente de la zona de cada equipo) y se vuelve a comprobar al guardar, por si venció con la ventana abierta. Un admin puede revocarlo antes. Cada cambio queda en `Bitacora_Cambios` (usuario, fecha, antes → después y si fue con permiso temporal), que sube al central y el admin consulta en la pantalla **Bitácora**. Las devoluciones **no** requieren permiso (decisión del proyecto).

**Limitación:** la restricción vive en la aplicación, no en la base de datos. MariaDB solo conoce una cuenta por equipo (`farmacia_app`), así que no puede distinguir qué empleado escribe; alguien con acceso directo a la base del equipo podría saltársela. Igual que los roles admin/empleado.

### Pagos y reembolsos con tarjeta

La app **no cobra ni reembolsa nada con tarjeta**: la terminal lo hace y la app registra y controla.
- **Al cobrar con tarjeta** (contado o abono) se captura la **autorización** del comprobante (4-20 caracteres alfanuméricos; normalmente 6) y, opcionalmente, los **últimos 4 dígitos**. Se guardan en `Pagos.numero_referencia` y `Pagos.ultimos_digitos`.
- **Un ticket pagado con tarjeta se devuelve a la tarjeta:** la devolución en efectivo o transferencia solo se permite por la parte que se pagó así; no puede salir en efectivo lo que se cobró con tarjeta (el cargo seguiría en la cuenta del cliente). `cambio` y `saldo_a_favor` no mueven dinero y siguen permitidos.
- **Reembolso a tarjeta en dos pasos:** la devolución se registra con la *modalidad* (`cancelacion`: mismo día, antes del cierre de lote de la terminal; `devolucion`: después del cierre). Si se captura el folio de la terminal queda `completada`; si no, queda `solicitada` («pendiente en terminal») hasta que se registre el folio desde el detalle del ticket (botón «Registrar folio de reembolso»).
- El stock se reintegra al registrar la devolución (el producto ya volvió físicamente), aunque el reembolso siga pendiente.
- **No se sabe** si la comisión de la terminal se reembolsa: depende del contrato con el banco o procesador.

Un empleado de otra sucursal solo opera con una *asignación temporal* vigente autorizada por un admin, e iniciando sesión en un equipo de esa sucursal. La barra de estado muestra el estado de la replicación (`SHOW ALL REPLICAS STATUS`).

## 7. Estructura del repositorio

```
app/                     Aplicación Qt (src/{database,dialogs,pages,util}, resources/)
db/schema.sql            Esquema completo (tablas + triggers)
db/seed.sql              Datos de ejemplo para un equipo único
db/migraciones/          002_precios_por_sucursal.sql + aplicar-002.sh (bases ya desplegadas)
db/distribuido/real/     Central real: preparar-central.sh, preparar-nodo.sh, reparar-central.sh, datos de ejemplo
db/distribuido/          Demos locales: fragmentación, XA, autonomía (demo.sh con 3 nodos)
docs/                    Reporte.md, diagramas de arquitectura y ER
Plan_Proyecto.md         Alcance y diseño · Farmacia_actualizado.md: esquema base
```

## 8. Operación

- **Preparar el central** (una vez, o al cambiar permisos): `ssh ubuntu@<central> 'sudo bash -s' < db/distribuido/real/preparar-central.sh` (acepta `PW_REGISTRO=…` para conservar la contraseña de registro).
- **Migrar una base existente:** `sudo db/migraciones/aplicar-002.sh nodo` (precios por sucursal) y `aplicar-003.sh nodo` (permisos temporales y bitácora) en cada equipo, y `…central` en el servidor; `aplicar-004.sh` (pagos y reembolsos con tarjeta; **en todos los nodos y el central antes de usar la versión nueva de la app**: ningún nodo debe guardar `tarjeta` antes de que todos lo admitan), luego `preparar-central.sh` (permisos de las cuentas) y, en cada equipo, **`preparar-nodo.sh <ranura>` otra vez** para que `Permisos_Temporales` entre en la lista de tablas que replica. Debe aplicarse en **todos** los nodos antes de cargar datos nuevos, porque con replicación por filas los esquemas no pueden diferir.
- **Agregar un equipo:**
  1. Unirlo a **tu tailnet** (invitarlo con su propio correo en Tailscale → Users; *compartir solo el nodo* no basta, porque el central no puede iniciar conexiones hacia un nodo de otro tailnet y la replicación de subida fallaría).
  2. Preparar su MariaDB local (`preparar-nodo.sh` o el `my.cnf` del asistente).
  3. Ejecutar el asistente: dirección del central = **nombre `.ts.net`**, cuenta y contraseña de registro, y su sucursal.
- **Datos de ejemplo del central:** `ssh <central> 'sudo mariadb' < db/distribuido/real/datos-ejemplo-central.sql` (idempotente; requiere la migración 002).

## 9. Verificado

- Compilación limpia en Linux; el asistente y el login contra el central real funcionaron desde este equipo.
- Replicación `sucursal_1` en marcha el 2-nov (ambos hilos activos, retraso 0) y central restaurado con sus 15 tablas y 4 triggers.
- Migración 002 y datos de ejemplo probados en un MariaDB desechable: conservan datos, son repetibles y dejan un esquema idéntico al de una instalación nueva.
- Permisos temporales y bitácora probados en un MariaDB desechable: el `CHECK` rechaza más de 72 h y rangos invertidos, la vigencia da lo mismo con cualquier zona horaria de sesión, vencido/revocado no cuentan y la migración 003 es idéntica al esquema nuevo. La app compila; las pantallas nuevas (Permisos, Bitácora) y el bloqueo de botones **no se han probado en la interfaz**.
- Datos de ejemplo cargados en el central (7 usuarios el 2-nov). **No se ha vuelto a comprobar el estado del central desde entonces.**

## 10. Pendiente y riesgos conocidos

**Sin probar de punta a punta**
- La interfaz de precios por sucursal (botón, Punto de Venta con precio propio, «bajo mínimo»).
- La replicación de subida con ventas reales (una venta de empleado que llega al central) y la barra de estado en una sesión de empleado.
- `demo.sh` (se ajustaron los nombres de columna, pero no se volvió a ejecutar) y `docs/Reporte.md` / `diagrama_arquitectura.md`, que describen la demo anterior (puertos 13306/13307) y no el modelo por sesión.

**Incidente corregido (importante)**
- Un `DROP DATABASE` hecho a mano en un nodo con `log-bin` activo **subió al central y borró su base**: al enlazar, el central reproducía todo el historial del nodo. Se corrigió: la primera vez arranca desde la posición actual del binlog. **Para borrar una base en un nodo enlazado, usar antes `SET sql_log_bin=0;`**. La conexión `sucursal_1` ya existente conserva su posición anterior.

**Limitaciones**
- **Windows no está automatizado:** el `my.ini` se escribe a mano (el MariaDB de ese equipo usa el puerto 3307). El asistente muestra el contenido necesario.
- **TLS por nombre:** en Windows, conectar por IP falla con «no confiable» (el certificado vale solo para el nombre `.ts.net`). El asistente no avisa si se escribe una IP.
- **`bind-address=0.0.0.0`** en los nodos y en el central: lo restringe el firewall/Tailscale, pero en una red pública el 3306 local quedaría escuchando (las cuentas de la app solo aceptan conexión local).
- **Hash de contraseñas débil** (SHA-256 con sal, sin estiramiento) y sal con `QRandomGenerator::global()` en vez de uno criptográfico.
- **Hasta 9 sucursales** (por el incremento 10 de `auto_increment`).
- El certificado de Let's Encrypt del central caduca el 2027-01-31; falta automatizar su renovación.
- Un admin teclea dos contraseñas (usuario + `farmacia_remoto`). Propuesto, sin implementar: casilla «Recordar en este equipo».
- Un `DROP`/`TRUNCATE` o cualquier DDL hecho con registro binario en un nodo también se replica: los cambios de esquema deben hacerse con `sql_log_bin=0` en cada nodo.

**Pendiente decidido (anotado por el usuario)**
- **Reporte diario de movimientos** hechos en cada sucursal (ventas, cambios, devoluciones…): por implementar.
- Pagos y reembolsos con tarjeta: implementado (ver sección 6). Falta probarlo en la interfaz. Las devoluciones en efectivo de pagos con tarjeta **hechas antes** de este cambio no se anulan ni se marcan: quedan como estaban.

**Ideas abiertas**
- Aviso en el asistente cuando la dirección del central sea una IP.
- Rol de «gerente de sucursal» (gestionar su propio personal).
- Borrar productos/empleados/usuarios en el central puede romper la réplica de una sucursal sin conexión que ya los use (FK): preferir marcarlos inactivos. Y no se debe registrar **dos equipos para la misma sucursal** (misma ranura y `server-id`); el asistente aún no lo impide.
- Productos locales por sucursal (hoy el catálogo solo lo crea el central).
- Actualizar `docs/` al modelo por sesión.
