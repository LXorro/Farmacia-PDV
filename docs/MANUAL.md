# Farmacia PDV — Manual técnico, manual de usuario y plan de pruebas

Versión del documento: 2026-11-15 · Aplica a la versión de la app del commit `758b770` (o posterior) · Pensado para quien instale, configure y pruebe equipos **Windows**.

> Las contraseñas **no** están en este documento (este archivo se sube a GitHub). Las recibes por separado en `docs/CREDENCIALES.local.md`, que Git ignora. En el texto aparecen como `<REGISTRO>`, `<REMOTO>`, `<ADMIN>` y `<ROOT_LOCAL>`.

---

## 0. Antes de tocar nada: las 6 reglas de oro

1. **Una sucursal = un equipo.** Nunca registres dos equipos para la misma sucursal (compartirían ranura y `server-id` y la réplica se rompe). Para repetir una instalación en el *mismo* equipo, sigue la sección 5 (reinicio), no registres otro.
2. **Nunca ejecutes `DROP DATABASE` sin antes escribir `SET SESSION sql_log_bin=0;` en la *misma* sesión.** Un nodo enlazado envía todo lo que ocurra en la base `farmacia` al central; un `DROP DATABASE` normal **borra también la base del central** (ya pasó una vez). Y comprueba siempre que estás en *tu* MariaDB local (`SELECT @@server_id;` debe dar un número ≥ 12, **nunca 1**: el 1 es el central).
3. **Escribe siempre el nombre del central, no su IP:** `ip-172-31-41-253.tail560a17.ts.net`. Con la IP, Windows responde «not trusted» porque el certificado solo es válido para el nombre.
4. **Apunta la contraseña de `farmacia_registro`.** No se guarda en ningún sitio de la app. Si se pierde, se restablece en el central (sección 6.5).
5. **Aplica los cambios de esquema en todos los nodos antes de usar la app nueva** (sección 7). Un nodo sin migrar detiene su réplica.
6. **Cierra la app antes de reiniciar o borrar nada** (la app corre en pantalla completa: `Alt+F4` la cierra).

---

## 1. Qué es el sistema (en un minuto)

Punto de venta para farmacias con base de datos **distribuida**:

```
        AWS (Ubuntu) ── CENTRAL: MariaDB 10.11  (catálogo, personal, cuentas, ventas consolidadas)
              ▲  ▼        Tailscale (red privada 100.64.0.0/10) — el 3306 NO está en internet
   ┌──────────┴──┴───────────┐
   Sucursal A (PC + MariaDB local)   Sucursal B (PC + MariaDB local) …  (máx. 9)
```

- **Central:** fuente de verdad del **catálogo**, **personal**, **sucursales**, **cuentas** y **permisos temporales**. No vende.
- **Sucursal (cada PC):** tiene su propia base MariaDB. Vende aunque no haya internet; lo que vende **sube** al central y el catálogo **baja** del central, por replicación automática.
- **Empleado** → trabaja sobre la base local de su sucursal. **Administrador** → al iniciar sesión se conecta al central (pide otra contraseña) y ve todas las sucursales.
- **Equipo único** (sin central): una sola base local con todos los permisos; sirve para pruebas y demostraciones.

Quién escribe qué (lo hace cumplir MariaDB con permisos):

| Escribe el **central** (admin) y baja a las sucursales | Escribe la **sucursal** y sube al central |
|---|---|
| Sucursales, Empleados, Usuarios, Asignaciones temporales, Permisos temporales, Categorías, Proveedores, Productos (identidad y precio sugerido) | Clientes, Lotes, Inventario, Tickets, Ventas, Pagos, Devoluciones, Detalle de devolución, Productos por sucursal (precio propio), Bitácora de cambios |

---

## 2. Datos de referencia

### 2.1 Central

| Dato | Valor |
|---|---|
| Nombre (el que se escribe en el asistente) | `ip-172-31-41-253.tail560a17.ts.net` |
| Puerto MariaDB | `3306` |
| IP Tailscale (solo para SSH/diagnóstico) | `100.106.156.124` |
| Cuenta de registro (asistente) | `farmacia_registro` / `<REGISTRO>` |
| Cuenta de administración remota (cada inicio de sesión de admin) | `farmacia_remoto` / `<REMOTO>` |
| Certificado TLS | Let's Encrypt; **caduca el 2027-01-31** |

### 2.2 Sucursales y ranuras

La **ranura** (2 a 10) sale del id de la sucursal; el asistente la calcula y la muestra si hace falta. Determina `server-id`, `gtid_domain_id` y `auto_increment_offset`.

| id_sucursal | 1 | 11 | 21 | 31 | 41 | 51 | 61 | 71 | 81 |
|---|---|---|---|---|---|---|---|---|---|
| Ranura | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 |
| `server-id` | 12 | 13 | 14 | 15 | 16 | 17 | 18 | 19 | 20 |

Estado conocido (revísalo en la pantalla **Sucursales** antes de elegir):

| Sucursal | id | Estado |
|---|---|---|
| Sucursal Principal | 1 | **Ocupada** (equipo Linux de desarrollo) |
| Sucursal Norte | 11 | **Ocupada** (equipo Windows ya registrado, puerto local 3307) |
| Sucursal Sur | 21 | Existe como **inactiva**: el asistente solo lista las **activas**; un administrador debe activarla o crear otras |

Para tus instalaciones necesitas sucursales libres: pide a un admin que cree/active las que haga falta (pantalla **Sucursales**, con la sesión de admin sobre el central).

### 2.3 Usuarios de prueba (central)

| Usuario | Rol | Sucursal | Contraseña |
|---|---|---|---|
| `admin` | admin | (todas) | `<ADMIN>` |
| `marta` | admin | (todas) | `Marta2026` |
| `ana` | empleado | Principal | `Ana2026` |
| `luis` | empleado | Principal | `Luis2026` |
| `jorge` | empleado | Norte | `Jorge2026` |
| `pedro` | empleado **inactivo** | — | `Pedro2026` |

Un empleado solo entra en el equipo de **su** sucursal, salvo que un admin le autorice una **asignación temporal** vigente a la del equipo.

### 2.4 Qué contraseña pide cada pantalla

| Momento | Qué se escribe |
|---|---|
| Asistente → «cuenta administradora del MariaDB de este equipo» | `root` / `<ROOT_LOCAL>` (la del MariaDB que instalaste en ese PC). Se usa una vez, no se guarda |
| Asistente → «cuenta de registro» | `farmacia_registro` / `<REGISTRO>`. Una vez por equipo, no se guarda |
| Inicio de sesión (todos) | usuario y contraseña de la app |
| Inicio de sesión de **admin** (segunda ventana) | `<REMOTO>` (cuenta `farmacia_remoto`). Cada vez; no se guarda |

Por eso un administrador teclea **dos** contraseñas a propósito.

---

# PARTE A — MANUAL TÉCNICO

## 3. Requisitos de cada equipo Windows

| Requisito | Detalle |
|---|---|
| Windows | 10 u 11, 64 bits, cuenta con permisos de administrador (para editar `my.ini`, reiniciar el servicio y abrir el firewall) |
| MariaDB Server | 10.11 o superior (misma serie que el central o más nueva). Instalado como **servicio**, con contraseña de `root` conocida |
| Tailscale | Instalado, con sesión iniciada **en el tailnet del proyecto** (ver 4.2) |
| La app | Carpeta `FarmaciaApp-windows` (ver 4.3), con `FarmaciaApp.exe`, la carpeta `sqldrivers\` y las DLL |
| Pantalla | 1366×768 o mayor (la app abre a pantalla completa) |
| Hora | Zona horaria y hora correctas (sincronizar con Windows) |

## 4. Instalación de un equipo nuevo (con central)

Haz los pasos **en orden**. Si algo falla, no sigas: ve a la sección 6.

### 4.1 MariaDB local

1. Instala MariaDB Server (instalador MSI oficial). Marca **«Install as service»**, anota el **puerto** (3306 por defecto; si ese puerto ya está usado por otra instalación, usa 3307) y la contraseña de `root` (`<ROOT_LOCAL>`).
2. Comprueba que el servicio corre: `services.msc` → «MariaDB» en *En ejecución*, o en `cmd`:
   ```
   sc query MariaDB
   ```
   (el nombre del servicio puede ser `MySQL` o `MariaDB`; busca el que corresponda).
3. Comprueba que entras (cmd; ajusta la ruta a tu versión):
   ```
   "C:\Program Files\MariaDB 11.4\bin\mariadb.exe" -u root -p -h 127.0.0.1 -P 3306 -e "SELECT VERSION(), @@port;"
   ```

### 4.2 Tailscale

1. Instala Tailscale e inicia sesión **con la cuenta que reciba la invitación al tailnet** del responsable (consola de Tailscale → *Users* → *Invite users*). **No basta con «compartir el nodo»**: el central tiene que poder abrir conexiones hacia tu equipo (replicación de subida), y eso solo ocurre dentro del mismo tailnet.
2. Comprueba: icono de Tailscale en *Connected*, y en `cmd`:
   ```
   tailscale ip -4
   tailscale ping ip-172-31-41-253.tail560a17.ts.net
   ```
   La IP debe empezar con `100.` y el ping debe responder (`pong`). Si no resuelve el nombre, activa **MagicDNS** en la consola de Tailscale (DNS).
3. Comprueba el puerto del central (PowerShell):
   ```
   Test-NetConnection ip-172-31-41-253.tail560a17.ts.net -Port 3306
   ```
   `TcpTestSucceeded : True` es lo esperado.

### 4.3 La app

1. Descarga el artefacto `FarmaciaApp-windows` de GitHub → *Actions* → «Windows build» (corrida más reciente, **verde**; se lanza a mano con *Run workflow*). Descomprímelo en una carpeta con ruta corta y sin permisos especiales, por ejemplo `C:\FarmaciaApp\`.
2. Verifica que existen `FarmaciaApp.exe`, `sqldrivers\qsqlmysql.dll`, `libmariadb.dll` y las DLL de Qt (`Qt6Core.dll`, `Qt6Sql.dll`, `Qt6Network.dll`, `Qt6Widgets.dll`, `Qt6Svg.dll`…).
3. Si Windows muestra SmartScreen («Windows protegió su PC»): *Más información → Ejecutar de todas formas*.
4. **No la abras todavía** (si la abres antes de tiempo, el asistente fallará en el paso del `my.ini` y tendrás que repetirlo; no pasa nada grave, pero te ahorras un ciclo).

### 4.4 Preparar `my.ini` (el único paso manual)

MariaDB no activa el registro binario ni el `server-id` en caliente: hay que escribirlos en su archivo de configuración y reiniciar.

1. Averigua tu **ranura** (tabla 2.2) a partir del id de la sucursal que usarás. En esta guía: `RANURA`.
2. Abre **como administrador** el archivo `my.ini` que está en la carpeta `data` de MariaDB, normalmente:
   `C:\Program Files\MariaDB 11.4\data\my.ini`
   (Notepad → *Ejecutar como administrador* → Abrir). Si no sabes dónde está: `SELECT @@datadir;` en el cliente.
3. Dentro de la sección `[mysqld]` agrega estas líneas (sustituye `RANURA` y `12` = `10 + RANURA`):
   ```
   server-id=12
   gtid_domain_id=12
   log-bin=farmacia-bin
   binlog_format=ROW
   binlog-do-db=farmacia
   auto_increment_increment=10
   auto_increment_offset=2
   bind-address=0.0.0.0
   replicate-do-table=farmacia.Sucursales
   replicate-do-table=farmacia.Empleados
   replicate-do-table=farmacia.Usuarios
   replicate-do-table=farmacia.Asignaciones_Temporales
   replicate-do-table=farmacia.Permisos_Temporales
   replicate-do-table=farmacia.Categorias
   replicate-do-table=farmacia.Proveedores
   replicate-do-table=farmacia.Productos
   ```
   Ejemplo completo para la **ranura 3** (sucursal id 11): `server-id=13`, `gtid_domain_id=13`, `auto_increment_offset=3`.
4. **Guarda sin BOM** (en el Bloc de notas moderno: *UTF-8*; en versiones viejas elige *ANSI*). Con BOM, MariaDB puede ignorar la primera línea.
5. **Reinicia** el servicio (cmd como administrador; usa el nombre de tu servicio):
   ```
   net stop MariaDB
   net start MariaDB
   ```
   Si no arranca, revisa el archivo `*.err` de la carpeta `data`: casi siempre es una errata en el `my.ini`. **No sigas** hasta que el servicio quede en ejecución.
6. Verifica (debe dar `1 | 12 | 2 | 10 | ROW`, con tus valores):
   ```
   "C:\Program Files\MariaDB 11.4\bin\mariadb.exe" -u root -p -h 127.0.0.1 -P 3306 -e "SELECT @@log_bin,@@server_id,@@auto_increment_offset,@@auto_increment_increment,@@binlog_format;"
   ```
7. Si ya existía una instalación con **otra base de datos** en este mismo MariaDB (de otro proyecto), no pasa nada: `binlog-do-db=farmacia` impide que sus cambios suban al central.

### 4.4b Guía de replicación de MariaDB (qué se configura y por qué)

**Aviso importante:** el mensaje «Falta preparar MariaDB» del asistente está escrito para **Linux** (habla de `/etc/my.cnf.d/farmacia-nodo.cnf` y de `sudo systemctl restart mariadb`). En **Windows** no apliques esos pasos: el equivalente es editar `my.ini` (4.4) y reiniciar el servicio con `net stop` / `net start`. El **contenido** de las opciones es idéntico en ambos sistemas.

#### Qué hace cada opción

| Opción | Para qué sirve | Si falta o está mal |
|---|---|---|
| `server-id=10+ranura` | Identidad única del nodo en la replicación (el central usa 1) | Dos nodos con el mismo id se pisan; el asistente no continúa |
| `gtid_domain_id=10+ranura` | Cada nodo es su propio «dominio» de transacciones (GTID) | Posiciones de réplica ambiguas |
| `log-bin=farmacia-bin` | Activa el registro binario; sin él el central no puede leer las ventas | «El MariaDB local no tiene registro binario (log-bin) activo» |
| `binlog_format=ROW` | Registra filas, no sentencias: evita que los triggers (stock, subtotales) se ejecuten dos veces | El asistente lo rechaza si no es `ROW` |
| `binlog-do-db=farmacia` | Solo la base `farmacia` entra al registro; otras bases del equipo no llegan al central | Un cambio en otra base detendría la réplica del central |
| `auto_increment_increment=10` y `auto_increment_offset=ranura` | Cada nodo genera ids distintos (central: 1, 11, 21…; ranura 2: 2, 12, 22…) | Choques de ids al subir ventas |
| `bind-address=0.0.0.0` | MariaDB acepta conexiones por la red (la de Tailscale) y no solo locales | El central no puede conectarse a este equipo |
| `replicate-do-table=farmacia.<tabla>` (8 líneas) | Este equipo solo aplica del central las tablas del catálogo/personal | Podría intentar aplicar tablas que no le corresponden |

Se calcula así: **ranura** = tabla 2.2 (2 a 10) según el id de la sucursal; `server-id` = 10 + ranura.

#### Windows (resumen)

1. Editar `my.ini` (4.4), guardar sin BOM.
2. `net stop MariaDB` y `net start MariaDB` (cmd como administrador).
3. Abrir el puerto en el firewall (4.5).
4. Verificar con `SELECT @@log_bin,@@server_id,@@auto_increment_offset,@@auto_increment_increment,@@binlog_format;`.

#### Linux

Con el script del repositorio (hace los pasos 1 y 2; la ranura la indica el asistente):
```
sudo db/distribuido/real/preparar-nodo.sh <ranura>
```
o a mano: guardar las mismas líneas en `/etc/my.cnf.d/farmacia-nodo.cnf` (Arch/Fedora) o `/etc/mysql/mariadb.conf.d/60-farmacia-nodo.cnf` (Debian/Ubuntu), y `sudo systemctl restart mariadb`. Después abrir el puerto si hay firewall (`ufw`/`firewalld`) y verificar con la misma consulta.

#### Qué automatiza el asistente (no lo repitas a mano)

Cuando pulsas **Configurar**, con el registro binario apagado en el nodo (nada de esto viaja por la red), el asistente:
1. Crea la base `farmacia` y su esquema y la cuenta local `farmacia_app` (contraseña aleatoria).
2. En el **central** crea `repl_s<id>` (para que este nodo lea el binlog del central) y en el **nodo** crea `repl_central` (para que el central lea el de este nodo). Ambas con permiso solo de replicación y solo desde la red de Tailscale.
3. Copia las 8 tablas del central con la **posición exacta del binlog** (así la réplica continúa justo desde ahí).
4. Arranca la réplica de **bajada** en el nodo (`CHANGE MASTER … START SLAVE`).
5. Crea o actualiza en el central la réplica de **subida** `sucursal_<id>` y la arranca. La primera vez empieza desde la posición **actual** del binlog del nodo, de modo que nada anterior al enlace sube al central.

Por eso, **después** del asistente no hay que escribir `CHANGE MASTER` ni crear cuentas a mano. Hacerlo por tu cuenta es la forma más fácil de dejar una réplica inconsistente.

#### Comprobación (solo lectura)

En el nodo: `SHOW ALL REPLICAS STATUS\G`. En el central: `SHOW ALL SLAVES STATUS\G`. En ambos, `Slave_IO_Running: Yes`, `Slave_SQL_Running: Yes`, errores vacíos y `Seconds_Behind_Master: 0`.

#### Si algo se rompe

- Tras corregir la causa (red, firewall, migración pendiente): `STOP SLAVE; START SLAVE;` (en el central: `STOP SLAVE 'sucursal_<id>'; START SLAVE 'sucursal_<id>';`).
- No uses `RESET SLAVE`, `RESET MASTER`, `DROP DATABASE` ni `CHANGE MASTER` sin leer la sección 5 y el Anexo A.
- Tabla de síntomas y causas: secciones 6.2 y 6.3.

### 4.5 Firewall de Windows

El central debe poder conectarse **a tu** MariaDB por la red de Tailscale. En PowerShell como administrador (cambia el puerto si no es 3306):

```
New-NetFirewallRule -DisplayName "MariaDB Farmacia (Tailscale)" -Direction Inbound -Protocol TCP -LocalPort 3306 -RemoteAddress 100.64.0.0/10 -Action Allow -Profile Any
```

Sin esta regla la instalación **parece** correcta, pero la réplica de subida queda con «Can't connect to MySQL server» y las ventas no llegan al central.

### 4.6 Asistente de primer arranque

Abre `FarmaciaApp.exe`. Si la app no encuentra una base válida, aparece «Configurar base de datos — Farmacia».

| Campo | Qué poner |
|---|---|
| Usuario / contraseña de administración de MariaDB | `root` / `<ROOT_LOCAL>` |
| Puerto (local) | El de tu MariaDB (3306 o 3307). **El valor por defecto es 3306** |
| ☑ Conectar este equipo con un central | Márcala |
| Dirección del central | `ip-172-31-41-253.tail560a17.ts.net` (**nombre, no IP**) |
| Puerto del central | `3306` |
| Cuenta de registro | `farmacia_registro` |
| Contraseña de registro | `<REGISTRO>` |
| Sucursal | Pulsa **Consultar** y elige la tuya (solo salen las *activas*) |

Pulsa **Configurar**. El texto de estado avanza: «Comprobando la configuración de MariaDB…» → «Preparando la base de datos local…» → «Configurando la replicación con el central…». Qué hace por dentro (todo con el registro binario apagado en el nodo, para que nada de esto viaje por la red): crea la base `farmacia` y su esquema, crea la cuenta local `farmacia_app` con contraseña aleatoria, copia el catálogo del central con la posición exacta del binlog, crea las cuentas de replicación y arranca las dos réplicas.

- Si aparece **«Falta preparar MariaDB»**, es el paso 4.4 (incompleto); el texto del mensaje es de Linux, en Windows sigue 4.4 y la guía 4.4b. Corrígelo y vuelve a pulsar *Configurar*.
- Si aparece **«Este equipo no tiene dirección de Tailscale»**, abre Tailscale y conéctate.
- Al terminar, la app pasa sola al inicio de sesión.

### 4.7 Verificación de la instalación (hazla siempre)

1. **En el equipo:** inicia sesión con un empleado de **esa** sucursal. En la barra superior la insignia de sesión debe mostrar el rol y la réplica en verde.
2. **En MariaDB local** (cliente como root):
   ```
   SHOW ALL REPLICAS STATUS\G
   ```
   Debe verse `Slave_IO_Running: Yes`, `Slave_SQL_Running: Yes`, `Last_IO_Error`/`Last_SQL_Error` vacíos, `Seconds_Behind_Master: 0`.
3. **En el central** (SSH, lo hace el responsable):
   ```
   sudo mariadb -e "SHOW ALL SLAVES STATUS\G" | grep -E "Connection_name|Slave_(IO|SQL)_Running|Last_(IO|SQL)_Error|Seconds_Behind"
   ```
   Debe existir una conexión `sucursal_<id>` con ambos hilos en `Yes`.
4. Haz la **prueba de humo** (sección 11). El instalado se considera bueno solo si pasa completa.

### 4.8 Ticket e impresora (opcional)

1. Instala la impresora térmica en Windows con el driver del fabricante y fija en sus preferencias el papel de **80 mm** (rollo). Imprime la página de prueba de Windows.
2. En la app, abre cualquier ticket → **Ver / imprimir ticket** → **Configurar…**: elige la impresora, deja 80 mm y **42 columnas**, y captura razón social, RFC y domicilio.
3. Para calibrar, imprime un ticket con una línea larga: si se corta por la derecha, baja las columnas a 40; si sobra mucho margen, súbelas (hasta 48).
4. Sin impresora, todo funciona con **Guardar PDF…**. La configuración vive en el registro de Windows (`HKCU\Software\Farmacia\FarmaciaApp`, clave `ticket`): el reinicio de nivel 1 (sección 5) también la borra.

## 4b. Variante: equipo único (sin central)

Para demostraciones o pruebas locales sin red:

1. Instala MariaDB (4.1). **No necesitas** Tailscale, `my.ini` ni firewall.
2. Abre la app, escribe `root` / `<ROOT_LOCAL>` y el puerto, **deja desmarcada** la casilla del central y pulsa **Configurar**.
3. La base queda vacía: en el inicio de sesión aparece **«Crear administrador inicial»** (solo cuando no hay usuarios). Crea el admin y entra.
4. Datos de ejemplo (opcional): con el cliente de MariaDB, `USE farmacia;` y ejecuta `db\seed.sql`.
5. En este modo no hay replicación; nada sube a ningún lado.

---

## 5. Reiniciar un equipo (volver a hacer el setup)

Hay **tres niveles**. Usa el más bajo que resuelva tu caso. **Cierra la app antes.**

### Nivel 1 — Reiniciar solo la configuración de la app (recomendado, seguro)

Hace que el asistente vuelva a aparecer. **No toca ninguna base de datos**: lo local y el central quedan intactos, y las réplicas siguen donde iban.

En `cmd`:

```
reg delete "HKCU\Software\Farmacia" /f
rmdir /s /q "%APPDATA%\Farmacia"
```

- La app guarda sus ajustes en `HKCU\Software\Farmacia\FarmaciaApp` (subclaves `db`, `central`, `nodo`) y el certificado descargado del central en `%APPDATA%\Farmacia\FarmaciaApp\`.
- Si dice «no se pudo encontrar la clave» o «no existe la ruta», es que ya estaba limpio: no es un error.
- Verifica: `reg query "HKCU\Software\Farmacia"` debe responder que no existe.
- Al abrir la app se muestra el asistente. Como la base `farmacia` ya existe, el asistente **respeta lo que hay**: solo crea las tablas que falten, genera de nuevo la contraseña de `farmacia_app`, vuelve a copiar el catálogo del central y **reanuda** la réplica existente (la conexión `sucursal_<id>` del central conserva por dónde iba).
- Limitación: un asistente sobre una base **vieja** no agrega columnas nuevas. Si la base viene de una versión anterior de la app, usa las migraciones (sección 7) o el nivel 2.

### Nivel 2 — Reiniciar también la base local (solo si es imprescindible)

**Antes de empezar, responde:** ¿esta sucursal ya vendió algo que subió al central?

- Comprueba en la base local (`mariadb -u root -p -h 127.0.0.1 -P <puerto>`):
  ```
  SELECT COUNT(*) AS tickets FROM farmacia.Tickets;
  ```
- **Si es 0** (nada vendido): no hay riesgo de colisión; sigue con los pasos 1–4.
- **Si es mayor que 0** (ya hay ventas): la base nueva volvería a numerar tickets, ventas, pagos, etc. desde 1 con el mismo desfase que usó la anterior, y esos números **ya existen en el central**; la réplica de subida se detendría con «Duplicate entry». Antes hay que **limpiar en el central los datos de esa sucursal** (Anexo A) o decidir que esa sucursal no se reinicia.

Pasos:

1. **Espera a que todo haya subido:** en el central, `Seconds_Behind_Master: 0` y sin error para `sucursal_<id>`.
2. **Borra con el registro binario apagado, en una sola sesión.** Abre el cliente y pega **estas tres líneas juntas**:
   ```
   SELECT @@server_id;                  -- DEBE ser 12..20. Si ves 1, ESTÁS EN EL CENTRAL: SALTE.
   SET SESSION sql_log_bin = 0;
   DROP DATABASE farmacia;
   ```
   o, en un solo comando (la misma sesión garantizada):
   ```
   "C:\Program Files\MariaDB 11.4\bin\mariadb.exe" -u root -p -h 127.0.0.1 -P 3306 -e "SELECT @@server_id; SET SESSION sql_log_bin=0; DROP DATABASE farmacia;"
   ```
   **Nunca** uses `-h` con el nombre del central ni con una IP `100.x` al borrar.
3. Reinicia la configuración de la app (Nivel 1).
4. Abre la app y repite el asistente (4.6). Como la base ya no existe, se crea desde cero.
5. **Comprueba que el central sigue entero** (con un admin en la app: *Productos*, *Usuarios*, *Sucursales* con datos; o `sudo mariadb -e "SHOW TABLES FROM farmacia;"`).

### Nivel 3 — Reinstalar MariaDB en el equipo (casi nunca)

Se pierde el binlog local, y el central conserva una posición que ya no existe (error 1236, «Could not find first log file»). Antes de reiniciar, el responsable debe, en el central:

```
STOP SLAVE 'sucursal_<id>';
RESET SLAVE 'sucursal_<id>' ALL;
```

(y limpiar los datos de esa sucursal si la nueva base vuelve a numerar desde 1, Anexo A). El asistente creará la conexión de nuevo, esta vez desde la posición actual del binlog.

### Lo que **nunca** debes hacer para «reiniciar»

- `DROP DATABASE farmacia` sin `SET SESSION sql_log_bin=0` en la misma sesión (borra la base **del central**).
- Borrar archivos de `data\` con el servicio en marcha.
- Registrar el mismo equipo con otra sucursal «para probar»: la sucursal anterior queda con una réplica colgando.
- Ejecutar scripts de limpieza conectado al central sin leer antes qué borran.

---

## 6. Diagnóstico y solución de problemas

### 6.1 Errores del asistente

| Mensaje / síntoma | Causa probable | Qué hacer |
|---|---|---|
| «Can't connect to MySQL server on '127.0.0.1'» | MariaDB local detenido o puerto equivocado | `services.msc` → iniciar; revisa el puerto del asistente |
| «Access denied for user 'root'» | Contraseña de root equivocada | Revisa `<ROOT_LOCAL>`; si la perdiste, restablécela con el procedimiento de MariaDB (`--skip-grant-tables`) |
| «Falta preparar MariaDB» (con un `my.cnf`) | `my.ini` sin escribir, ranura equivocada, servicio no reiniciado | 4.4; recuerda que el texto menciona rutas de Linux, en Windows es `my.ini` |
| «Este equipo no tiene dirección de Tailscale» | Tailscale desconectado | Conéctalo; `tailscale ip -4` |
| «not trusted» / «SSL connection error» | Escribiste la IP del central | Usa el nombre `ip-172-31-41-253.tail560a17.ts.net` |
| «Unknown MySQL server host» / tarda y falla | MagicDNS apagado o sin red | `tailscale ping` al nombre; activa MagicDNS |
| «Access denied» con `farmacia_registro` | Contraseña de registro equivocada | Revisa `<REGISTRO>`; si se perdió: 6.5 |
| «El central no tiene sucursales» | Ninguna sucursal activa | Un admin debe crearla/activarla |
| La lista no incluye mi sucursal | Está *inactiva* | Un admin la activa en *Sucursales* |
| Se queda en «Configurando la replicación…» | Falló un paso con el central | Anota el texto de error que aparece y mira 6.2 |
| «El MariaDB local no tiene registro binario (log-bin) activo» | `my.ini` sin `log-bin` o sin reiniciar | 4.4 |

### 6.2 Réplica rota (diagnóstico en el equipo)

```
SHOW ALL REPLICAS STATUS\G
```

| Campo | Significa |
|---|---|
| `Slave_IO_Running: No` + `Last_IO_Error: error connecting…` | No llega al central (Tailscale, DNS, central caído) |
| `Slave_SQL_Running: No` + `Last_SQL_Error: Table '…' doesn't exist` | Falta una migración en este nodo (sección 7) |
| `Last_SQL_Error: … foreign key constraint fails` | El central borró algo (producto/empleado) que este nodo todavía usa |
| `Seconds_Behind_Master` creciendo | Réplica lenta o con la red cortada |

Reiniciar los hilos tras arreglar la causa: `STOP SLAVE; START SLAVE;`.

### 6.3 Réplica de subida rota (se ve en el central)

```
sudo mariadb -e "SHOW ALL SLAVES STATUS\G"
```
Busca `Connection_name: sucursal_<id>`:

| Síntoma | Causa | Qué hacer |
|---|---|---|
| `Last_IO_Error: error connecting to master 'repl_central@100.x:3306' … Can't connect` | Firewall de Windows (4.5), equipo apagado o fuera de Tailscale | Abrir el puerto; reconectar. Se reanuda sola |
| `Last_SQL_Error: Duplicate entry … for key 'PRIMARY'` | Se reinició la base de una sucursal que ya había subido datos | Anexo A |
| `Last_IO_Errno: 1236` | El binlog del nodo ya no existe | Nivel 3 |
| Falta la conexión `sucursal_<id>` | El asistente no llegó al paso final | Repite el asistente (Nivel 1) |

### 6.4 Problemas dentro de la app

| Síntoma | Causa | Qué hacer |
|---|---|---|
| «Error de conexión» al abrir | MariaDB local detenido | Iniciar el servicio |
| El asistente reaparece solo | Se borró el registro, la base o falló el acceso a `farmacia_app` | Normal si fue intencional; si no, repite el asistente |
| «Aún no se han recibido usuarios del central» | La réplica de bajada no ha traído `Usuarios` | Comprueba Tailscale y la réplica local (6.2) |
| «No estás asignado a esta sucursal…» | Empleado de otra sucursal | Un admin debe autorizar una asignación temporal |
| Admin: «Sin conexión con el central» | Contraseña `<REMOTO>` incorrecta, Tailscale caído | Reintenta; o entra en *modo local* (solo lectura de lo replicado) |
| Cobro con tarjeta o devolución falla con «Unknown column»/enum | Migración 004 sin aplicar en ese nodo o en el central | Sección 7 |
| Botón que parece activo pero no responde | Estado deshabilitado (permiso o selección) | Revisa si falta seleccionar una fila o si no tienes permiso |

### 6.5 Restablecer la contraseña de `farmacia_registro` (lo hace el responsable, en el central)

Solo si se perdió. Por SSH al central:
```
sudo mariadb -e "ALTER USER 'farmacia_registro'@'100.64.0.0/255.192.0.0' IDENTIFIED BY '<NUEVA>'; FLUSH PRIVILEGES;"
```
o reejecutar `db/distribuido/real/preparar-central.sh` con `PW_REGISTRO=<NUEVA>` (también reaplica los permisos de todas las cuentas). Cambiar esta contraseña **no** afecta a los equipos ya enlazados: solo se usa en el asistente.

## 7. Migraciones y cambios de esquema

Un equipo **nuevo** ya trae el esquema actual dentro de la app: **no necesita migraciones**. Las necesita un equipo con una base *vieja*:

| Migración | Qué agrega | Archivo |
|---|---|---|
| 002 | Precios por sucursal (`Productos_Sucursal`, `precio_sugerido`) | `db/migraciones/002_precios_por_sucursal.sql` |
| 003 | Permisos temporales y bitácora | `003_permisos_y_bitacora.sql` |
| 004 | Pagos y reembolsos con tarjeta | `004_reembolsos_tarjeta.sql` |
| 005 | **Totales calculados:** `Tickets.total` y `saldo_pendiente` pasan a `VIRTUAL` (corrige que el central mostraba 0.00) | `005_totales_calculados.sql` |

Orden **obligatorio** al desplegar un cambio:
1. Aplicar la migración en **todos** los nodos y en el central (en Windows: abrir el cliente, `USE farmacia;` y `SOURCE ruta\migracion.sql;`, con `SET SESSION sql_log_bin=0;` antes, en nodos; en el central, el responsable por SSH: `sudo mariadb < archivo.sql`).
2. Reejecutar `preparar-central.sh` en el central.
3. Solo entonces usar la versión nueva de la app.

> Estado al 2026-11-20: la **004** está aplicada en el central y en el portátil Linux (falta en el equipo Windows). La **005** está preparada y **pendiente** en todos los nodos. Aplícala **primero en el central** (corrige al instante lo ya subido) y luego en cada equipo. Confirma la 004 con `DESCRIBE farmacia.Pagos;` (debe existir `ultimos_digitos`) y la 005 con `SELECT column_name, extra FROM information_schema.columns WHERE table_name='Tickets' AND column_name IN ('total','saldo_pendiente');` (debe decir `VIRTUAL GENERATED`).
>
> **Defecto que corrige la 005:** sin ella, un administrador conectado al central ve `total` y `saldo_pendiente` en 0.00 en tickets, Inicio y Reporte diario. En la sucursal se ve bien. Mientras no esté aplicada, las pruebas P-05, N-08 y N-17 fallan en el central.

## 8. Operaciones frecuentes del responsable

- **Agregar una sucursal:** admin → *Sucursales* → nueva. El central le asigna el id (1, 11, 21…). Máximo 9.
- **Ver qué pasa en el central:** `sudo mariadb`, luego `SHOW ALL SLAVES STATUS\G` y `SHOW TABLES FROM farmacia;`.
- **Respaldo del central:** `ssh … 'sudo mariadb-dump --single-transaction --routines --triggers farmacia' > respaldo.sql`.
- **Renovación del certificado TLS** (caduca 2027-01-31): hoy es manual; planéala antes de esa fecha.
- **Llave SSH del central:** copiarla con permisos 600 antes de usarla y borrar la copia al terminar.

---

# PARTE B — MANUAL DE USUARIO

## 9. Uso diario

### 9.1 Iniciar sesión
1. Escribe usuario y contraseña. La contraseña distingue mayúsculas.
2. **Empleado:** entra directo a la base de su sucursal.
3. **Administrador:** aparece una segunda ventana pidiendo la contraseña de administración remota. Con ella trabaja sobre el central y ve **todas las sucursales**. Si no hay conexión puede entrar en **modo local** (solo consulta lo replicado, sin editar).
4. La barra superior muestra tu **insignia**: rol, alcance («Sucursal …»/«Todas»), «Encargado hasta …» si tienes un permiso temporal, y el estado de la sincronización (verde = al día).
5. La app abre a **pantalla completa**; para cerrarla usa `Alt+F4` o la opción de salir del menú.

### 9.2 El menú
Agrupado en **Ventas** (Inicio, Punto de Venta, Clientes, Tickets, Reporte diario), **Inventario** (Productos, Categorías, Lotes/Inventario, Proveedores) y **Administración** (Empleados, Sucursales, Usuarios, Asignaciones, Permisos, Bitácora). Cada usuario ve solo lo que le corresponde (tabla de la sección 9.9).

### 9.3 Vender (Punto de Venta, solo sucursales)
1. Busca el producto por **código de barras o nombre** y agrégalo. El sistema toma el lote con existencias de la sucursal; si pides más de lo que hay avisa «Stock insuficiente».
2. Elige el **cliente** (o «Venta de mostrador (sin registrar)») y el **empleado** que atiende.
3. **Cobrar:** elige el método:
   - **Efectivo** o **transferencia:** captura el monto.
   - **Mixto** (solo ventas de contado): combina tarjeta, transferencia y efectivo. Captura cuánto va con tarjeta (con su autorización) y con transferencia; el **efectivo es lo que falta**. Escribe el efectivo recibido y la ventana calcula el **cambio**. Debe haber al menos dos métodos y la suma tiene que ser exactamente el total.
   - **Tarjeta:** la app **no cobra**; cobras en la terminal y registras aquí la **autorización** del comprobante (4 a 20 letras/números, normalmente 6) y, si quieres, los **últimos 4 dígitos**.
   - **Diferido (a crédito):** exige un cliente registrado y una fecha límite; después se registran **abonos**.
4. Al terminar aparece «Venta registrada: Ticket #…» con el botón **«Ver / imprimir ticket»**.
5. El precio es el de **tu sucursal**; si no se fijó uno, se cobra el precio sugerido del catálogo. Cambiar un precio no altera tickets viejos.

### 9.3b El ticket impreso
- **Verlo:** desde «Ver / imprimir ticket» al cobrar o desde el detalle de cualquier ticket (botón del mismo nombre). Se muestra tal como saldría en el papel de 80 mm.
- **Guardar PDF…:** genera un PDF del ancho del rollo. Sirve como comprobante o para probar sin impresora.
- **Imprimir…:** usa la impresora configurada o te deja elegir una. Sin impresoras instaladas, ofrece el PDF.
- **Configurar…:** impresora, ancho del papel (80 mm; 58 mm = 32 columnas), columnas por línea (42 por defecto), nombre comercial, razón social, RFC, domicilio, teléfono y el texto del pie. **Se guarda por equipo.** Si faltan la razón social o el RFC, la vista lo avisa.
- Contenido: establecimiento, folio, fecha, quién atendió, cliente, productos con cantidad, precio e importe, **IVA desglosado** (los precios ya lo incluyen), forma de pago (con autorización y últimos 4 de la tarjeta), efectivo recibido y cambio, saldo y fecha límite si fue a crédito, y las devoluciones del ticket.
- El efectivo recibido y el cambio **solo salen al cobrar**; una reimpresión no los muestra.
- El ticket **no es una factura**. El desglose de IVA usa la categoría del producto (medicamento a 0 %, el resto a 16 %): confírmalo con tu contador.

### 9.4 Tickets y devoluciones
- **Tickets:** lista, filtros y *Detalle* del ticket (productos, pagos, devoluciones).
- **Devolución:** desde el detalle, elige las líneas y cantidades, el **estado del producto** (sellado, abierto, dañado, caducado), el **empleado que atiende** y el **tipo de reembolso**. Las devoluciones están **siempre permitidas**, también a quien no tiene permiso temporal.
- **Ticket pagado con tarjeta:** el reembolso solo puede ser **a la tarjeta** (efectivo/transferencia solo por la parte que se pagó así). Elige la **modalidad**: *cancelación* (el mismo día, antes del cierre de lote de la terminal) o *devolución* (después). Queda **«solicitada / pendiente en terminal»**; haces el reembolso en la terminal y, con el folio que imprime, pulsas **«Registrar folio de reembolso»** (4 a 20 letras/números) y pasa a **completada**.
- El inventario se reintegra al registrar la devolución (aunque el reembolso siga pendiente).

### 9.5 Clientes
Alta, edición, baja lógica y consulta de saldo. El cliente pertenece a **una sucursal**.

### 9.6 Inventario, productos y lotes
- **Productos / Categorías / Proveedores:** el catálogo lo administra el **admin** en el central y baja a las sucursales. El empleado solo consulta.
- **Precio de tu sucursal:** un empleado con permiso temporal puede fijar *Precio en mi sucursal*, si lo maneja y su stock mínimo.
- **Lotes:** alta/edición de lotes (número, caducidad, cantidad, **costo**). Un empleado necesita permiso temporal para ello.

### 9.7 Permiso temporal «encargado» (lo concede un admin)
Un empleado **sin permiso** solo hace lo esencial: vender, cobrar, consultar, registrar abonos y **devoluciones**. Un admin puede concederle el rol «encargado» por **1, 2 o 3 días** (24/48/72 h exactas): pantalla **Permisos** → elegir empleado y duración. Vence solo (también sin conexión) y puede **revocarse** antes. La insignia muestra «Encargado hasta …».

### 9.8 Reporte diario y bitácora
- **Reporte diario:** elige la fecha (hasta hoy) y, si eres admin, una sucursal o todas. Incluye resumen, ventas por sucursal, cobros por método, **corte de efectivo** (cobrado − devuelto), devoluciones, productos más vendidos, movimientos de inventario, cambios de la bitácora y alertas. **Exportar a PDF** guarda el mismo reporte en A4 (la carpeta sugerida es *Documentos*).
- **Bitácora (admin):** cada cambio hecho con permiso temporal queda con usuario, fecha, valor anterior → nuevo.

### 9.9 Qué ve cada rol

| Pantalla | Empleado | Admin |
|---|---|---|
| Inicio, Punto de Venta, Clientes, Tickets, Inventario | Solo su sucursal | Consulta de todas; **no vende** |
| Productos, Categorías | Lectura (+ precio propio con permiso) | Alta/edición/baja |
| Proveedores, Empleados, Sucursales, Usuarios, Asignaciones, Permisos, Bitácora | Ocultas | Administración |
| Reporte diario | Su sucursal | Una o todas |

---

# PARTE C — PLAN DE PRUEBAS

## 10. Cómo usar este plan

- **Un equipo, una hoja.** Copia las tablas a una hoja de cálculo o imprímelas y marca ✔ / ✘ / N/A en cada fila. Anota fecha, equipo (nombre de Windows), sucursal y versión de la app (commit o fecha del artefacto).
- Cada fila: **ID · acción · resultado esperado**. Si el resultado real difiere, es un **defecto**: usa la plantilla de la sección 20.
- **Prueba negativa** = intentar romperlo a propósito. Un mensaje claro y sin cierre inesperado es un **éxito**; un cierre de la app, una pantalla congelada o un dato incorrecto guardado es un **defecto**.
- Antes de cada bloque que modifica datos, anota el estado inicial (stock, saldo).
- Donde el resultado esperado dice «mensaje claro», lo que se evalúa es que **no se cierre la app**, que el texto explique qué hacer y que **no quede nada a medias** en los datos.

### Datos de prueba sugeridos
Productos del catálogo de ejemplo (códigos de `db/seed.sql` en equipo único; en central, los de `datos-ejemplo-central.sql`): ten al menos un producto con **stock abundante**, uno con **stock 1**, uno **sin stock**, uno con **lote caducado**, uno que **requiere receta**, y un cliente con saldo.

## 11. Prueba de humo (15 min, tras cada instalación)

| ID | Acción | Esperado |
|---|---|---|
| H-01 | Abrir la app | Pantalla de inicio de sesión (o asistente si es el primer arranque) |
| H-02 | Iniciar sesión con el empleado de la sucursal | Entra; insignia con rol y réplica en verde |
| H-03 | Vender 1 producto en efectivo | «Ticket #… registrado»; baja 1 unidad de stock |
| H-04 | Abrir el ticket y revisar el detalle | Producto, precio y pago correctos |
| H-05 | Devolver ese producto (efectivo, sellado) | Devolución registrada; stock vuelve a subir |
| H-06 | Esperar ~1 min y mirar el central (admin, otra sesión): *Tickets* | El ticket y la devolución aparecen en el central |
| H-07 | Con admin, cambiar el precio sugerido de un producto | En ≤ 1 min el cambio aparece en la sucursal |
| H-08 | Reporte diario de hoy → Exportar a PDF | PDF abre; las cifras coinciden con lo vendido |
| H-09 | Cerrar sesión/cerrar app y volver a abrirla | Entra sin pedir el asistente |
| H-10 | `SHOW ALL REPLICAS STATUS\G` local | Ambos hilos `Yes`, sin errores |

## 12. Instalación y asistente

| ID | Prueba | Esperado |
|---|---|---|
| I-01 | Abrir la app sin MariaDB corriendo (servicio detenido) | Mensaje «Error de conexión … Verifica que MariaDB esté corriendo» y salida limpia |
| I-02 | Asistente con contraseña de root incorrecta | «Access denied»; el botón Configurar vuelve a habilitarse; no se crea nada |
| I-03 | Asistente con puerto equivocado | Mensaje de no conexión; sin cierre |
| I-04 | Con la casilla de central marcada y **sin** `my.ini` preparado | Aparece «Falta preparar MariaDB» con instrucciones; nada se crea |
| I-05 | `my.ini` con ranura **equivocada** (p. ej. offset 5 para una sucursal de ranura 3) | Vuelve a salir «Falta preparar MariaDB» (compara server-id y offset) |
| I-06 | `my.ini` con `binlog_format=MIXED` | Mismo mensaje: debe ser ROW |
| I-07 | Dirección del central con **IP** | Falla con «not trusted»/error SSL; con el nombre funciona (documenta el texto) |
| I-08 | Central con nombre mal escrito | Error de nombre desconocido, sin cierre |
| I-09 | Contraseña de registro incorrecta | «Access denied»; sin cambios locales |
| I-10 | Cuenta de registro vacía / contraseña vacía | «Indica la cuenta y la contraseña de registro del central» |
| I-11 | Dirección del central vacía | «Indica la dirección del central» |
| I-12 | **Consultar** sin conexión de red (Tailscale desconectado) | Error con detalle; la ventana sigue respondiendo (tiempo de espera ~8 s) |
| I-13 | Tailscale desconectado y pulsar Configurar | «Este equipo no tiene dirección de Tailscale…» |
| I-14 | Pulsar **Configurar** dos veces rápido | Solo una ejecución; no se duplican cuentas ni errores |
| I-15 | Cancelar el asistente a medias | La app termina; al reabrir vuelve el asistente; el estado local es coherente |
| I-16 | Cerrar el asistente con la X durante «Configurando la replicación…» | Sin cierre brusco; al reabrir, repetir el asistente funciona (idempotente) |
| I-17 | Instalación completa, sucursal libre | Termina y abre el login; réplicas activas (H-10) |
| I-18 | Dos instalaciones en dos equipos con **sucursales distintas** | Ambas operan; sus ventas no se mezclan ni chocan en el central |
| I-19 | Repetir el asistente en el mismo equipo (Nivel 1) con la base existente | No pierde datos locales; réplicas siguen; catálogo actualizado |
| I-20 | Sucursal inactiva no aparece en la lista | Correcto |
| I-21 | Equipo único: asistente sin central | Base creada; «Crear administrador inicial» disponible |
| I-22 | Equipo único: crear admin con contraseña vacía / muy corta | Rechaza con mensaje claro |
| I-23 | Equipo único: abrir de nuevo tras crear el admin | El botón «Crear administrador inicial» **ya no aparece** |
| I-24 | Mover la carpeta de la app a otra ruta y abrirla | Funciona (la configuración vive en el registro) |
| I-25 | Abrir la app dos veces a la vez | No corrompe datos (anota lo que ocurre) |
| I-26 | Ejecutar sin las DLL (copiar solo el `.exe`) | Falla al arrancar por DLL faltante: confirma que el paquete completo es necesario |
| I-27 | Cambiar el idioma/región de Windows a inglés y abrir | La app sigue en español, sin caracteres rotos |

## 13. Reinicio (probar lo que se documenta en la sección 5)

| ID | Prueba | Esperado |
|---|---|---|
| R-01 | Nivel 1: borrar clave de registro y carpeta AppData con la app cerrada | El asistente reaparece al abrir |
| R-02 | Nivel 1 con la base intacta | Tras el asistente, los datos locales siguen; réplicas `Yes` |
| R-03 | Tras R-02, vender y comprobar en el central | Llega al central |
| R-04 | Ejecutar los comandos de borrado con la clave ya inexistente | Mensajes de «no existe», sin daño |
| R-05 | **Nivel 2 sin ventas** (`Tickets` = 0), con `sql_log_bin=0` | La base local se borra; **la del central sigue intacta** (verificar tablas y conteos) |
| R-06 | Tras R-05, repetir el asistente | Se reconstruye; catálogo igual al central |
| R-07 | Tras R-06, vender | Sube al central sin errores de duplicado |
| R-08 | **(Solo con el responsable, en entorno de prueba)** Nivel 2 con ventas previas y **sin** limpiar el central | Esperado y documentado: la réplica de subida se detiene con «Duplicate entry». Confirma el aviso del manual y recupera con el Anexo A |
| R-09 | Tras R-08, aplicar el Anexo A y reanudar | La réplica continúa sin errores |
| R-10 | `SELECT @@server_id` antes del `DROP` | Número 12..20; nunca 1 |

> **Prohibido:** probar `DROP DATABASE` **sin** `sql_log_bin=0` en un nodo enlazado. Ya se sabe qué ocurre y borra el central. Si quieres evidencia, léela en `ESTADO_PROYECTO.md` (incidente 1).

## 14. Inicio de sesión y permisos

| ID | Prueba | Esperado |
|---|---|---|
| L-01 | Usuario y contraseña vacíos | «Escribe usuario y contraseña.» |
| L-02 | Solo usuario, sin contraseña | Mismo mensaje |
| L-03 | Contraseña incorrecta | Mensaje de credenciales inválidas; sin pistas de si existe el usuario |
| L-04 | Usuario inexistente | Mismo mensaje genérico |
| L-05 | Mayúsculas/minúsculas en el usuario (`ANA`) | Documenta si entra o no; debe ser consistente |
| L-06 | Espacios antes/después del usuario | Se recortan; entra |
| L-07 | Contraseña con espacios o caracteres especiales (`ñ`, `'`, `"`, `;`, `--`, `%`) | Sin errores SQL; entra solo si es la correcta |
| L-08 | Inyección SQL: usuario `' OR '1'='1` | No entra; sin error de SQL visible |
| L-09 | Usuario inactivo (`pedro`) | No entra; mensaje claro |
| L-10 | Empleado de **otra** sucursal sin asignación | «No estás asignado a esta sucursal…» |
| L-11 | Empleado de otra sucursal **con** asignación temporal vigente | Entra; la sucursal de la sesión es la del equipo |
| L-12 | Asignación vencida (fecha_fin ayer) | No entra |
| L-13 | Asignación en estado distinto de «aprobada» | No entra |
| L-14 | Admin con `<REMOTO>` correcta | Entra sobre el central; ve todas las sucursales |
| L-15 | Admin con `<REMOTO>` incorrecta | «No se conectó con el central» + pregunta por modo local |
| L-16 | Admin: cancelar la ventana de contraseña remota | Pregunta por modo local; «No» vuelve al login |
| L-17 | Admin en modo local | Puede consultar; **no** puede editar catálogo/personal/cuentas |
| L-18 | Admin sin red al central (Tailscale apagado) | Falla en ≤ ~5 s y ofrece modo local; sin congelarse |
| L-19 | Contraseña remota con espacios al final | Documenta el comportamiento |
| L-20 | Sesión de empleado: intentar abrir pantallas de administración | No están visibles |
| L-21 | Admin: el botón de «Punto de Venta» | No hay (el central no vende) |
| L-22 | Cerrar la app a media sesión y volver a iniciar | Sin sesión residual; pide login |
| L-23 | Cambiar la hora del equipo un día atrás/adelante | Permisos y asignaciones respetan la fecha **UTC** del servidor; documenta |

## 15. Punto de venta, tickets, devoluciones y pagos

| ID | Prueba | Esperado |
|---|---|---|
| V-01 | Buscar producto por código de barras existente | Se agrega al carrito |
| V-02 | Buscar por nombre parcial | Lista o agrega el correcto |
| V-03 | Código inexistente | «Producto no encontrado» |
| V-04 | Producto inactivo | No se encuentra |
| V-05 | Producto sin lotes disponibles | «Sin existencias» |
| V-06 | Cantidad 0 | Rechazo o ignorado, sin añadir |
| V-07 | Cantidad negativa | No permitido |
| V-08 | Cantidad decimal o texto (`1.5`, `abc`) | Rechazado/normalizado, sin cierre |
| V-09 | Cantidad enorme (`999999999999`) | Rechazada con mensaje; sin desbordamiento |
| V-10 | Pedir exactamente el stock disponible | Se vende; el stock queda en 0 |
| V-11 | Pedir stock + 1 | «Stock insuficiente» y no vende |
| V-12 | Producto repartido en varios lotes, pedir más que un lote | Descuenta de más de un lote correctamente; suma coherente |
| V-13 | Lote caducado | No debe venderse (o advertir); anota el comportamiento |
| V-14 | Producto que requiere receta | Comportamiento esperado según el diseño; anótalo |
| V-15 | Venta de mostrador, efectivo exacto | Ticket `pagado`, sin cambio |
| V-16 | Efectivo mayor que el total | Muestra cambio; el pago registrado es el total |
| V-17 | Efectivo menor que el total en venta de contado | No permite cerrar |
| V-18 | Descuento (si la pantalla lo ofrece): 0, normal, igual al subtotal, mayor | Mayor al subtotal rechazado; total nunca negativo |
| V-19 | Cobrar con carrito vacío | «El carrito está vacío.» |
| V-20 | Doble clic en Cobrar | Un solo ticket |
| V-21 | Cerrar la app a mitad de la venta (antes de cobrar) | Nada se guarda ni descuenta |
| V-22 | Pago **transferencia** con referencia | Se registra con referencia |
| V-23 | Pago **tarjeta** con autorización de 6 caracteres | Se registra; ticket pagado |
| V-24 | Tarjeta con autorización vacía | Rechazada |
| V-25 | Tarjeta con autorización de 3 o de 21 caracteres | Rechazada (rango 4–20) |
| V-26 | Tarjeta con autorización con espacios o símbolos | Rechazada (solo letras y números) |
| V-27 | Últimos 4 dígitos: vacío / 4 dígitos / 3 / letras | Vacío y 4 dígitos OK; los demás, rechazados |
| V-28 | Venta **diferida** con cliente de mostrador | «El pago diferido requiere un cliente registrado» |
| V-29 | Diferida con cliente y fecha límite pasada | Rechazada o marcada vencida; anota |
| V-30 | Abono parcial a un ticket diferido | Estado `parcial`; saldo baja |
| V-31 | Abono que liquida el saldo | `pagado`; saldo 0 |
| V-32 | Abono mayor al saldo | Rechazado |
| V-33 | Abono con tarjeta (autorización) | Se registra con autorización |
| V-34 | Ver detalle de un ticket sin seleccionar fila | «Selecciona un ticket de la lista.» |
| V-35 | Devolución parcial (1 de 3 unidades) | Registra; queda disponible para devolver solo 2 |
| V-36 | Devolver más de lo vendido | Rechazada |
| V-37 | Devolver de nuevo una línea ya devuelta por completo | «No queda cantidad disponible para devolver de esta línea.» |
| V-38 | Devolución sin elegir empleado | «Selecciona el empleado que atiende la devolución.» |
| V-39 | Producto devuelto *sellado* | Stock se reintegra |
| V-40 | Producto devuelto *dañado/caducado* | Según diseño (no reintegrar); verifica stock |
| V-41 | Devolver un ticket pagado con **tarjeta** como efectivo | Rechazado (solo a la tarjeta) |
| V-42 | Devolución a tarjeta, modalidad *cancelación* | Queda `solicitada` («pendiente en terminal») |
| V-43 | Registrar folio de reembolso válido (6 caracteres) | Pasa a `completada` |
| V-44 | Folio inválido (3 o 21 caracteres, símbolos) | «El folio debe tener de 4 a 20 letras o números, sin espacios» |
| V-45 | Dejar el folio vacío | Permitido por diseño; la devolución sigue pendiente |
| V-46 | Ticket pagado mitad tarjeta/mitad efectivo | Efectivo solo hasta lo pagado en efectivo; el resto a tarjeta |
| V-47 | Reembolso «saldo a favor» y «cambio» | Permitidos; no mueven dinero |
| V-48 | Reporte diario tras V-42 | Devolución de tarjeta aparece como pendiente (cualquier fecha) |
| V-49 | Dos ventas del mismo producto casi simultáneas en dos equipos de **distintas** sucursales | Cada una descuenta su stock; sin interferencia |
| V-50 | Vender con la red de Tailscale **desconectada** | Funciona (la base local no necesita red); sube al reconectar |

## 15b. Pago mixto y ticket impreso

| ID | Prueba | Esperado |
|---|---|---|
| PM-01 | Cobrar $98 con tarjeta $50 + transferencia $20 (resto efectivo, recibido $40) | Efectivo a cobrar $28, cambio $12; venta registrada |
| PM-02 | En la base, `Pagos` del ticket | 3 filas: efectivo $28 (sin el cambio), tarjeta $50 con autorización, transferencia $20; suman $98 |
| PM-03 | Mixto con un solo método (todo con tarjeta o todo en efectivo) | «Un pago mixto combina al menos dos métodos»; no se cobra |
| PM-04 | Tarjeta + transferencia mayores al total | «suman más que el total» |
| PM-05 | Tarjeta sin autorización, o de 3 o 21 caracteres | Rechazada |
| PM-06 | Últimos dígitos con 3 números o letras | Rechazados (vacío está permitido) |
| PM-07 | Efectivo recibido menor al efectivo a cobrar | «no alcanza: faltan $…» |
| PM-08 | Tarjeta + transferencia sin efectivo | Se acepta; no pide efectivo recibido |
| PM-09 | Elegir «mixto» con modalidad «diferido» | «El pago mixto aplica solo a ventas de contado»; no se vende |
| PM-10 | Cancelar la ventana de pago mixto | No se vende nada ni baja el stock |
| PM-11 | Efectivo y tarjeta simples tras el cambio | Siguen funcionando igual que antes |
| PM-12 | Devolver en efectivo más de lo no pagado con tarjeta en un ticket mixto | Rechazado («Pagó con tarjeta… solo se puede devolver $…») |
| PM-13 | Devolver a tarjeta más de lo pagado con tarjeta | Rechazado («Solo quedan $… por reembolsar a la tarjeta») |
| PM-14 | Devolución a tarjeta de un ticket mixto | Queda «pendiente en terminal» hasta capturar el folio |
| PM-15 | Reporte diario con ventas mixtas | «Cobros por método» reparte el efectivo, la tarjeta y la transferencia; el corte de efectivo cuenta solo el efectivo |
| TK-01 | «Ver / imprimir ticket» tras cobrar | Se ve el ticket con folio, fecha, productos, IVA, total y pago |
| TK-02 | Las columnas de cantidad, precio e importe | Alineadas; el importe a la derecha; letra de ancho fijo |
| TK-03 | Producto con nombre muy largo | Se parte por palabras en varias líneas; ninguna línea se sale del ancho |
| TK-04 | Ticket con 1 producto y con 40 productos | Ambos completos; sin cortes raros |
| TK-05 | Ticket de venta a crédito con abono | Muestra los abonos, «SALDO PENDIENTE» y fecha límite |
| TK-06 | Ticket pagado con tarjeta | «Tarjeta ****NNNN» y la autorización |
| TK-07 | Ticket con devoluciones | Sección «DEVOLUCIONES REGISTRADAS»; la de tarjeta sin folio dice «pendiente en terminal» |
| TK-08 | Reimprimir desde el detalle de un ticket | Igual, sin «Efectivo recibido» ni «Cambio» |
| TK-09 | Admin conectado al central abre el ticket de una sucursal | Se ve completo, con los datos de esa sucursal |
| TK-10 | Configurar: RFC inválido (`XX123`) | «El RFC debe tener 12 caracteres…»; no guarda |
| TK-11 | Configurar razón social y RFC | Salen en el encabezado y desaparece el aviso «Faltan datos fiscales» |
| TK-12 | Cambiar columnas a 48 y a 32 | El ticket se vuelve a armar sin líneas partidas |
| TK-13 | Guardar PDF y abrirlo | Papel de 80 mm de ancho; letra legible; acentos y `ñ` correctos |
| TK-14 | Imprimir sin impresoras instaladas | Aviso claro y la opción del PDF; la venta no se pierde |
| TK-15 | Imprimir en la térmica real de 80 mm | El texto cabe en el ancho, sin saltos de línea inesperados (**calibrar columnas si no**) |
| TK-16 | Impresora apagada o sin papel | Mensaje claro; el ticket sigue disponible para reimprimir |
| TK-17 | Reinicio de nivel 1 del equipo | La configuración del ticket se borra con el resto (volver a configurarla) |
| TK-18 | Desglose de IVA: producto de categoría «medicamento» y otro de higiene en el mismo ticket | IVA 0 % sobre el medicamento e IVA 16 % sobre el otro; subtotal + IVA = total |

## 16. Catálogo, inventario, clientes, administración

| ID | Prueba | Esperado |
|---|---|---|
| C-01 | Admin: crear producto con código de barras nuevo | Se crea; baja a las sucursales en ≤ 1 min |
| C-02 | Código de barras **repetido** | Rechazado (único) |
| C-03 | Campos obligatorios vacíos | Mensajes de validación; no guarda |
| C-04 | Texto muy largo en nombre/descr. | Rechaza o recorta sin error de SQL visible |
| C-05 | Caracteres especiales y acentos (`Ñandú`, `’`, `😀`) | Se guardan y se ven bien en el central y en la sucursal |
| C-06 | Precio negativo / 0 / con 3 decimales / `abc` | Negativo y texto rechazados |
| C-07 | Editar precio sugerido | Tickets viejos conservan su precio |
| C-08 | Empleado **sin** permiso intenta fijar precio propio | Botón no disponible o rechazado |
| C-09 | Empleado **con** permiso temporal fija «Precio en mi sucursal» | Se guarda; la venta usa ese precio; queda en bitácora |
| C-10 | Permiso vence a mitad de la sesión | El siguiente guardado se rechaza (se vuelve a comprobar al guardar) |
| C-11 | Crear lote con caducidad pasada | Advertencia/rechazo o estado `caducado`; anota |
| C-12 | Lote con cantidad 0 o negativa | Rechazado |
| C-13 | Número de lote repetido (mismo producto/sucursal) | Rechazado o aceptado según el diseño; anota |
| C-14 | Costo unitario mayor al precio | Permitido o advertido; anota |
| C-15 | Admin: alta de empleado y de usuario ligado | El usuario baja a las sucursales; puede iniciar sesión |
| C-16 | Usuario con nombre repetido | Rechazado |
| C-17 | Contraseña de usuario: vacía/corta | Rechazada |
| C-18 | Desactivar un usuario | Deja de poder entrar (en la sucursal tras replicar) |
| C-19 | Admin: borrar un producto con ventas | Rechazado por clave foránea o inactivado; **no** debe romper réplicas (anota qué hace) |
| C-20 | Admin: borrar un empleado/usuario usado en otra sucursal | Comportamiento documentado en Limitaciones; probar solo en entorno de prueba |
| C-21 | Admin: crear sucursal | Recibe id 1, 11, 21…; aparece en el asistente (si está activa) |
| C-22 | Admin: desactivar una sucursal | Deja de listarse en el asistente |
| C-23 | Clientes: alta con teléfono/correo inválidos | Validación o aceptación documentada |
| C-24 | Cliente con el mismo nombre en otra sucursal | Permitido; independiente |
| C-25 | Admin: concesión de permiso por 1, 2 y 3 días | La insignia del empleado dice «Encargado hasta …» con la hora correcta |
| C-26 | Permiso por 4 días (si pudieras forzarlo) | Rechazado (la base lo impide con un `CHECK` de 72 h) |
| C-27 | Revocar un permiso | El empleado pierde las funciones de encargado en el siguiente guardado |
| C-28 | Bitácora tras C-09 | Muestra usuario, fecha, antes → después y «con permiso» |
| C-29 | Asignación temporal: crear, aprobar, vencer | Coherente con L-11/L-12 |
| C-30 | Equipos en husos horarios distintos | Vigencia de permisos coincide (UTC) |

## 17. Replicación, red y robustez

| ID | Prueba | Esperado |
|---|---|---|
| N-01 | Venta en sucursal → verla en el central | Aparece en ≤ ~1 min |
| N-02 | Cambio de catálogo en el central → verlo en la sucursal | Aparece en ≤ ~1 min |
| N-03 | Apagar Tailscale en la sucursal 5 min; vender 3 tickets; reconectar | Los 3 suben, sin duplicados ni huecos |
| N-04 | Apagar el equipo de la sucursal con ventas pendientes de subir; encender | Se ponen al día solos |
| N-05 | Reiniciar el servicio MariaDB local | La app reconecta o pide reiniciarla; réplicas reanudan solas |
| N-06 | Reiniciar el central (lo hace el responsable) | Sucursales reanudan; verificar `replicate_do_db` de `sucursal_<id>` (se pierde al reiniciar, pero el filtro del nodo basta) |
| N-07 | Dos sucursales vendiendo a la vez | Sin choques de ID en el central |
| N-08 | Misma venta mirada desde la sucursal y el central | Cifras idénticas (subtotal, total, pagos) |
| N-09 | Ejecutar `SELECT` pesado/consultas largas durante réplica | Sin bloqueos visibles |
| N-10 | Regla de un solo escritor: con el usuario `farmacia_app` intentar `INSERT` en `Productos` desde un cliente SQL | `INSERT command denied` |
| N-11 | Cuenta `farmacia_remoto`: intentar escribir en `Tickets` en el central | Denegado |
| N-12 | Desde otro equipo **sin** Tailscale intentar conectar al 3306 del central | No conecta (tiempo agotado) |
| N-13 | Dejar la app abierta 8 horas | Sigue respondiendo; la insignia sigue actualizando |
| N-14 | Reloj del equipo mal (fecha +1 día) | Documenta el efecto en reportes y permisos |
| N-15 | Disco casi lleno / crecimiento del binlog | Anota el tamaño del binlog tras 1 día de uso |
| N-16 | Desconectar el cable/Wi-Fi durante «Cobrar» | La venta se guarda **completa o nada** (transacción) |
| N-17 | Con la migración 005: vender a crédito con abono parcial y mirarlo con un admin **conectado al central** (Tickets, detalle, Inicio y Reporte diario) | `total` y `saldo_pendiente` correctos (no 0.00) e iguales a los de la sucursal |

## 18. Reporte diario, PDF y pantalla

| ID | Prueba | Esperado |
|---|---|---|
| P-01 | Reporte de un día **sin** movimientos | Sin errores; secciones vacías con aviso |
| P-02 | Reporte de hoy después de V-03…V-50 | Cifras coinciden con tickets y pagos |
| P-03 | Corte de efectivo = cobrado − devuelto | Verificar con una calculadora |
| P-04 | Fecha futura | No se permite (hasta hoy) |
| P-05 | Admin: una sucursal vs todas | Las cifras de «todas» = suma de las individuales |
| P-06 | Admin sobre central con una sucursal desconectada | El reporte avisa que puede estar incompleto |
| P-07 | Exportar a PDF y abrirlo | A4, texto legible, acentos y símbolos correctos |
| P-08 | Exportar a una ruta sin permiso / nombre inválido | Error claro; sin cierre |
| P-09 | Exportar dos veces el mismo nombre | Pide confirmar sobrescritura |
| P-10 | Cambiar el tamaño/escala de pantalla de Windows (100/125/150 %) | Interfaz legible, botones no cortados |
| P-11 | Pantalla pequeña (1366×768) | Todo accesible |
| P-12 | Modo oscuro de Windows activado | La app mantiene su apariencia (no hay modo oscuro propio) |
| P-13 | Botones deshabilitados (Eliminar sin selección) | Se ven claramente atenuados |
| P-14 | Insignia de sesión con réplica caída | Cambia de color y texto |

## 19. Pruebas de seguridad básicas

| ID | Prueba | Esperado |
|---|---|---|
| S-01 | Buscar contraseñas en el registro: `reg query "HKCU\Software\Farmacia" /s` | Solo `farmacia_app` (aleatoria); **nunca** root, registro ni remota |
| S-02 | Buscar `<REGISTRO>` o `<REMOTO>` en archivos de la carpeta de la app y de AppData | No aparecen |
| S-03 | Entrar como empleado y tratar de hacer una acción de admin por la interfaz | No hay manera de hacerlo |
| S-04 | Campos de texto con `'`, `"`, `\`, `;` en todos los formularios | Sin errores SQL |
| S-05 | Pegar 10 000 caracteres en un campo | Rechazado o recortado sin congelar |
| S-06 | Tras cerrar sesión no se puede volver con «atrás» | No hay navegación posible |
| S-07 | `SHOW GRANTS FOR 'farmacia_app'@'localhost'` | Solo lo previsto (lectura de catálogo, escritura de tablas de sucursal) |

## 20. Plantilla de reporte de defecto

```
ID de la prueba:
Equipo / sucursal / versión de la app (commit o fecha del artefacto):
Fecha y hora (local y UTC):
Pasos exactos (1, 2, 3 …):
Resultado esperado:
Resultado obtenido (texto literal del mensaje o captura):
¿Se cerró la app?  ¿Quedó algo a medias en los datos?
¿Se reproduce?  (siempre / a veces / una vez)
Estado de réplica (SHOW ALL REPLICAS STATUS en el nodo y SHOW ALL SLAVES STATUS en el central):
Gravedad:  Bloqueante · Alta · Media · Baja
```

## 21. Criterios de aceptación para dar por buena la entrega

- [ ] Prueba de humo (H-01…H-10) completa en **cada** equipo instalado.
- [ ] Todas las filas I-, L-, V-, C-, PM- y TK- marcadas; los defectos *Bloqueante/Alta* corregidos o documentados.
- [ ] N-01…N-04 y N-16 superadas (la réplica no pierde ni duplica datos).
- [ ] R-01…R-07 superadas **sin** tocar el central.
- [ ] Ninguna contraseña de `root`, registro o remota en el equipo (S-01/S-02).
- [ ] Migraciones 002, 003, 004 y **005** aplicadas en el central y en todos los nodos (`DESCRIBE Pagos;` con `ultimos_digitos`; `Tickets.total` en `VIRTUAL`).
- [ ] El central responde a `SHOW ALL SLAVES STATUS` con todas las sucursales en `Yes/Yes` y retraso 0.
- [ ] Hay un respaldo reciente del central (sección 8).
- [ ] La contraseña de `farmacia_registro` está anotada en un lugar seguro, y la de este documento (`CREDENCIALES.local.md`) no se subió al repositorio.

---

## Anexo A — Limpiar en el central los datos de **una** sucursal

**Solo** si reiniciaste la base local de una sucursal que ya había subido ventas (R-08) y lo decide el responsable. Se ejecuta **en el central** (SSH + `sudo mariadb`). No borra catálogo, personal ni cuentas, solo lo que esa sucursal escribe. Se ejecuta con el registro binario apagado en esa sesión para que nada se propague.

1. Detén su réplica y toma nota de la sucursal (`@s` = su `id_sucursal`: 1, 11, 21…):
   ```
   STOP SLAVE 'sucursal_11';
   ```
2. Vista previa (revisa que las cifras son las que esperas):
   ```
   SET @s = 11;
   SELECT 'Tickets', COUNT(*) FROM farmacia.Tickets WHERE id_sucursal=@s
   UNION ALL SELECT 'Lotes', COUNT(*) FROM farmacia.Lotes WHERE id_sucursal=@s
   UNION ALL SELECT 'Clientes', COUNT(*) FROM farmacia.Clientes WHERE id_sucursal=@s
   UNION ALL SELECT 'Devoluciones', COUNT(*) FROM farmacia.Devoluciones WHERE id_sucursal=@s;
   ```
3. Borrado, en **una sola sesión** y dentro de una transacción (respeta el orden por las claves foráneas):
   ```
   SET SESSION sql_log_bin = 0;
   USE farmacia;
   SET @s = 11;
   START TRANSACTION;
   DELETE dd FROM Detalle_Devolucion dd JOIN Devoluciones d ON d.id_devolucion = dd.id_devolucion WHERE d.id_sucursal = @s;
   DELETE FROM Devoluciones WHERE id_sucursal = @s;
   DELETE p FROM Pagos p JOIN Tickets t ON t.id_ticket = p.id_ticket WHERE t.id_sucursal = @s;
   DELETE v FROM Ventas v JOIN Tickets t ON t.id_ticket = v.id_ticket WHERE t.id_sucursal = @s;
   DELETE FROM Tickets WHERE id_sucursal = @s;
   DELETE i FROM Inventario i JOIN Lotes l ON l.id_lote = i.id_lote WHERE l.id_sucursal = @s;
   DELETE FROM Lotes WHERE id_sucursal = @s;
   DELETE FROM Clientes WHERE id_sucursal = @s;
   DELETE FROM Productos_Sucursal WHERE id_sucursal = @s;
   DELETE FROM Bitacora_Cambios WHERE id_sucursal = @s;
   -- Revisa los conteos de la vista previa (ahora deben ser 0) y SOLO entonces:
   COMMIT;      -- o ROLLBACK; si algo no cuadra
   ```
   (El `DELETE` de `Ventas` dispara el trigger que devuelve stock a los lotes; como los lotes se borran después, es inofensivo.)
4. Rehaz el asistente en la sucursal (Nivel 1) para que `START SLAVE 'sucursal_11'` y todo quede en marcha, o directamente: `START SLAVE 'sucursal_11';` si el nodo ya está configurado.
5. Comprueba `SHOW ALL SLAVES STATUS\G`: ambos hilos `Yes`, sin errores.

## Anexo B — Comandos de referencia (Windows)

| Para | Comando |
|---|---|
| Ver servicios de MariaDB | `sc query MariaDB` |
| Reiniciar el servicio | `net stop MariaDB` y `net start MariaDB` (cmd como administrador) |
| Abrir el cliente | `"C:\Program Files\MariaDB 11.4\bin\mariadb.exe" -u root -p -h 127.0.0.1 -P 3306` |
| Ver la IP de Tailscale | `tailscale ip -4` |
| Probar el central | `Test-NetConnection ip-172-31-41-253.tail560a17.ts.net -Port 3306` (PowerShell) |
| Reinicio de la app (Nivel 1) | `reg delete "HKCU\Software\Farmacia" /f` y `rmdir /s /q "%APPDATA%\Farmacia"` |
| Ver los ajustes guardados | `reg query "HKCU\Software\Farmacia" /s` |
| Estado de réplica local | `SHOW ALL REPLICAS STATUS\G` |
| Estado de réplica en el central | `SHOW ALL SLAVES STATUS\G` |

## Anexo C — Qué no cubre este plan (honestidad)

- **Hay flujos sin probar de punta a punta por el autor** (por eso están en el plan): una venta de un empleado que llegue al central por la réplica de subida (N-01), el flujo completo de tarjeta en la interfaz (V-23…V-48), y la demo `demo.sh`.
- Los resultados esperados que dicen «según el diseño» o «anota» son comportamientos que no están fijados en esta documentación: tu observación **define** el comportamiento real y debe volcarse al plan.
- La restricción de «encargado» vive en la aplicación; alguien con acceso directo a la base local podría saltarla. Es una limitación conocida, no un defecto de las pruebas.
- Con la sucursal Norte ya registrada en un equipo Windows previo, **no** repitas su alta en otro equipo.
