# Demo de autonomía local (central + sucursales con el esquema real)

Tres instancias MariaDB desechables con el `db/schema.sql` real de la app,
replicadas en ambos sentidos. Es independiente de la demo de `../` (otros
puertos) y del MariaDB del sistema.

| nodo | puerto | offset auto_increment |
|---|---|---|
| central | 13310 | 1 |
| norte | 13311 | 2 |
| sur | 13312 | 3 |

```
./demo.sh init       # crea los 3 nodos, esquema, replicación y datos de ejemplo
./demo.sh prueba     # escenario automático con verificaciones (corte y recuperación)
./demo.sh status | corte | restaurar | start | stop | limpiar
```

## Topología

- **Hacia abajo (central → sucursales):** `Sucursales`, `Empleados`, `Usuarios`,
  `Categorias`, `Proveedores`, `Productos`. Solo se capturan en el central; el
  login y el catálogo funcionan sin conexión porque ya están en cada sucursal.
- **Hacia arriba (sucursal → central):** `Clientes`, `Lotes`, `Inventario`,
  `Tickets`, `Ventas`, `Pagos`, `Devoluciones`, `Detalle_Devolucion`. Cada
  sucursal escribe solo sus filas; el central las consolida con replicación
  multi-origen (una conexión por sucursal, con su propio filtro).
- **Sin conflictos de ID:** `auto_increment_increment=10` y un `offset` distinto
  por nodo; los IDs generados nunca coinciden.
- **`binlog_format=ROW`:** necesario para que los triggers (stock, subtotal,
  estado de pago) no se disparen una segunda vez en el nodo que replica.
- **Esquema sin binlog:** cada nodo carga el esquema con `sql_log_bin=0`;
  la replicación transporta solo datos.

## Probar con la app

Con los nodos arriba, abre la app: en el asistente indica el puerto del nodo
(p. ej. `13311` para norte), usuario `root` y contraseña vacía. El asistente
detecta que las tablas ya existen, solo crea el usuario de la app y guarda el
puerto. Usuarios de ejemplo: `admin/admin123`, `ana/ana123` (norte),
`luis/luis123` (sur). Para cambiar de nodo, borra `~/.config/Farmacia/`.

Simula la caída con `./demo.sh corte`; las ventas siguen y, tras
`./demo.sh restaurar`, el central las recibe.

## Cuentas de usuario

- No hay registro libre: las crea un admin desde la pantalla Usuarios. Solo si
  la tabla está vacía, el login ofrece crear el administrador inicial (nunca en
  una sucursal, que las recibe del central).
- Un admin en una sucursal crea cuentas **conectándose al central** con la
  cuenta `farmacia_remoto`. Su contraseña se pide al iniciar sesión y no se guarda.
  Sin conexión al central, el admin puede entrar en modo local (solo lectura de
  lo replicado); las cuentas existentes siguen entrando.
- Esa cuenta y `farmacia_registro` las crea `db/distribuido/real/preparar-central.sh`
  en el central (ver «Modelo por sesión»). `demo.sh init` crea la primera con la
  contraseña `remoto_pw`.
- Esa conexión va sobre TLS cuando el central es un servidor real expuesto más
  allá de la red local (ver «Central real sobre Tailscale» más abajo).

## Modelo por sesión (sin elegir «central» o «sucursal»)

El equipo ya no tiene una función fija. El asistente de primer arranque pide el
central (dirección Tailscale, cuenta `farmacia_registro` y su contraseña, que no se
guardan) y la sucursal de este equipo, y deja la replicación andando en ambos
sentidos: cuentas de replicación en cada lado, copia inicial de lo que baja del
central con su posición GTID exacta, `CHANGE MASTER` de la sucursal hacia el central
y del central hacia la sucursal (conexión con nombre `sucursal_<id>`).

- **Empleado:** trabaja sobre la base local (su sucursal), con o sin conexión.
- **Administrador:** al iniciar sesión se conecta al central con `farmacia_remoto`
  y esa conexión pasa a ser la de toda la sesión: ve todas las sucursales y
  administra catálogo, personal, sucursales y cuentas. Sin conexión puede entrar en
  modo local, de solo lectura para lo que administra el central.
- **Único paso manual:** MariaDB no permite activar `log-bin` ni el `server-id` en
  caliente. Si falta, el asistente lo detecta y muestra el `my.cnf` a usar;
  `db/distribuido/real/preparar-nodo.sh <ranura>` lo escribe y reinicia MariaDB.
  La ranura (2..10) sale del id de la sucursal y fija `server-id`, `gtid_domain_id`
  y `auto_increment_offset`.
- **Central:** `ssh <central> 'sudo bash -s' < db/distribuido/real/preparar-central.sh`
  crea `farmacia_registro` y ajusta `farmacia_remoto`.
- Solo Linux por ahora: en Windows el `my.ini` hay que escribirlo a mano con el mismo contenido.

## Un solo escritor por tabla (lo hace cumplir la base)

Los permisos dependen de la cuenta con la que se conecta cada sesión:

| conexión | escribe | solo lee |
|---|---|---|
| `farmacia_remoto` (admin, en el central) | sucursales, empleados, usuarios, asignaciones, catálogo | clientes, lotes, inventario, tickets, ventas, pagos, devoluciones |
| `farmacia_app` de una sucursal (empleado) | clientes, lotes, inventario, tickets, ventas, pagos, devoluciones | lo que baja del central |
| `farmacia_app` de un equipo único | todo | — |

El central **no vende**: Punto de Venta no existe ahí y Clientes, Inventario y
Tickets son de consulta (el admin ve lo de todas las sucursales). Así ninguna
fila tiene dos escritores y no hay conflictos de edición. En una sucursal,
Productos, Categorías, Proveedores, Empleados y Sucursales son de solo lectura
(cualquier intento de escribir ahí lo rechaza MariaDB).

## Operar en otra sucursal (asignaciones temporales)

Un admin autoriza en el central a un empleado a operar en otra sucursal durante
un periodo (pantalla *Asignaciones*; la autorización es la aprobación y queda
registrado quién la dio). La asignación baja a las sucursales por replicación.
El empleado solo puede aprovecharla **iniciando sesión en un equipo de esa
sucursal** — por eso sus ventas quedan en el fragmento de la sucursal donde
está físicamente, y funciona sin conexión.

- Cada equipo de sucursal conoce su sucursal: el primer admin que inicia sesión
  ahí la elige (una vez; viene de la lista que replicó el central).
- Un empleado de otra sucursal sin asignación vigente no entra. En el equipo
  central solo entran admins.
- Las asignaciones se administran desde el central.

## Indicador de sincronización

La barra de estado del app (solo en Central y Sucursal) muestra el estado de la replicación, y se
actualiza cada 10 s con `SHOW ALL REPLICAS STATUS` sobre la base local (la cuenta `farmacia_app`
recibe `REPLICA MONITOR`, que solo permite ver ese estado):

- verde **Sincronizado**: los hilos de replicación corren y el retraso es ≤ 30 s.
- naranja **Sincronizando (retraso N s)**.
- rojo **Sin conexión con el central (trabajando en local)**; en el central dice con qué sucursales.

Desde la sucursal solo se ve la replicación *de bajada* (central → sucursal); si la de subida
va atrasada, se nota en el central.

## Central real sobre Tailscale

Para probar esta capa de autonomía con un central que no es `localhost` (p. ej.
una VPS), la red privada se hace con [Tailscale](https://tailscale.com) en vez
de abrir el puerto 3306 al público:

- Central y sucursales se unen al mismo tailnet; MariaDB solo escucha ese
  puerto en la interfaz `tailscale0` (`ufw` lo deja así de fábrica), nunca en
  la IP pública.
- El driver `QMYSQL` que usa la app **exige TLS siempre** para esta conexión
  (no es opcional ni algo que se pueda desactivar desde `setConnectOptions`:
  `MYSQL_OPT_SSL_MODE` no lo reconoce este driver, y sin certificado el
  servidor responde «SSL is required, but the server does not support it»).
  Por eso el central necesita TLS sí o sí, aunque el tráfico ya viaje cifrado
  dentro del túnel de Tailscale.
- En vez de un certificado autofirmado (que obliga a distribuirlo a cada
  sucursal e importarlo al almacén de confianza de Windows — frágil y no
  escala), se usa el certificado HTTPS que emite el propio Tailscale, firmado
  por Let's Encrypt:

  1. Activar **HTTPS Certificates** en <https://login.tailscale.com/admin/dns>
     (una vez por tailnet).
  2. En el central: `sudo tailscale cert <nombre-magicdns>.ts.net` — genera un
     `.crt`/`.key` de verdad, confiado por cualquier sistema sin configuración
     extra.
  3. Apuntar `ssl-cert`/`ssl-key` de MariaDB a esos archivos y reiniciar.
  4. En la app, la dirección del central (`central/host`, pantalla de
     configuración) debe ser ese **nombre `.ts.net`**, no la IP de Tailscale —
     el certificado es válido para ese nombre, no para la IP, y Windows
     rechaza la conexión por «CN/SAN no coincide» si no coincide exactamente.
- Ese certificado expira (Let's Encrypt, ~90 días); conviene automatizar su
  renovación con un cron que repita el paso 2 y reinicie MariaDB.
- Con esto, la app no necesita ningún manejo especial de certificados: el
  código de conexión al central (`DatabaseManager::conectarCentral`) es TLS
  normal, confiado por la cadena de certificación pública del sistema.

## Límites conocidos

- Un admin no puede corregir desde el central el stock ni los tickets de una
  sucursal: lo hace la sucursal. Es el precio de tener un único escritor.
- La replicación asíncrona no resuelve conflictos; funciona porque cada tabla
  tiene un único nodo escritor.
- La contraseña de replicación (`repl_pw`) es de demostración.
