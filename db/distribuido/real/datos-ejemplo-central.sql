-- Datos de ejemplo del CENTRAL: todo lo que escribe el administrador (Sucursales, Empleados, Usuarios,
-- Asignaciones temporales, Categorías, Proveedores y Productos), con un poco de cada estado.
-- Se ejecutan CON registro binario a propósito: bajan a las sucursales por replicación.
-- Idempotente (se puede repetir sin duplicar). Requiere la migración 002 ya aplicada (precio_sugerido).
--
--   ssh <central> 'sudo mariadb' < db/distribuido/real/datos-ejemplo-central.sql
--
-- Cuentas (solo desarrollo):  ana / Ana2026 · luis / Luis2026 · jorge / Jorge2026 (empleados)
--                             marta / Marta2026 (administradora) · pedro / Pedro2026 (inactiva)
USE farmacia;

-- Sucursales («Sucursal Principal» y «Sucursal Norte» normalmente ya existen).
INSERT INTO Sucursales (nombre, direccion, telefono, rol, estado)
SELECT n, d, t, 'sucursal', e FROM (
    SELECT 'Sucursal Principal' n, 'Av. Principal 100' d, '5550000001' t, 'activa' e UNION ALL
    SELECT 'Sucursal Norte', 'Calle Norte 45', '5550000002', 'activa' UNION ALL
    SELECT 'Sucursal Sur', 'Blvd. Sur 780', '5550000003', 'inactiva') x
WHERE NOT EXISTS (SELECT 1 FROM Sucursales s WHERE s.nombre = x.n);

INSERT INTO Categorias (nombre, tipo, descripcion)
SELECT n, t, d FROM (
    SELECT 'Analgésicos' n, 'medicamento' t, 'Medicamentos para el dolor' d UNION ALL
    SELECT 'Antibióticos', 'medicamento', 'Requieren receta médica' UNION ALL
    SELECT 'Antigripales', 'medicamento', 'Resfriado, gripe y tos' UNION ALL
    SELECT 'Higiene personal', 'higiene', 'Productos de cuidado personal' UNION ALL
    SELECT 'Suplementos', 'suplemento', 'Vitaminas y suplementos alimenticios' UNION ALL
    SELECT 'Cuidado del bebé', 'bebé', 'Pañales, fórmulas y accesorios') x
WHERE NOT EXISTS (SELECT 1 FROM Categorias c WHERE c.nombre = x.n);

INSERT INTO Proveedores (nombre_empresa, telefono, correo, direccion, fecha_ultima_compra, estado)
SELECT n, t, c, d, u, e FROM (
    SELECT 'Distribuidora Farmex' n, '5551112222' t, 'ventas@farmex.com' c, 'Parque Industrial 1' d, DATE_SUB(CURRENT_DATE, INTERVAL 5 DAY) u, 'activo' e UNION ALL
    SELECT 'Laboratorios Salud SA', '5553334444', 'contacto@labsalud.com', 'Zona Industrial 2', DATE_SUB(CURRENT_DATE, INTERVAL 20 DAY), 'activo' UNION ALL
    SELECT 'Droguería del Norte', '5557778888', 'pedidos@drognorte.com', 'Calle Comercio 12', DATE_SUB(CURRENT_DATE, INTERVAL 200 DAY), 'suspendido' UNION ALL
    SELECT 'Importadora Vital', '5559990000', 'info@vital.com', 'Av. del Puerto 300', NULL, 'inactivo') x
WHERE NOT EXISTS (SELECT 1 FROM Proveedores p WHERE p.nombre_empresa = x.n);

-- Catálogo: identidad del producto y precio SUGERIDO; cada sucursal fija el suyo en Productos_Sucursal.
INSERT IGNORE INTO Productos (nombre, marca, descripcion, codigo_barras, id_categoria, id_proveedor, precio_sugerido, requiere_receta, estado)
SELECT x.nombre, x.marca, x.descr, x.codigo,
       (SELECT id_categoria FROM Categorias WHERE nombre = x.cat),
       (SELECT id_proveedor FROM Proveedores WHERE nombre_empresa = x.prov),
       x.precio, x.receta, x.estado
FROM (
    SELECT 'Paracetamol 500mg' nombre, 'GenFar' marca, 'Caja con 20 tabletas' descr, '7501000000011' codigo, 'Analgésicos' cat, 'Distribuidora Farmex' prov, 28.00 precio, FALSE receta, 'activo' estado UNION ALL
    SELECT 'Ibuprofeno 400mg', 'GenFar', 'Caja con 10 tabletas', '7501000000059', 'Analgésicos', 'Distribuidora Farmex', 35.00, FALSE, 'activo' UNION ALL
    SELECT 'Naproxeno 250mg', 'Farmex', 'Caja con 30 tabletas', '7501000000066', 'Analgésicos', 'Distribuidora Farmex', 62.00, FALSE, 'activo' UNION ALL
    SELECT 'Amoxicilina 500mg', 'Farmex', 'Caja con 12 cápsulas', '7501000000028', 'Antibióticos', 'Distribuidora Farmex', 70.00, TRUE, 'activo' UNION ALL
    SELECT 'Azitromicina 500mg', 'Farmex', 'Caja con 3 tabletas', '7501000000073', 'Antibióticos', 'Laboratorios Salud SA', 145.00, TRUE, 'activo' UNION ALL
    SELECT 'Loratadina 10mg', 'AlerMed', 'Caja con 10 tabletas', '7501000000103', 'Antigripales', 'Laboratorios Salud SA', 38.00, FALSE, 'activo' UNION ALL
    SELECT 'Jarabe para la tos 120ml', 'RespiraMás', 'Frasco con dosificador', '7501000000110', 'Antigripales', 'Droguería del Norte', 76.00, FALSE, 'activo' UNION ALL
    SELECT 'Jabón antibacterial', 'Cuidamax', 'Barra 120g', '7501000000035', 'Higiene personal', 'Laboratorios Salud SA', 18.00, FALSE, 'activo' UNION ALL
    SELECT 'Alcohol en gel 250ml', 'Cuidamax', 'Frasco con atomizador', '7501000000080', 'Higiene personal', 'Laboratorios Salud SA', 42.00, FALSE, 'activo' UNION ALL
    SELECT 'Multivitamínico', 'VitaPlus', 'Frasco 30 tabletas', '7501000000042', 'Suplementos', 'Laboratorios Salud SA', 110.00, FALSE, 'activo' UNION ALL
    SELECT 'Vitamina C 1g', 'VitaPlus', 'Tubo con 10 efervescentes', '7501000000097', 'Suplementos', 'Laboratorios Salud SA', 85.00, FALSE, 'activo' UNION ALL
    SELECT 'Pañales etapa 3', 'BabySoft', 'Paquete con 40 piezas', '7501000000127', 'Cuidado del bebé', 'Importadora Vital', 189.00, FALSE, 'activo' UNION ALL
    SELECT 'Jarabe infantil sabor fresa', 'Pediátrica', 'Frasco 100ml (retirado del mercado)', '7501000000134', 'Antigripales', 'Droguería del Norte', 64.00, FALSE, 'descontinuado') x;

-- Personal: Ana, Luis y Pedro (ya no trabaja aquí) en la Principal; Jorge y Sofía en Norte.
INSERT INTO Empleados (id_sucursal, nombre, apellido_paterno, apellido_materno, puesto, telefono, correo, estado)
SELECT (SELECT id_sucursal FROM Sucursales WHERE nombre = x.suc), x.nombre, x.ap, x.am, x.puesto, x.tel, x.correo, x.estado
FROM (
    SELECT 'Sucursal Principal' suc, 'Ana' nombre, 'López' ap, 'Cruz' am, 'Farmacéutico' puesto, '5559990001' tel, 'ana.lopez@farmacia.com' correo, 'activo' estado UNION ALL
    SELECT 'Sucursal Principal', 'Luis', 'Ramírez', 'Pérez', 'Cajero', '5559990002', 'luis.ramirez@farmacia.com', 'activo' UNION ALL
    SELECT 'Sucursal Principal', 'Pedro', 'Salas', NULL, 'Cajero', '5559990003', NULL, 'inactivo' UNION ALL
    SELECT 'Sucursal Norte', 'Jorge', 'Hernández', 'Ruiz', 'Cajero', '5559990004', 'jorge.hernandez@farmacia.com', 'activo' UNION ALL
    SELECT 'Sucursal Norte', 'Sofía', 'Mendoza', 'Gil', 'Farmacéutico', '5559990005', 'sofia.mendoza@farmacia.com', 'activo') x
WHERE NOT EXISTS (SELECT 1 FROM Empleados e WHERE e.nombre = x.nombre AND e.apellido_paterno = x.ap);

-- Cuentas. Hash = SHA-256(sal + contraseña) en hexadecimal, igual que PasswordUtil de la app.
INSERT INTO Usuarios (nombre_usuario, id_empleado, contrasena_hash, sal, rol, estado) VALUES
    ('ana', (SELECT id_empleado FROM Empleados WHERE nombre = 'Ana' AND apellido_paterno = 'López'), '01c7c1709cda161023b2e542cbb76ea8c85f0319dfa48fd2a9f5fc7a4d136635', 'gShElxPkJDm3HrhiVqKmvGlyuqOpNQrr', 'empleado', 'activo'),
    ('luis', (SELECT id_empleado FROM Empleados WHERE nombre = 'Luis' AND apellido_paterno = 'Ramírez'), '399d9995d8e14a4bd841487e197e3d0608331ca90a75a1a443cd473fa1c2d295', 'bEBgQELsw11CdYbm8HLoJJiO36PHqSme', 'empleado', 'activo'),
    ('jorge', (SELECT id_empleado FROM Empleados WHERE nombre = 'Jorge' AND apellido_paterno = 'Hernández'), '87c9bbd335e86d03c2d877f4bd21da352fe4e3054d77f8fbb8b97933c4d42c6f', 'Et2C1ZQushzJSzA7Hybl7Wy5ghKw0CnD', 'empleado', 'activo'),
    ('marta', NULL, 'd8a5fbe852d66f5dc25ff9bc8c556f4ca088bdd4f6b99398805d1eb5889b1c02', 's5KeJbdtXA2pJ1ZNtBBjIRZiIAxpvK1L', 'admin', 'activo'),
    ('pedro', (SELECT id_empleado FROM Empleados WHERE nombre = 'Pedro' AND apellido_paterno = 'Salas'), '88fac2601eb1735a8eb1868c6544be7d7eb984cdce4de5a9d1bbce60cbc60e8d', 'MiUCywWCcxoix3a19ByBMf2welKrZe9Y', 'empleado', 'inactivo')
ON DUPLICATE KEY UPDATE nombre_usuario = nombre_usuario;

-- Asignaciones temporales: las autoriza la administradora (marta). Una vigente y una ya revocada.
INSERT INTO Asignaciones_Temporales (id_empleado, id_sucursal, fecha_inicio, fecha_fin, id_autoriza, estado)
SELECT (SELECT id_empleado FROM Empleados WHERE nombre = x.emp AND apellido_paterno = x.ap),
       (SELECT id_sucursal FROM Sucursales WHERE nombre = x.suc),
       x.inicio, x.fin, (SELECT id_usuario FROM Usuarios WHERE nombre_usuario = 'marta'), x.estado
FROM (
    SELECT 'Jorge' emp, 'Hernández' ap, 'Sucursal Principal' suc, DATE_SUB(CURRENT_DATE, INTERVAL 2 DAY) inicio, DATE_ADD(CURRENT_DATE, INTERVAL 14 DAY) fin, 'aprobada' estado UNION ALL
    SELECT 'Luis', 'Ramírez', 'Sucursal Norte', DATE_SUB(CURRENT_DATE, INTERVAL 30 DAY), DATE_SUB(CURRENT_DATE, INTERVAL 16 DAY), 'revocada') x
WHERE NOT EXISTS (SELECT 1 FROM Asignaciones_Temporales a
                  WHERE a.id_empleado = (SELECT id_empleado FROM Empleados WHERE nombre = x.emp AND apellido_paterno = x.ap)
                    AND a.id_sucursal = (SELECT id_sucursal FROM Sucursales WHERE nombre = x.suc)
                    AND a.estado = x.estado);
