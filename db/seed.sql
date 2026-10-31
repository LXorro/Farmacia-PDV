-- Datos de ejemplo para desarrollo/pruebas locales
USE farmacia;

INSERT INTO Sucursales (nombre, direccion, telefono, rol, estado) VALUES
    ('Central', 'Av. Principal 100', '5550000001', 'central', 'activa'),
    ('Sucursal Norte', 'Calle Norte 45', '5550000002', 'sucursal', 'activa');

INSERT INTO Categorias (nombre, tipo, descripcion) VALUES
    ('Analgésicos', 'medicamento', 'Medicamentos para el dolor'),
    ('Higiene personal', 'higiene', 'Productos de cuidado personal'),
    ('Suplementos', 'suplemento', 'Vitaminas y suplementos alimenticios');

INSERT INTO Proveedores (nombre_empresa, telefono, correo, direccion, estado) VALUES
    ('Distribuidora Farmex', '5551112222', 'ventas@farmex.com', 'Parque Industrial 1', 'activo'),
    ('Laboratorios Salud SA', '5553334444', 'contacto@labsalud.com', 'Zona Industrial 2', 'activo');

INSERT INTO Productos (nombre, marca, descripcion, codigo_barras, id_categoria, id_proveedor, precio_compra, precio_venta, requiere_receta, estado) VALUES
    ('Paracetamol 500mg', 'GenFar', 'Caja con 20 tabletas', '7501000000011', 1, 1, 15.00, 28.00, FALSE, 'activo'),
    ('Amoxicilina 500mg', 'Farmex', 'Caja con 12 cápsulas', '7501000000028', 1, 1, 40.00, 70.00, TRUE, 'activo'),
    ('Jabón antibacterial', 'Cuidamax', 'Barra 120g', '7501000000035', 2, 2, 8.00, 18.00, FALSE, 'activo'),
    ('Multivitamínico', 'VitaPlus', 'Frasco 30 tabletas', '7501000000042', 3, 2, 60.00, 110.00, FALSE, 'activo');

INSERT INTO Empleados (id_sucursal, nombre, apellido_paterno, apellido_materno, puesto, telefono, correo, estado) VALUES
    (1, 'Ana', 'García', 'López', 'Farmacéutico', '5559990001', 'ana.garcia@farmacia.com', 'activo'),
    (2, 'Luis', 'Martínez', 'Pérez', 'Cajero', '5559990002', 'luis.martinez@farmacia.com', 'activo');

INSERT INTO Clientes (id_sucursal, nombre, apellido_paterno, apellido_materno, telefono, correo, direccion, estado) VALUES
    (1, 'María', 'Hernández', 'Ruiz', '5558887777', 'maria.hz@correo.com', 'Calle 5 #123', 'activo'),
    (2, 'Jorge', 'Ramírez', 'Cruz', '5558886666', 'jorge.rc@correo.com', 'Av. Central 200', 'activo');

INSERT INTO Lotes (id_producto, id_sucursal, numero_lote, fecha_ingreso, fecha_caducidad, cantidad_inicial, cantidad_disponible, estado) VALUES
    (1, 1, 'LOTE-A1', CURRENT_DATE, DATE_ADD(CURRENT_DATE, INTERVAL 18 MONTH), 100, 100, 'disponible'),
    (1, 2, 'LOTE-A2', CURRENT_DATE, DATE_ADD(CURRENT_DATE, INTERVAL 18 MONTH), 50, 50, 'disponible'),
    (2, 1, 'LOTE-B1', CURRENT_DATE, DATE_ADD(CURRENT_DATE, INTERVAL 12 MONTH), 40, 40, 'disponible'),
    (3, 1, 'LOTE-C1', CURRENT_DATE, DATE_ADD(CURRENT_DATE, INTERVAL 24 MONTH), 80, 80, 'disponible'),
    (4, 2, 'LOTE-D1', CURRENT_DATE, DATE_ADD(CURRENT_DATE, INTERVAL 20 MONTH), 30, 30, 'disponible');
