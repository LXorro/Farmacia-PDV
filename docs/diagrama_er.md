# Diagrama ER — Farmacia (con capa de sucursales)

Basado en `Farmacia_actualizado.md` + la capa de multi-sucursal de
`Plan_Proyecto.md` (sección 2), tal como está implementado en `db/schema.sql`.

```mermaid
erDiagram
    SUCURSALES ||--o{ CLIENTES : "registra"
    SUCURSALES ||--o{ EMPLEADOS : "emplea"
    SUCURSALES ||--o{ LOTES : "almacena"
    SUCURSALES ||--o{ TICKETS : "origina"
    SUCURSALES ||--o{ DEVOLUCIONES : "origina"

    CATEGORIAS ||--o{ PRODUCTOS : "clasifica"
    PROVEEDORES ||--o{ PRODUCTOS : "suministra"
    PRODUCTOS ||--o{ LOTES : "tiene"

    LOTES ||--o{ INVENTARIO : "genera movimientos"
    LOTES ||--o{ VENTAS : "surte"

    EMPLEADOS ||--o{ TICKETS : "atiende"
    CLIENTES ||--o{ TICKETS : "compra"
    TICKETS ||--o{ VENTAS : "detalla"
    TICKETS ||--o{ PAGOS : "recibe"
    TICKETS ||--o{ DEVOLUCIONES : "origina"
    EMPLEADOS ||--o{ DEVOLUCIONES : "registra"
    DEVOLUCIONES ||--o{ DETALLE_DEVOLUCION : "detalla"
    VENTAS ||--o{ DETALLE_DEVOLUCION : "se devuelve en"

    SUCURSALES {
        int id_sucursal PK
        string nombre
        string direccion
        enum rol "central | sucursal"
        enum estado
    }
    CATEGORIAS {
        int id_categoria PK
        string nombre
        string tipo
    }
    PROVEEDORES {
        int id_proveedor PK
        string nombre_empresa
        enum estado
    }
    PRODUCTOS {
        int id_producto PK
        string nombre
        string codigo_barras
        int id_categoria FK
        int id_proveedor FK
        decimal precio_compra
        decimal precio_venta
        bool requiere_receta
    }
    CLIENTES {
        int id_cliente PK
        int id_sucursal FK "fragmentación vertical"
        string nombre
        enum estado
    }
    EMPLEADOS {
        int id_empleado PK
        int id_sucursal FK
        string nombre
        string puesto
    }
    LOTES {
        int id_lote PK
        int id_producto FK
        int id_sucursal FK "fragmentación horizontal"
        date fecha_caducidad
        int cantidad_disponible
    }
    INVENTARIO {
        int id_movimiento PK
        int id_lote FK
        enum tipo_movimiento
        int cantidad
        enum motivo
    }
    TICKETS {
        int id_ticket PK
        int id_sucursal FK "fragmentación horizontal"
        int id_empleado FK
        int id_cliente FK
        decimal subtotal "auto (triggers)"
        decimal total "generada"
        decimal saldo_pendiente "generada"
        enum estado_pago
    }
    VENTAS {
        int id_venta PK
        int id_ticket FK
        int id_lote FK
        int cantidad
        decimal precio_unitario
        decimal subtotal "generada"
    }
    PAGOS {
        int id_pago PK
        int id_ticket FK
        decimal monto
        enum metodo_pago
    }
    DEVOLUCIONES {
        int id_devolucion PK
        int id_sucursal FK
        int id_ticket FK
        int id_empleado FK
        enum estado
    }
    DETALLE_DEVOLUCION {
        int id_detalle_devolucion PK
        int id_devolucion FK
        int id_venta FK
        int cantidad
        bool reintegrar_stock
    }
```

## Leyenda de la capa distribuida

- **Catálogo maestro** (sin `id_sucursal`, replicado centro→sucursales):
  `Categorias`, `Proveedores`, `Productos`.
- **Fragmentación horizontal** (`id_sucursal` en la tabla, cada sucursal
  solo tiene sus propias filas): `Lotes`, `Tickets`, `Devoluciones`
  (y por herencia de FK: `Inventario`, `Ventas`, `Pagos`, `Detalle_Devolucion`).
- **Fragmentación vertical**: `Clientes` — el registro vive en la
  sucursal donde se dio de alta.
