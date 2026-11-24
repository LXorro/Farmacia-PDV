# Hallazgos de las pruebas automatizadas

Generado con la suite de `app/tests/` (Qt Test, plataforma `offscreen`, contra una MariaDB desechable en el puerto
33307; nunca contra la base real). Estado de la última ejecución completa:

| Suite | Qué prueba | Resultado |
|---|---|---|
| `validacion` | validadores y filtros con 138 entradas hostiles (SQL, HTML, NUL, Unicode, textos de 5 MB) | **138/138** |
| `humo` | abre las 15 pantallas y 19 diálogos con ambos roles y pulsa todos los botones (con y sin fila seleccionada, confirmando borrados) | **72/72**, sin cierres ni errores técnicos |
| `formularios` | fuzz de 8 diálogos: un campo hostil/extremo cada vez, y escritura tecla a tecla | 38/42 → **hallazgo 4** |
| `ventas` | punto de venta manejado como usuario | 25/28 → **hallazgos 1, 5, 6** |
| `acceso` | menús por rol, permisos temporales, login | 30/33 → **hallazgos 2, 3** |
| `bd` | ¿acepta MariaDB datos imposibles? | 11/29 → **hallazgo 7** |

Ninguna prueba provocó un cierre inesperado (crash) de la aplicación.

## Hallazgos (de mayor a menor impacto)

### 1. Sobreventa: el stock de un lote puede quedar negativo — ALTO — ✅ CORREGIDO
**Reproducir:** en el punto de venta, lote de 50 unidades; agregar el mismo producto con cantidad 40 **dos veces** y cobrar.
**Resultado:** el lote queda en **-30**.
**Causa:** `PuntoVentaPage::agregarAlCarrito` compara la cantidad pedida con el stock del lote sin sumar lo que ya hay en el
carrito, y `cobrar()` revisa el stock línea por línea (`FOR UPDATE`) sin agrupar por lote.
**Arreglo:** sumar por `idLote` las líneas del carrito al agregar y al cobrar; y como red de seguridad
`ALTER TABLE Lotes ADD CONSTRAINT ck_lotes_stock CHECK (cantidad_disponible >= 0)`.
Test: `test_ventas  sobreventa_agregando_el_mismo_producto_varias_veces`.
**Corrección aplicada** (`puntoventapage.cpp`): al agregar se descuenta lo ya apartado en el carrito y se usa el primer lote donde
quepa; al cobrar se agrupa por lote antes de comparar con el stock. No se añadió el `CHECK` de la base (ver hallazgo 7).

### 2. Se puede desactivar o degradar al único administrador — ALTO — ✅ CORREGIDO
**Reproducir:** Usuarios → editar la cuenta admin → Estado «inactivo» (o Rol «empleado») → Guardar.
**Resultado:** queda **0 administradores activos**; nadie puede volver a administrar el sistema sin tocar la base a mano.
`UsuariosPage::eliminarUsuario` sí impide borrarse a sí mismo, pero `UsuarioDialog::guardar` no tiene esa protección.
**Arreglo:** en `UsuarioDialog::guardar`, si el usuario editado es admin activo y el cambio lo deja inactivo/empleado, contar
los admin activos restantes y rechazar si no queda ninguno (y avisar si se desactiva a sí mismo).
Tests: `test_acceso  no_se_puede_desactivar_al_unico_admin`, `no_se_puede_degradar_al_unico_admin`.
**Corrección aplicada** (`usuariodialog.cpp`): al editar una cuenta admin activa se exige que quede otro admin activo, y no se
permite quitarse el rol ni desactivar la propia cuenta con la sesión abierta.

### 3. El login no limita intentos fallidos — MEDIO
**Resultado:** tras 25 contraseñas incorrectas seguidas, la correcta entra de inmediato (permite fuerza bruta).
Además la contraseña se guarda con un solo SHA-256 con sal (`PasswordUtil`), que es rápido de atacar si se filtra la tabla.
**Arreglo:** retraso creciente o bloqueo temporal tras N fallos; y un KDF lento (PBKDF2 con muchas iteraciones, Argon2) en vez de un hash simple.
Test: `test_acceso  login_sin_bloqueo_por_intentos`.

### 4. Sucursales y Categorías no normalizan los espacios — BAJO
**Reproducir:** crear una sucursal (o categoría) llamada `Zeta   Doble    Espacio` tecleando los espacios.
**Resultado:** se guarda con los espacios repetidos (Producto, Proveedor, Cliente y Empleado sí los normalizan). Permite
nombres «distintos» que se ven iguales.
**Causa:** `SucursalDialog` y `CategoriaDialog` usan `.trimmed()` en vez de `Validacion::normalizar()` al guardar nombre,
dirección y tipo.
Test: `test_formularios  escritura_real`.

### 5. La búsqueda del punto de venta no escapa los comodines de LIKE — BAJO
**Reproducir:** escribir `%` en «buscar producto» y pulsar Agregar → agrega un producto cualquiera.
También: `LIMIT 1` sin `ORDER BY`, así que un texto como «para» agrega un producto arbitrario sin avisar si hay varios.
**Arreglo:** escapar `%`, `_` y `\` en el texto antes de armar el `LIKE`, y mostrar una lista cuando hay más de una coincidencia.
Test: `test_ventas  comodin_en_busqueda_no_debe_coincidir_con_todo`.

### 6. El POS vende un producto con precio $0 — BAJO (defensa en profundidad)
Desde la interfaz **no se llega** a ese estado (`ProductoDialog` exige ≥ 0.01 y un 0 en el precio por sucursal se guarda
como «usar el sugerido»). Aparece solo si el dato llega por otro camino (replicación, edición directa, datos viejos):
el POS no lo revalida y la base lo permite (`precio_sugerido DEFAULT 0`).
Test: `test_ventas  producto_con_precio_cero`.

### 7. La base de datos casi no tiene restricciones `CHECK` — MEDIO (diseño)
Solo existe una (`ck_permisos_duracion`, y esa sí funciona: rechaza permisos de más de 72 h). La BD **acepta**, por ejemplo:
cantidad de venta 0 o negativa (una venta de -1000 unidades *sube* el stock a 1050), precio negativo, stock negativo,
`cantidad_disponible > cantidad_inicial`, pagos negativos o mayores que el total, descuento mayor que el subtotal,
devolver más de lo vendido (el stock sube), caducidad anterior al ingreso, costo/precio negativos, hash de contraseña vacío,
ticket de una sucursal vendido por un empleado de otra.
Hoy solo la interfaz lo impide; cualquier otra ruta de escritura (otro programa, la réplica, un error futuro) lo guardaría.
**Arreglo:** agregar `CHECK` y/o triggers de validación (un bloque `ALTER TABLE … ADD CONSTRAINT …` por tabla).
Tests: todos los de `test_bd`.

## Observaciones sin fallo
- La autorización por rol vive en la interfaz (menús y botones ocultos) y en `LoteDialog`/`ProductoSucursalDialog`; los demás
  diálogos no comprueban el rol al guardar y la cuenta de MariaDB es la misma para todos. No hay ruta en la interfaz para que un
  empleado llegue a ellos (el test `menu_por_rol` lo comprueba), pero no hay una segunda barrera.
- La colación `utf8mb4_unicode_ci` hace que `ＡＤＭＩＮ`, `admín` y `ADMIN` entren como `admin`. No es escalada (la contraseña
  sigue siendo obligatoria); conviene saberlo.
- Correcto y verificado: SQL injection en login, búsqueda y los 8 formularios (consultas preparadas); el login no distingue
  «usuario inexistente» de «contraseña mala»; cuentas inactivas no entran; permisos temporales vencidos, futuros o
  revocados no habilitan nada; un empleado con permiso no puede elegir otra sucursal; el permiso que vence con el formulario
  abierto se vuelve a comprobar al guardar.

## Cómo ejecutarlo
```bash
cd app/tests
./bd_pruebas.sh start                      # BD desechable en el puerto 33307
FARMACIA_TEST_SOCKET=/tmp/farmacia-test-db/m.sock ./run_tests.sh          # todas las suites
FARMACIA_TEST_SOCKET=/tmp/farmacia-test-db/m.sock ./run_tests.sh acceso   # una sola
./bd_pruebas.sh stop
```
Rechaza correr contra el puerto 3306. Las pruebas usan un `QSettings` temporal, así que no tocan tus ajustes.
