-- Datos que se capturan SOLO en el central y bajan por replicación a las
-- sucursales: sucursales, empleados, usuarios y catálogo maestro.
-- IDs explícitos: el auto_increment de cada nodo tiene desfase propio.
USE farmacia;

INSERT INTO Sucursales (id_sucursal, nombre, direccion, telefono, rol, estado) VALUES
    (1, 'Central', 'Av. Principal 100', '5550000001', 'central',  'activa'),
    (2, 'Norte',   'Calle Norte 20',    '5550000002', 'sucursal', 'activa'),
    (3, 'Sur',     'Calle Sur 30',      '5550000003', 'sucursal', 'activa');

INSERT INTO Categorias (id_categoria, nombre, tipo, descripcion) VALUES
    (1, 'Medicamentos',    'medicamento', 'Medicamentos de venta libre y con receta'),
    (2, 'Higiene personal', 'higiene',    'Productos de cuidado personal');

INSERT INTO Proveedores (id_proveedor, nombre_empresa, telefono, correo, direccion, estado) VALUES
    (1, 'Distribuidora Médica SA', '5551112222', 'ventas@dismed.com', 'Calle Industria 1', 'activo');

INSERT INTO Productos (id_producto, nombre, marca, descripcion, codigo_barras, id_categoria, id_proveedor,
                       precio_sugerido, requiere_receta, estado) VALUES
    (1, 'Paracetamol 500mg', 'GenFar',   'Caja con 20 tabletas', '7501000000011', 1, 1,  28.00, FALSE, 'activo'),
    (2, 'Amoxicilina 500mg', 'Farmex',   'Caja con 12 cápsulas', '7501000000028', 1, 1,  70.00, TRUE,  'activo'),
    (3, 'Jabón antibacterial', 'Cuidamax', 'Barra 120g',          '7501000000035', 2, 1,  18.00, FALSE, 'activo');

INSERT INTO Empleados (id_empleado, id_sucursal, nombre, apellido_paterno, puesto, estado) VALUES
    (1, 1, 'Admin', 'Central', 'Administrador', 'activo'),
    (2, 2, 'Ana',   'García',  'Farmacéutico',  'activo'),
    (3, 3, 'Luis',  'Martínez', 'Cajero',       'activo');

-- Misma fórmula que PasswordUtil: SHA-256 de (sal + contraseña), en hexadecimal.
INSERT INTO Usuarios (id_usuario, id_empleado, nombre_usuario, contrasena_hash, sal, rol, estado) VALUES
    (1, 1, 'admin', SHA2(CONCAT('saladminsaladminsaladminsaladm', 'admin123'), 256), 'saladminsaladminsaladminsaladm', 'admin',    'activo'),
    (2, 2, 'ana',   SHA2(CONCAT('salanasalanasalanasalanasalana', 'ana123'),   256), 'salanasalanasalanasalanasalana', 'empleado', 'activo'),
    (3, 3, 'luis',  SHA2(CONCAT('salluissalluissalluissalluissa', 'luis123'),  256), 'salluissalluissalluissalluissa', 'empleado', 'activo');
