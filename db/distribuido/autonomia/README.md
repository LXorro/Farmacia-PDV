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
  cuenta `farmacia_remoto` (solo escribe `Usuarios`; lee `Empleados` y
  `Sucursales`). Su contraseña se pide al usarla y no se guarda. Sin conexión al
  central no se pueden crear cuentas; las existentes siguen entrando.
- En el asistente, la función **Central** crea esa cuenta y muestra su
  contraseña una sola vez; **Sucursal** pide la dirección del central.
  (`demo.sh init` la crea con la contraseña `remoto_pw`.)
- Pendiente: TLS en esa conexión (necesario al exponer el central en internet).

## Un solo escritor por tabla (lo hace cumplir la base)

El asistente de la app da a `farmacia_app` permisos según la función del equipo:

| función | escribe | solo lee |
|---|---|---|
| Central | sucursales, empleados, usuarios, asignaciones, catálogo | clientes, lotes, inventario, tickets, ventas, pagos, devoluciones |
| Sucursal | clientes, lotes, inventario, tickets, ventas, pagos, devoluciones | lo que baja del central |
| Equipo único | todo | — |

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

## Límites conocidos

- Un admin no puede corregir desde el central el stock ni los tickets de una
  sucursal: lo hace la sucursal. Es el precio de tener un único escritor.
- La replicación asíncrona no resuelve conflictos; funciona porque cada tabla
  tiene un único nodo escritor.
- La contraseña de replicación (`repl_pw`) es de demostración.
