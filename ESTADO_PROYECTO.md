# Estado del proyecto — Farmacia PDV (BD distribuidas)

Actualizado: 2026-11-15 · Última versión subida: `758b770` · Complementa a `Plan_Proyecto.md` (alcance y diseño original). Este archivo es el punto de entrada: si empiezas de cero, léelo completo antes de tocar nada.

## 1. Qué es

Sistema de punto de venta para farmacias, hecho como entregable de **Bases de Datos Distribuidas**. Una app de escritorio (Qt 6 / C++, ~9.000 líneas) trabaja sobre MariaDB y demuestra **fragmentación**, **replicación** y **transacciones distribuidas** (XA) con un esquema multi-sucursal real. El profesor pidió control de accesos y de lo que puede editar un vendedor, y aceptó un diseño sencillo si el cliente lo quiere así.

- Stack: Qt 6 (Widgets, Sql, Svg, Network), driver `QMYSQL`, MariaDB (10.11 en el central), CMake.
- Compilar en Linux: `cmake -S app -B app/build && cmake --build app/build -j"$(nproc)"` → `app/build/FarmaciaApp`. El `db/schema.sql` va **incrustado** en el ejecutable (`resources.qrc`): hay que recompilar para que un equipo nuevo cree el esquema actual.
- Windows: `.github/workflows/windows.yml` (solo manual, `workflow_dispatch`).

## 2. Arquitectura

```
                 AWS (EC2, Ubuntu)                       Tailscale (red privada, 100.64.0.0/10)
          ┌────────────────────────────┐
          │  CENTRAL — MariaDB          │◄──── replicación de bajada (catálogo, personal, cuentas, permisos)
          │  server-id 1, log-bin, ROW  │────► replicación de subida (ventas, lotes, clientes, bitácora…)
          └────────────▲───────────────┘
                       │ conexión de administración (farmacia_remoto, TLS)
        ┌──────────────┴──────────────┐
   Sucursal A (equipo + MariaDB local)   Sucursal B (equipo + MariaDB local) …
```

- **Central:** servidor en AWS con MariaDB. Consolida las ventas de todas las sucursales y es la fuente de verdad del catálogo, el personal y las cuentas.
- **Sucursales:** cada equipo conserva su MariaDB local, así que vende sin conexión y se sincroniza al volver. **Un equipo = una sucursal**, fijada la primera vez (asistente).
- **Red:** el puerto 3306 del central solo es alcanzable por Tailscale (no se abre a internet). El tráfico va cifrado en el túnel y la conexión de la app usa TLS con un certificado de Let's Encrypt emitido para el nombre `ip-172-31-41-253.tail560a17.ts.net`.
- **Modo por sesión (no por equipo):**
  - **Empleado:** trabaja sobre la base local de su sucursal.
  - **Administrador:** al iniciar sesión se conecta al central (pide la contraseña de `farmacia_remoto`) y esa conexión pasa a ser la **conexión por defecto de toda la sesión**: las pantallas, que ya usaban la conexión por defecto, ven todas las sucursales y administran catálogo, personal, sucursales y cuentas. Si no hay conexión, puede entrar en **modo local** (solo lectura de lo que administra el central).
  - **Equipo único:** sin central configurado, una sola base local con todos los permisos.
  - Implementación: `DatabaseManager::funcion()` devuelve el modo de la sesión; `activarModoCentral()` clona la conexión del central como la por defecto.

## 3. Reparto de datos (un solo escritor por tabla; lo hacen cumplir los permisos de MariaDB)

| Escribe el **central** (admin) → baja a las sucursales | Escribe la **sucursal** (empleados) → sube al central |
|---|---|
| Sucursales, Empleados, Usuarios, Asignaciones_Temporales | Clientes, Lotes (con `costo_unitario`), Inventario |
| Permisos_Temporales | Bitacora_Cambios |
| Categorías, Proveedores | Tickets, Ventas, Pagos |
| Productos (identidad + `precio_sugerido`) | Devoluciones, Detalle_Devolucion |
| | Productos_Sucursal (precio propio, activo, stock mínimo) |

- Sin fila en `Productos_Sucursal`, la sucursal vende al precio sugerido. `Ventas.precio_unitario` guarda lo cobrado, así que cambiar un precio no altera tickets viejos.
- El catálogo es único porque `codigo_barras` es único: con varios escritores dos sucursales podrían crear el mismo código a la vez.
- El central **no vende**: no tiene Punto de Venta y las pantallas operativas son de consulta.
- Las listas de tablas viven en `kTablasDelCentral` / `kTablasDeSucursal` (`dbinicializador.cpp`), y se repiten en `preparar-central.sh` y `preparar-nodo.sh`: **al agregar una tabla hay que tocar los tres**.

## 4. Replicación

- **Bajada (central → sucursal):** la sucursal replica las tablas del central (`replicate-do-table` en su `my.cnf`).
- **Subida (sucursal → central):** el central tiene una conexión con nombre por sucursal (`sucursal_<id>`), multi-origen, con `replicate_do_db=farmacia`.
- **Sin conflictos de ID:** `auto_increment_increment=10` y un `offset` distinto por nodo. El central usa offset 1; cada sucursal una *ranura* de 2 a 10 (caben 9). Los ids de sucursal los genera el central (1, 11, 21…): id 1 → ranura 2, id 11 → ranura 3. De la ranura salen `server_id = 10 + ranura`, `gtid_domain_id` y el offset.
- `binlog-do-db=farmacia` en cada nodo: con `log-bin` activo el binlog recoge **todas** las bases del MariaDB local (con ROW el filtro es por la base de la tabla).
- `binlog_format=ROW`: necesario para que los triggers (stock, subtotal, estado de pago) no se disparen dos veces.
- El esquema y las cuentas se crean con `sql_log_bin=0`: la replicación transporta solo datos.
- **Asistente de primer arranque** (automático): comprueba el MariaDB local, crea la base y la cuenta `farmacia_app`, copia el catálogo del central con la posición GTID exacta, crea las cuentas de replicación en ambos lados y arranca las dos réplicas. La primera vez, el central empieza desde la posición **actual** del binlog del nodo (ver incidente 1).
- **Único paso manual:** MariaDB no activa `log-bin` ni `server-id` en caliente. Si faltan, el asistente muestra el `my.cnf` necesario; en Linux lo escribe `db/distribuido/real/preparar-nodo.sh <ranura>`.
- **Un equipo nuevo no necesita migraciones** (el esquema incrustado ya es el actual), **salvo** que ya tenga una base `farmacia` vieja: con base existente el asistente solo crea las tablas que faltan, no las columnas. Se decidió no automatizar eso.

## 5. Cuentas y contraseñas (qué es cada una; los valores no se guardan aquí)

| Cuenta | Tipo | Cuándo se pide |
|---|---|---|
| Usuario de la app (`Usuarios`) | Aplicación, hash SHA-256 + sal | En cada inicio de sesión |
| `farmacia_remoto` | MariaDB del central | En cada inicio de sesión de **admin**; no se guarda |
| `farmacia_registro` | MariaDB del central | Solo en el asistente (una vez por equipo); no se guarda |
| `farmacia_app` | MariaDB local | La genera el asistente (aleatoria) y se guarda en `QSettings` (`~/.config/Farmacia/FarmaciaApp.conf`) |
| `repl_s<id>` / `repl_central` | Replicación | Las genera el asistente; viven solo dentro de MariaDB |

- `preparar-central.sh` imprime la contraseña de `farmacia_registro` **una sola vez** y no la guarda: anótala. Si se pierde, se reejecuta el script (acepta `PW_REGISTRO=…` para fijarla).
- Usuarios de ejemplo del central: `admin` (admin, creado a mano), `marta` (admin), `ana` y `luis` (empleados de Sucursal Principal), `jorge` (empleado de Sucursal Norte), `pedro` (inactivo) y `Javier` (creado a mano). Las contraseñas de ejemplo están en el comentario de `db/distribuido/real/datos-ejemplo-central.sql`.

## 6. Funcionalidad

### Pantallas y roles

| Pantalla | Empleado | Admin (sobre el central) |
|---|---|---|
| Inicio, Punto de Venta, Clientes, Tickets, Inventario | Solo su sucursal | Consulta de todas (no vende) |
| Productos, Categorías | Lectura; fija **su precio** con permiso temporal («Precio en mi sucursal») | CRUD del catálogo |
| Proveedores, Empleados, Sucursales, Usuarios, Asignaciones | Ocultas | CRUD |
| Permisos (temporales) · Bitácora | Ocultas | Conceder/revocar · consulta |
| Reporte diario (+ PDF) | Su sucursal | Una sucursal o todas |

Un empleado de **otra** sucursal solo entra con una *asignación temporal* vigente autorizada por un admin, y opera en la sucursal **del equipo**. Es una decisión tomada: no se implementará «el usuario decide la sucursal» (los datos de otra sucursal no existen en la base local del equipo).

### Permiso temporal «encargado»

Un empleado **sin permiso solo hace lo esencial de un vendedor**: vender y cobrar, consultar productos y existencias, ver tickets y clientes, registrar abonos y **devoluciones** (decisión: siempre permitidas). Un admin puede concederle el rol «encargado» por **1, 2 o 3 días** (24/48/72 h exactas; lo exigen la app y un `CHECK` de la base), y mientras esté vigente puede fijar el precio/«lo maneja»/mínimo de su sucursal (`Productos_Sucursal`) y dar de alta o editar lotes (con su costo).

- Se guarda en `Permisos_Temporales` (la escribe el central y baja a las sucursales); vence solo aunque no haya conexión (las fechas van en **UTC**, independiente de la zona de cada equipo) y se vuelve a comprobar al guardar. Un admin puede revocarlo antes.
- Cada cambio queda en `Bitacora_Cambios` (usuario, fecha, antes → después, si fue con permiso), dentro de la misma transacción que el cambio; sube al central y el admin la consulta en **Bitácora**.
- **Limitación:** la restricción vive en la aplicación, no en la base. MariaDB solo conoce una cuenta por equipo (`farmacia_app`), así que no distingue qué empleado escribe; alguien con acceso directo a la base del equipo podría saltársela. Igual que los roles admin/empleado.

### Pagos y reembolsos con tarjeta

La app **no cobra ni reembolsa nada con tarjeta**: la terminal lo hace y la app registra y controla.
- Al cobrar con tarjeta (contado o abono) se captura la **autorización** del comprobante (4-20 caracteres alfanuméricos; normalmente 6) y, opcionalmente, los **últimos 4 dígitos** (`Pagos.numero_referencia`, `Pagos.ultimos_digitos`).
- Un ticket pagado con tarjeta se devuelve a la tarjeta: efectivo o transferencia solo por la parte que se pagó así (el cargo seguiría en la cuenta del cliente). `cambio` y `saldo_a_favor` no mueven dinero y siguen permitidos.
- Reembolso a tarjeta en dos pasos: se registra con la *modalidad* (`cancelacion`: mismo día, antes del cierre de lote; `devolucion`: después) y queda `solicitada` («pendiente en terminal») hasta capturar el folio de la terminal (botón «Registrar folio de reembolso» en el detalle del ticket) → `completada`.
- El stock se reintegra al registrar la devolución (el producto ya volvió), aunque el reembolso siga pendiente. No se sabe si la comisión de la terminal se reembolsa: depende del contrato con el banco o procesador.

### Reporte diario de movimientos (con PDF)

Se elige la fecha (hasta hoy) y, si es un admin sobre el central, una sucursal o todas. Incluye resumen, ventas por sucursal, cobros por método, **corte de efectivo** (cobrado − devuelto, sin fondo inicial), devoluciones (con las de tarjeta pendientes de cualquier fecha), productos más vendidos, movimientos de inventario, cambios de la bitácora y alertas al cierre. **Exportar a PDF** imprime el mismo HTML sobre A4 (`QPdfWriter`, sin dependencias extra). Código: `app/src/util/reportediario.cpp` (genera el HTML) y `app/src/pages/reportediariopage.*`.
- Cada sucursal registra con la hora de su equipo (`DATE(fecha)` sobre lo guardado); la bitácora va en UTC y se convierte a hora local.
- Con la vista de todas las sucursales, los datos de una sucursal sin conexión con el central pueden estar incompletos (el propio reporte lo avisa).

### Ticket impreso y pago mixto

- **Ticket** (`util/ticketimpreso.*`, `dialogs/vistaticketdialog.*`): texto por columnas con fuente de ancho fijo (Space Mono, incluida en la app), de modo que ninguna línea se parte sola. Se ve en pantalla, se guarda en PDF de 80 mm y se imprime con `QPrinter`; se abre al cobrar y desde el detalle del ticket. Configuración por equipo en `QSettings` (clave `ticket/*`): impresora, ancho, columnas, razón social, RFC, domicilio, teléfono, pie, regla de IVA. IVA desglosado por tasa según la categoría del producto (`medicamento` a 0 %): aproximación por validar con un contador. Requiere `Qt6::PrintSupport`.
- **Pago mixto** (`dialogs/pagomixtodialog.*`): venta de contado con tarjeta + transferencia + efectivo; el efectivo es lo que falta. Una fila de `Pagos` por método (efectivo sin el cambio). Las devoluciones respetan la vía de pago. Sin cambios de esquema.

### Diseño de la interfaz

- **Menú lateral agrupado** (Ventas, Inventario, Administración). Cada entrada guarda en `Qt::UserRole` el índice de su página en el `QStackedWidget`, porque el menú ya no coincide 1 a 1 con él; las secciones no son seleccionables.
- **Encabezado por pantalla** con título y una línea de ayuda (el Inicio no lo muestra; las páginas con `labelAyuda` la aportan al encabezado y la ocultan).
- **Botones con tres pesos:** principal (acento; también el botón por defecto de cada diálogo), secundario (gris pizarra) y destructivo (rojo). Son de color sólido porque los iconos son blancos. **Cada variante declara su propio `:disabled`**: un selector por nombre (`QPushButton#x`) le gana a `QPushButton:disabled`.
- **Insignia de sesión** en la barra superior: rol y alcance, «Encargado hasta …» y estado de la réplica con color; se refresca cada 10 s.

## 7. Estructura del repositorio

```
app/                     Aplicación Qt (src/{database,dialogs,pages,util}, resources/)
db/schema.sql            Esquema completo (tablas + triggers); incrustado en el ejecutable
db/seed.sql              Datos de ejemplo para un equipo único
db/migraciones/          002 precios por sucursal · 003 permisos y bitácora · 004 tarjeta (+ aplicar-00N.sh)
db/distribuido/real/     Central real: preparar-central.sh, preparar-nodo.sh, reparar-central.sh, datos-ejemplo-central.sql
db/distribuido/          Demos locales: fragmentación, XA, autonomía (demo.sh con 3 nodos)
docs/                    MANUAL.md (técnico, de usuario y plan de pruebas para Windows), Reporte.md y diagramas (describen la demo anterior)
Plan_Proyecto.md         Alcance y diseño · Farmacia_actualizado.md: esquema base
```

## 8. Operación

**Orden al desplegar un cambio de esquema** (con replicación por filas los esquemas no pueden diferir y ningún nodo debe guardar un valor nuevo antes de que todos lo admitan):
1. `aplicar-00N.sh nodo` en cada equipo y `…central` en el servidor (o el `.sql` por `ssh … 'sudo mariadb' < …`).
2. `preparar-central.sh` (permisos de las cuentas del central) y, si cambió la lista de tablas que bajan, `preparar-nodo.sh <ranura>` otra vez en cada equipo (reescribe el `my.cnf` y reinicia MariaDB).
3. Solo entonces usar la versión nueva de la app.

**Agregar un equipo:**
1. Unirlo a **tu tailnet** (invitarlo con su propio correo en Tailscale → Users; *compartir solo el nodo* no basta: el central no puede iniciar conexiones hacia un nodo de otro tailnet y la replicación de subida fallaría).
2. Preparar su MariaDB local (`preparar-nodo.sh` o el `my.cnf` del asistente).
3. Ejecutar el asistente: dirección del central = **nombre `.ts.net`** (no la IP, ver limitaciones), cuenta y contraseña de registro, y su sucursal. Un equipo por sucursal.

**Comprobaciones de solo lectura útiles:**
- Central: `SHOW ALL SLAVES STATUS\G` (por sucursal: ambos hilos en `Yes`, `Last_*_Error` vacío, `Seconds_Behind_Master`).
- Equipo local: `SHOW ALL REPLICAS STATUS\G` (la barra de estado y la insignia usan lo mismo).
- Tablas y permisos: `SHOW TABLES FROM farmacia`, `SHOW GRANTS FOR 'farmacia_remoto'@'100.64.0.0/255.192.0.0'`.

**Datos de ejemplo del central:** `ssh <central> 'sudo mariadb' < db/distribuido/real/datos-ejemplo-central.sql` (idempotente; requiere la migración 002).

## 9. Entorno y notas para quien continúe

- **Quién y cómo:** el usuario escribe y espera respuestas en **español neutro** (tú; nunca voseo). Su shell de login es **fish**: no hay heredocs ni `VAR=valor` (usar `set VAR valor`; para pasar SQL, `echo "…" | ssh …`). Dentro de comillas simples de `ssh` manda bash del servidor.
- **`sudo` y el central:** el asistente de código **no tiene `sudo`** en el equipo ni permiso para escribir en el central por SSH (se bloqueó); lo hace el usuario en su terminal (`! comando` no sirve para `sudo`, no hay terminal para la contraseña). Las comprobaciones de solo lectura por SSH sí se permitieron.
- **Central:** `ubuntu@100.106.156.124` (nombre `ip-172-31-41-253.tail560a17.ts.net`), MariaDB 10.11.14. La llave está en `~/Desktop/AWS/farmacia_aws.pem` con permisos 755: SSH la rechaza, hay que copiarla (`install -m 600 … /tmp/k.pem`). Borrar la copia al terminar. Hay otra llave y un certificado ahí; no se versionan (`.gitignore` excluye `*.pem`, `*.key`).
- **Nodos conocidos:** el portátil de desarrollo (Linux/CachyOS, `catnvrnmnt`) es la `Sucursal Principal` (id 1, ranura 2, conexión `sucursal_1`), con un MariaDB 13.x local; un equipo Windows (`cat`, MariaDB en el puerto **3307**) quedó registrado como `Sucursal Norte` (id 11, `sucursal_11`) y lleva días fuera de la red; hay un tercer nodo Windows (`alejandro`) de **otra cuenta** de Tailscale que no se reconoce.
- **Tailscale en el portátil:** `tailscaled` no arranca solo tras reiniciar (`sudo systemctl enable --now tailscaled`; después `sudo tailscale up`). Tras actualizar el kernel hay que reiniciar o falla el módulo `tun`. Hay un aviso de MagicDNS (systemd-resolved con NetworkManager) que no impide usar los nombres `.ts.net`.
- **Cómo se verificó lo que no se puede probar a mano:** MariaDB desechables (`mariadb-install-db --no-defaults` + `mariadbd` en un puerto alto y un socket propio) para validar esquema, migraciones, `CHECK`s y consultas; un arnés Qt aparte (no versionado) que enlaza las fuentes de la app y dibuja con `QT_QPA_PLATFORM=offscreen` para obtener capturas y para generar el reporte/PDF contra datos de ejemplo.

## 10. Estado por nodo (último verificado; revalidar antes de fiarse)

| Nodo | 002 | 003 | 004 | Notas |
|---|---|---|---|---|
| Central | aplicada | aplicada (15-nov) | aplicada (15-nov) | Datos de ejemplo cargados; réplicas `sucursal_1` (activa, retraso 0) y `sucursal_11` (esperando al equipo Windows) |
| Portátil Linux (Principal) | aplicada | aplicada | aplicada (15-nov) | Réplica de bajada recuperada tras aplicar la 003; `my.cnf` con `binlog-do-db` |
| Equipo Windows (Norte) | desconocido | desconocido | desconocido | Sin conexión desde hace días; con una base vieja habrá que aplicar 002, 003 y 004 y regenerar su `my.ini` (con `binlog-do-db` y `Permisos_Temporales`) |

La **005** (totales `VIRTUAL`, ver incidente 7) está preparada y **pendiente** en todos los nodos. La 004 falta solo en el equipo Windows (sin conexión: `sucursal_11` no conecta, «Connection timed out» a su puerto 3307). Hasta aplicarla ahí, el cobro con tarjeta y las devoluciones de la versión nueva fallan en ese equipo (columnas y valor de enum inexistentes). Verificada en el central el 15-nov: columnas de tarjeta presentes y `sucursal_1` al día.

## 11. Incidentes y lecciones

1. **Un `DROP DATABASE` hecho a mano en un nodo con `log-bin` subió al central y le borró su base.** Al enlazar, el central reproducía todo el historial del nodo. Corregido: la primera vez arranca desde la posición actual del binlog. **Para borrar una base en un nodo enlazado, usar antes `SET sql_log_bin=0;`** (cualquier DDL con binlog también se replica). La conexión `sucursal_1` previa conserva su posición anterior.
2. **Otra base del equipo (de otro proyecto) detuvo la réplica del central** (`Table 'buchanan_quiz.usuarios' doesn't exist`): el binlog incluye todas las bases. Corregido con `binlog-do-db=farmacia` en el nodo y `replicate_do_db=farmacia` en la conexión del central (este filtro dinámico se pierde si el central reinicia; el del nodo basta).
3. **Una migración sin aplicar en un nodo detuvo su réplica de bajada** (`Table 'farmacia.Permisos_Temporales' doesn't exist`) y la app trataba el permiso como inexistente (la cuenta de la app no podía leer la tabla). Lección: migrar **todos** los nodos antes de que el central escriba datos nuevos.
4. **«No trusted» en Windows al conectar por IP:** el certificado vale solo para el nombre `.ts.net`; Windows verifica nombre y cadena, el cliente de Linux no. Usar siempre el nombre.
5. **Botón Eliminar deshabilitado que parecía activo:** el selector por nombre del QSS le ganaba a `:disabled`; ver diseño.
6. **Contraseña de registro «perdida»:** `preparar-central.sh` la imprime una sola vez. No se guarda en ningún archivo; anotarla.

7. **`Tickets.total` y `saldo_pendiente` llegaban en 0.00 al central.** Eran columnas calculadas **guardadas** (`STORED`). El nodo (MariaDB 13.0.2) inserta el ticket con subtotal 0 y un trigger lo actualiza después; el central (10.11.14) no recalcula esas columnas al aplicar las actualizaciones por filas. **Reproducido** con un 10.11.14 oficial (firma GPG verificada) como central y un 13.0.2 como nodo; con 13.0.2 en ambos lados no falla. Solución: la migración 005 las pasa a `VIRTUAL` (se calculan al leer, no se replican); `schema.sql` ya las define así. Probado: idempotente, esquema nuevo = migrado en las 18 tablas, venta contado/crédito/abonos/tarjeta replicada bien, y la réplica no se rompe con un solo lado convertido (en cualquier orden; conviene el central primero). Sin determinar si la causa es la versión del esclavo o la mezcla de versiones. `Ventas.subtotal` también es `STORED` pero nada la actualiza después de insertarla, así que no se ve afectada.

## 12. Limitaciones conocidas

- **Windows no está automatizado:** el `my.ini` se escribe a mano (el asistente muestra el contenido).
- **TLS por nombre:** el asistente no avisa si se escribe una IP en lugar del nombre `.ts.net`.
- **`bind-address=0.0.0.0`** en nodos y central: lo restringe el firewall/Tailscale, pero en una red pública el 3306 local quedaría escuchando (las cuentas de la app solo aceptan conexión local; la de replicación, solo el rango de Tailscale).
- **Hash de contraseñas débil** (SHA-256 con sal, sin estiramiento) y sal con `QRandomGenerator::global()` en vez de uno criptográfico.
- **Hasta 9 sucursales** (incremento 10 de `auto_increment`).
- El certificado de Let's Encrypt del central caduca el **2027-01-31**; falta automatizar la renovación.
- Un admin teclea dos contraseñas (usuario + `farmacia_remoto`) a propósito: guardar la segunda en el equipo la expondría a cualquiera con acceso al archivo de ajustes.
- Borrar un producto/empleado/usuario en el central puede romper la réplica de una sucursal sin conexión que ya los use (clave foránea). Y **no se debe registrar dos equipos para la misma sucursal** (misma ranura y `server-id`); el asistente aún no lo impide.
- Las devoluciones en efectivo de pagos con tarjeta **anteriores** al cambio de tarjeta no se anulan ni se marcan.

## 13. Verificado

- Compilación limpia en Linux; asistente, login de admin contra el central, concesión de un permiso temporal y su efecto en una sesión de empleado (`ana`) comprobados por el usuario.
- Migraciones 002, 003 y 004, datos de ejemplo, `CHECK` de 72 h, vigencia en cualquier zona horaria y cálculo de reembolsos por vía de pago: probados en MariaDB desechables (idempotentes y con esquema idéntico al de una instalación nueva).
- Ticket y pago mixto: arnés Qt contra MariaDB desechable (cobro mixto de punta a punta con sus 8 validaciones, devoluciones sobre un ticket mixto, reimpresión, configuración, PDF de 80 mm y salida por `QPrinter` a PDF); 2,400 tickets aleatorios sin ninguna línea más ancha que sus columnas. **Sin impresora térmica real.**
- Reporte diario y PDF: generados contra datos de ejemplo y un día vacío (cifras coinciden); pantalla, menú agrupado, encabezados, jerarquía de botones, insignia y diálogos: capturas con el arnés sin pantalla.
- **Sin probar de punta a punta:** una venta de empleado que llegue al central por la réplica de subida; el flujo completo de tarjeta (cobro con autorización, devolución solicitada, folio, completada) en la interfaz real; `demo.sh` (solo se ajustaron nombres de columna); `docs/Reporte.md` y `diagrama_arquitectura.md` describen la demo anterior (puertos 13306/13307), no el modelo por sesión.

## 14. Pendiente e ideas

**Para cerrar el despliegue**
- Aplicar la migración 004 en el equipo Windows (y 002/003 antes, si conserva una base vieja); revisar que su firewall deje entrar el puerto de MariaDB desde `100.64.0.0/10`.
- Probar en la interfaz el flujo de tarjeta y la venta de subida.

**Planeado (ver `Plan_Proyecto.md`, sección 6)**
- **Implementado el 2026-11-20 (sin commitear):** ticket de **80 mm** (texto por columnas, 42 por defecto; vista en pantalla, PDF e impresión por el driver) y **pago mixto**. Pendiente: fase ESC/POS (corte y cajón), probar con impresora real, tasa de IVA por producto, datos fiscales en la base y comprobante de devolución aparte; lo legal está por validar con un contador. Ver `Plan_Proyecto.md` §6.7.
- **Aplicar la migración 005** (primero en el central, luego en cada nodo): `sudo db/migraciones/aplicar-005.sh`. Corrige el defecto de los totales en 0.00 en el central (incidente 7).

**Ideas abiertas (no pedidas aún)**
- Gráfica de ventas de 7 días en el Inicio y modo oscuro.
- Aviso en el asistente cuando la dirección del central sea una IP; impedir un segundo equipo para la misma sucursal; detectar una base con esquema viejo (propuesto y descartado por ahora).
- Marcar como inactivos (no borrar) productos/empleados/usuarios en el central.
- Casilla «Recordar en este equipo» para la contraseña de `farmacia_remoto` (opt-in).
- Rol de «gerente de sucursal» (gestionar su propio personal); productos locales por sucursal.
- Actualizar `docs/` (Reporte, diagramas) al modelo por sesión.
