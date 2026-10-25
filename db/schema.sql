-- =====================================================================
-- Farmacia — esquema MySQL/MariaDB con capa multi-sucursal
-- Basado en Farmacia_actualizado.md + Plan_Proyecto.md (sección 2)
--
-- Nota sobre fragmentación física: MySQL/MariaDB no permiten FOREIGN KEY
-- en tablas particionadas (ni que sean referenciadas por FK). Para no
-- sacrificar integridad referencial en el sistema en uso, este esquema
-- usa `id_sucursal` como columna normal (fragmentación lógica) en las
-- tablas que la necesitan. El `PARTITION BY` físico para el entregable
-- de fragmentación se demuestra aparte, en db/fragmentacion_demo.sql,
-- sobre copias desnormalizadas de estas tablas.
-- =====================================================================

CREATE DATABASE IF NOT EXISTS farmacia
  CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
USE farmacia;

SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

-- ---------------------------------------------------------------------
-- Sucursales
-- ---------------------------------------------------------------------
CREATE TABLE Sucursales (
    id_sucursal   INT AUTO_INCREMENT PRIMARY KEY,
    nombre        VARCHAR(100) NOT NULL,
    direccion     VARCHAR(200),
    telefono      VARCHAR(20),
    rol           ENUM('central', 'sucursal') NOT NULL DEFAULT 'sucursal',
    estado        ENUM('activa', 'inactiva') NOT NULL DEFAULT 'activa'
) ENGINE=InnoDB;

-- ---------------------------------------------------------------------
-- Catálogo maestro (se define en el central, se replica de solo lectura
-- hacia cada sucursal vía replicación nativa MySQL/MariaDB)
-- ---------------------------------------------------------------------
CREATE TABLE Categorias (
    id_categoria  INT AUTO_INCREMENT PRIMARY KEY,
    nombre        VARCHAR(50) NOT NULL,
    tipo          VARCHAR(50),
    descripcion   TEXT
) ENGINE=InnoDB;

CREATE TABLE Proveedores (
    id_proveedor        INT AUTO_INCREMENT PRIMARY KEY,
    nombre_empresa      VARCHAR(100) NOT NULL,
    telefono            VARCHAR(20),
    correo              VARCHAR(100),
    direccion           VARCHAR(200),
    fecha_registro      DATE NOT NULL DEFAULT (CURRENT_DATE),
    fecha_ultima_compra DATE,
    estado              ENUM('activo', 'inactivo', 'suspendido') NOT NULL DEFAULT 'activo'
) ENGINE=InnoDB;

CREATE TABLE Productos (
    id_producto      INT AUTO_INCREMENT PRIMARY KEY,
    nombre           VARCHAR(100) NOT NULL,
    marca            VARCHAR(100),
    descripcion      TEXT,
    codigo_barras    VARCHAR(50) UNIQUE,
    id_categoria     INT,
    id_proveedor     INT,
    precio_compra    DECIMAL(10,2) NOT NULL DEFAULT 0,
    precio_venta     DECIMAL(10,2) NOT NULL DEFAULT 0,
    requiere_receta  BOOLEAN NOT NULL DEFAULT FALSE,
    estado           ENUM('disponible', 'agotado', 'suspendido', 'caducado') NOT NULL DEFAULT 'disponible',
    CONSTRAINT fk_productos_categoria FOREIGN KEY (id_categoria) REFERENCES Categorias(id_categoria),
    CONSTRAINT fk_productos_proveedor FOREIGN KEY (id_proveedor) REFERENCES Proveedores(id_proveedor)
) ENGINE=InnoDB;

-- ---------------------------------------------------------------------
-- Personas (fragmentación vertical/horizontal por sucursal de origen)
-- ---------------------------------------------------------------------
CREATE TABLE Clientes (
    id_cliente        INT AUTO_INCREMENT PRIMARY KEY,
    id_sucursal       INT NOT NULL,
    nombre            VARCHAR(50) NOT NULL,
    apellido_paterno  VARCHAR(50),
    apellido_materno  VARCHAR(50),
    telefono          VARCHAR(20),
    correo            VARCHAR(100),
    direccion         VARCHAR(200),
    fecha_registro    DATE NOT NULL DEFAULT (CURRENT_DATE),
    estado            ENUM('activo', 'inactivo') NOT NULL DEFAULT 'activo',
    CONSTRAINT fk_clientes_sucursal FOREIGN KEY (id_sucursal) REFERENCES Sucursales(id_sucursal)
) ENGINE=InnoDB;

CREATE TABLE Empleados (
    id_empleado         INT AUTO_INCREMENT PRIMARY KEY,
    id_sucursal         INT NOT NULL,
    nombre              VARCHAR(50) NOT NULL,
    apellido_paterno    VARCHAR(50),
    apellido_materno    VARCHAR(50),
    puesto              VARCHAR(50),
    telefono            VARCHAR(20),
    correo              VARCHAR(100),
    fecha_contratacion  DATE NOT NULL DEFAULT (CURRENT_DATE),
    estado              ENUM('activo', 'inactivo') NOT NULL DEFAULT 'activo',
    CONSTRAINT fk_empleados_sucursal FOREIGN KEY (id_sucursal) REFERENCES Sucursales(id_sucursal)
) ENGINE=InnoDB;

-- ---------------------------------------------------------------------
-- Inventario (fragmentado horizontalmente por sucursal vía id_sucursal)
-- ---------------------------------------------------------------------
CREATE TABLE Lotes (
    id_lote              INT AUTO_INCREMENT PRIMARY KEY,
    id_producto          INT NOT NULL,
    id_sucursal          INT NOT NULL,
    numero_lote          VARCHAR(50),
    fecha_ingreso         DATE NOT NULL DEFAULT (CURRENT_DATE),
    fecha_caducidad      DATE,
    cantidad_inicial     INT NOT NULL,
    cantidad_disponible  INT NOT NULL,
    estado               ENUM('disponible', 'agotado', 'caducado') NOT NULL DEFAULT 'disponible',
    CONSTRAINT fk_lotes_producto FOREIGN KEY (id_producto) REFERENCES Productos(id_producto),
    CONSTRAINT fk_lotes_sucursal FOREIGN KEY (id_sucursal) REFERENCES Sucursales(id_sucursal)
) ENGINE=InnoDB;

CREATE TABLE Inventario (
    id_movimiento     INT AUTO_INCREMENT PRIMARY KEY,
    id_lote           INT NOT NULL,
    tipo_movimiento   ENUM('entrada', 'salida') NOT NULL,
    cantidad          INT NOT NULL,
    fecha_movimiento  DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    motivo            ENUM('compra', 'venta', 'devolucion', 'daño', 'caducidad', 'ajuste') NOT NULL,
    CONSTRAINT fk_inventario_lote FOREIGN KEY (id_lote) REFERENCES Lotes(id_lote)
) ENGINE=InnoDB;

-- ---------------------------------------------------------------------
-- Ventas (fragmentadas horizontalmente por sucursal vía Tickets.id_sucursal)
-- ---------------------------------------------------------------------
CREATE TABLE Tickets (
    id_ticket         INT AUTO_INCREMENT PRIMARY KEY,
    id_sucursal       INT NOT NULL,
    id_empleado       INT NOT NULL,
    id_cliente        INT,
    fecha_venta       DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    subtotal          DECIMAL(10,2) NOT NULL DEFAULT 0,
    descuento         DECIMAL(10,2) NOT NULL DEFAULT 0,
    total             DECIMAL(10,2) GENERATED ALWAYS AS (subtotal - descuento) STORED,
    modalidad_pago    ENUM('contado', 'diferido') NOT NULL DEFAULT 'contado',
    monto_pagado      DECIMAL(10,2) NOT NULL DEFAULT 0,
    saldo_pendiente   DECIMAL(10,2) GENERATED ALWAYS AS (subtotal - descuento - monto_pagado) STORED,
    fecha_limite_pago DATE,
    estado_pago       ENUM('pagado', 'parcial', 'pendiente', 'vencido') NOT NULL DEFAULT 'pendiente',
    estado_ticket     ENUM('completado', 'cancelado', 'devuelto') NOT NULL DEFAULT 'completado',
    CONSTRAINT fk_tickets_sucursal FOREIGN KEY (id_sucursal) REFERENCES Sucursales(id_sucursal),
    CONSTRAINT fk_tickets_empleado FOREIGN KEY (id_empleado) REFERENCES Empleados(id_empleado),
    CONSTRAINT fk_tickets_cliente FOREIGN KEY (id_cliente) REFERENCES Clientes(id_cliente)
) ENGINE=InnoDB;

CREATE TABLE Ventas (
    id_venta        INT AUTO_INCREMENT PRIMARY KEY,
    id_ticket       INT NOT NULL,
    id_lote         INT NOT NULL,
    cantidad        INT NOT NULL,
    precio_unitario DECIMAL(10,2) NOT NULL,
    subtotal        DECIMAL(10,2) GENERATED ALWAYS AS (cantidad * precio_unitario) STORED,
    CONSTRAINT fk_ventas_ticket FOREIGN KEY (id_ticket) REFERENCES Tickets(id_ticket),
    CONSTRAINT fk_ventas_lote FOREIGN KEY (id_lote) REFERENCES Lotes(id_lote)
) ENGINE=InnoDB;

CREATE TABLE Pagos (
    id_pago            INT AUTO_INCREMENT PRIMARY KEY,
    id_ticket          INT NOT NULL,
    monto              DECIMAL(10,2) NOT NULL,
    fecha_pago         DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    metodo_pago        ENUM('efectivo', 'tarjeta', 'transferencia') NOT NULL,
    numero_referencia  VARCHAR(100),
    estado             ENUM('aplicado', 'pendiente', 'cancelado') NOT NULL DEFAULT 'aplicado',
    CONSTRAINT fk_pagos_ticket FOREIGN KEY (id_ticket) REFERENCES Tickets(id_ticket)
) ENGINE=InnoDB;

CREATE TABLE Devoluciones (
    id_devolucion     INT AUTO_INCREMENT PRIMARY KEY,
    id_sucursal       INT NOT NULL,
    id_ticket         INT NOT NULL,
    id_empleado       INT NOT NULL,
    fecha_devolucion  DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    motivo            VARCHAR(200),
    total_devuelto    DECIMAL(10,2) NOT NULL DEFAULT 0,
    tipo_reembolso    ENUM('efectivo', 'transferencia', 'saldo_a_favor', 'cambio') NOT NULL,
    estado            ENUM('solicitada', 'aprobada', 'rechazada', 'completada') NOT NULL DEFAULT 'solicitada',
    CONSTRAINT fk_devoluciones_sucursal FOREIGN KEY (id_sucursal) REFERENCES Sucursales(id_sucursal),
    CONSTRAINT fk_devoluciones_ticket FOREIGN KEY (id_ticket) REFERENCES Tickets(id_ticket),
    CONSTRAINT fk_devoluciones_empleado FOREIGN KEY (id_empleado) REFERENCES Empleados(id_empleado)
) ENGINE=InnoDB;

CREATE TABLE Detalle_Devolucion (
    id_detalle_devolucion  INT AUTO_INCREMENT PRIMARY KEY,
    id_devolucion          INT NOT NULL,
    id_venta               INT NOT NULL,
    cantidad               INT NOT NULL,
    importe                DECIMAL(10,2) NOT NULL,
    estado_producto        ENUM('sellado', 'abierto', 'dañado', 'caducado') NOT NULL,
    reintegrar_stock       BOOLEAN NOT NULL DEFAULT FALSE,
    CONSTRAINT fk_detdev_devolucion FOREIGN KEY (id_devolucion) REFERENCES Devoluciones(id_devolucion),
    CONSTRAINT fk_detdev_venta FOREIGN KEY (id_venta) REFERENCES Ventas(id_venta)
) ENGINE=InnoDB;

-- ---------------------------------------------------------------------
-- Usuarios (cuentas de acceso a la app) — tabla separada de Empleados:
-- no todo empleado tiene por qué tener acceso al sistema. `rol` controla
-- el alcance en la aplicación: 'admin' ve/administra todas las sucursales
-- y el catálogo maestro; 'empleado' opera solo la sucursal de su
-- Empleado ligado (fragmentación reflejada también en la capa de app).
-- ---------------------------------------------------------------------
CREATE TABLE Usuarios (
    id_usuario       INT AUTO_INCREMENT PRIMARY KEY,
    id_empleado      INT NULL UNIQUE,
    nombre_usuario   VARCHAR(50) NOT NULL UNIQUE,
    contrasena_hash  CHAR(64) NOT NULL,
    sal              CHAR(32) NOT NULL,
    rol              ENUM('admin', 'empleado') NOT NULL DEFAULT 'empleado',
    estado           ENUM('activo', 'inactivo') NOT NULL DEFAULT 'activo',
    fecha_creacion   DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    CONSTRAINT fk_usuarios_empleado FOREIGN KEY (id_empleado) REFERENCES Empleados(id_empleado)
) ENGINE=InnoDB;

SET FOREIGN_KEY_CHECKS = 1;

-- =====================================================================
-- Triggers — automatizaciones descritas en "Funcionamiento general"
-- de Farmacia_actualizado.md
-- =====================================================================

DELIMITER $$

-- Al insertar una venta: descuenta del lote, registra salida en Inventario
-- y resincroniza Tickets.subtotal
CREATE TRIGGER trg_ventas_after_insert
AFTER INSERT ON Ventas
FOR EACH ROW
BEGIN
    UPDATE Lotes
       SET cantidad_disponible = cantidad_disponible - NEW.cantidad,
           estado = IF(cantidad_disponible - NEW.cantidad <= 0, 'agotado', estado)
     WHERE id_lote = NEW.id_lote;

    INSERT INTO Inventario (id_lote, tipo_movimiento, cantidad, motivo)
    VALUES (NEW.id_lote, 'salida', NEW.cantidad, 'venta');

    UPDATE Tickets
       SET subtotal = (SELECT COALESCE(SUM(subtotal), 0) FROM Ventas WHERE id_ticket = NEW.id_ticket)
     WHERE id_ticket = NEW.id_ticket;
END$$

-- Al eliminar/cancelar una línea de venta: revierte el efecto anterior
CREATE TRIGGER trg_ventas_after_delete
AFTER DELETE ON Ventas
FOR EACH ROW
BEGIN
    UPDATE Lotes
       SET cantidad_disponible = cantidad_disponible + OLD.cantidad,
           estado = 'disponible'
     WHERE id_lote = OLD.id_lote;

    INSERT INTO Inventario (id_lote, tipo_movimiento, cantidad, motivo)
    VALUES (OLD.id_lote, 'entrada', OLD.cantidad, 'ajuste');

    UPDATE Tickets
       SET subtotal = (SELECT COALESCE(SUM(subtotal), 0) FROM Ventas WHERE id_ticket = OLD.id_ticket)
     WHERE id_ticket = OLD.id_ticket;
END$$

-- Al registrar un pago/abono: resincroniza Tickets.monto_pagado y estado_pago
CREATE TRIGGER trg_pagos_after_insert
AFTER INSERT ON Pagos
FOR EACH ROW
BEGIN
    UPDATE Tickets
       SET monto_pagado = (SELECT COALESCE(SUM(monto), 0) FROM Pagos WHERE id_ticket = NEW.id_ticket AND estado = 'aplicado')
     WHERE id_ticket = NEW.id_ticket;

    UPDATE Tickets
       SET estado_pago = CASE
                            WHEN saldo_pendiente <= 0 THEN 'pagado'
                            WHEN monto_pagado > 0 THEN 'parcial'
                            ELSE 'pendiente'
                          END
     WHERE id_ticket = NEW.id_ticket;
END$$

-- Al aprobar una devolución con reintegro de stock: regresa unidades al lote
CREATE TRIGGER trg_detdev_after_insert
AFTER INSERT ON Detalle_Devolucion
FOR EACH ROW
BEGIN
    DECLARE v_id_lote INT;

    IF NEW.reintegrar_stock = TRUE THEN
        SELECT id_lote INTO v_id_lote FROM Ventas WHERE id_venta = NEW.id_venta;

        UPDATE Lotes
           SET cantidad_disponible = cantidad_disponible + NEW.cantidad,
               estado = 'disponible'
         WHERE id_lote = v_id_lote;

        INSERT INTO Inventario (id_lote, tipo_movimiento, cantidad, motivo)
        VALUES (v_id_lote, 'entrada', NEW.cantidad, 'devolucion');
    END IF;
END$$

DELIMITER ;
