// Integridad a nivel de base de datos: ¿acepta MariaDB datos que no deberían existir?
// Cada caso intenta un INSERT/UPDATE inválido; el test PASA si la base lo rechaza.
// Si falla, la única defensa de ese dato es la interfaz (cualquier otro camino de escritura lo guardaría).
#include "testutil.h"

class TestBd : public QObject {
    Q_OBJECT

    // Devuelve true si la base ACEPTÓ la sentencia.
    static bool acepta(const QString &sql) {
        QSqlQuery q;
        const bool ok = q.exec(sql);
        return ok;
    }
    static void rechaza(const QString &descripcion, const QString &sql) {
        QSqlQuery q;
        q.exec("START TRANSACTION");
        const bool ok = q.exec(sql);
        q.exec("ROLLBACK");
        QVERIFY2(!ok, qPrintable("La BD ACEPTÓ: " + descripcion));
    }

private slots:
    void initTestCase() {
        QString e;
        QVERIFY2(T::conectarBd(&e), qPrintable(e));
        T::sembrarUsuarios();
    }

    // ---- cantidades y precios ---------------------------------------------------------------------------
    void ventas_cantidad_cero() {
        QSqlQuery q;
        q.exec("START TRANSACTION");
        q.exec("INSERT INTO Tickets(id_sucursal,id_empleado) VALUES (2,2)");
        const int t = q.lastInsertId().toInt();
        const bool ok = q.exec(QString("INSERT INTO Ventas(id_ticket,id_lote,cantidad,precio_unitario) VALUES (%1,2,0,10)").arg(t));
        q.exec("ROLLBACK");
        QVERIFY2(!ok, "La BD aceptó una línea de venta con cantidad 0");
    }
    void ventas_cantidad_negativa_inventa_stock() {
        QSqlQuery q;
        q.exec("START TRANSACTION");
        q.exec("INSERT INTO Tickets(id_sucursal,id_empleado) VALUES (2,2)");
        const int t = q.lastInsertId().toInt();
        const bool ok = q.exec(QString("INSERT INTO Ventas(id_ticket,id_lote,cantidad,precio_unitario) VALUES (%1,2,-1000,10)").arg(t));
        const int stock = T::escalar("SELECT cantidad_disponible FROM Lotes WHERE id_lote=2").toInt();
        q.exec("ROLLBACK");
        QVERIFY2(!ok, qPrintable(QString("La BD aceptó una venta de -1000 unidades; el stock del lote subió a %1").arg(stock)));
    }
    void ventas_precio_negativo() {
        QSqlQuery q;
        q.exec("START TRANSACTION");
        q.exec("INSERT INTO Tickets(id_sucursal,id_empleado) VALUES (2,2)");
        const int t = q.lastInsertId().toInt();
        const bool ok = q.exec(QString("INSERT INTO Ventas(id_ticket,id_lote,cantidad,precio_unitario) VALUES (%1,2,1,-50)").arg(t));
        q.exec("ROLLBACK");
        QVERIFY2(!ok, "La BD aceptó una venta con precio negativo");
    }
    void sobreventa_en_la_base() {
        QSqlQuery q;
        q.exec("START TRANSACTION");
        q.exec("INSERT INTO Tickets(id_sucursal,id_empleado) VALUES (2,2)");
        const int t = q.lastInsertId().toInt();
        const bool ok = q.exec(QString("INSERT INTO Ventas(id_ticket,id_lote,cantidad,precio_unitario) VALUES (%1,2,999999,10)").arg(t));
        const int stock = T::escalar("SELECT cantidad_disponible FROM Lotes WHERE id_lote=2").toInt();
        q.exec("ROLLBACK");
        QVERIFY2(!ok, qPrintable(QString("La BD permite vender más que el stock: el lote quedó en %1").arg(stock)));
    }
    void lotes_stock_negativo() {
        rechaza("Lotes.cantidad_disponible negativa", "UPDATE Lotes SET cantidad_disponible=-5 WHERE id_lote=1");
    }
    void lotes_disponible_mayor_que_inicial() {
        rechaza("Lotes.cantidad_disponible > cantidad_inicial", "UPDATE Lotes SET cantidad_disponible=cantidad_inicial+1000 WHERE id_lote=1");
    }
    void lotes_inicial_negativa() {
        rechaza("Lotes.cantidad_inicial negativa", "INSERT INTO Lotes(id_producto,id_sucursal,numero_lote,cantidad_inicial,cantidad_disponible) VALUES (1,1,'X',-5,-5)");
    }
    void lotes_caduca_antes_de_ingresar() {
        rechaza("Lote con fecha_caducidad anterior a fecha_ingreso",
                "INSERT INTO Lotes(id_producto,id_sucursal,numero_lote,fecha_ingreso,fecha_caducidad,cantidad_inicial,cantidad_disponible) "
                "VALUES (1,1,'X','2026-06-01','2020-01-01',5,5)");
    }
    void lotes_costo_negativo() {
        rechaza("Lotes.costo_unitario negativo", "UPDATE Lotes SET costo_unitario=-10 WHERE id_lote=1");
    }
    void productos_precio_negativo() {
        rechaza("Productos.precio_sugerido negativo", "UPDATE Productos SET precio_sugerido=-1 WHERE id_producto=1");
    }
    void productos_precio_sucursal_negativo() {
        rechaza("Productos_Sucursal.precio_venta negativo", "INSERT INTO Productos_Sucursal(id_producto,id_sucursal,precio_venta) VALUES (1,2,-5)");
    }
    void productos_codigo_duplicado() {
        rechaza("Productos con código de barras duplicado",
                "INSERT INTO Productos(nombre,codigo_barras,precio_sugerido) VALUES ('Otro','7501000000011',1)");
    }
    void pagos_monto_negativo() {
        QSqlQuery q;
        q.exec("START TRANSACTION");
        q.exec("INSERT INTO Tickets(id_sucursal,id_empleado) VALUES (2,2)");
        const int t = q.lastInsertId().toInt();
        const bool ok = q.exec(QString("INSERT INTO Pagos(id_ticket,monto,metodo_pago) VALUES (%1,-100,'efectivo')").arg(t));
        q.exec("ROLLBACK");
        QVERIFY2(!ok, "La BD aceptó un pago con monto negativo");
    }
    void pagos_sobrepago() {
        QSqlQuery q;
        q.exec("START TRANSACTION");
        q.exec("INSERT INTO Tickets(id_sucursal,id_empleado) VALUES (2,2)");
        const int t = q.lastInsertId().toInt();
        q.exec(QString("INSERT INTO Ventas(id_ticket,id_lote,cantidad,precio_unitario) VALUES (%1,2,1,10)").arg(t));
        const bool ok = q.exec(QString("INSERT INTO Pagos(id_ticket,monto,metodo_pago) VALUES (%1,99999,'efectivo')").arg(t));
        q.exec("ROLLBACK");
        QVERIFY2(!ok, "La BD aceptó un pago mayor al total del ticket (ticket de $10 pagado con $99,999)");
    }
    void tickets_descuento_mayor_al_subtotal() {
        QSqlQuery q;
        q.exec("START TRANSACTION");
        const bool ok = q.exec("INSERT INTO Tickets(id_sucursal,id_empleado,subtotal,descuento) VALUES (2,2,10,500)");
        q.exec("ROLLBACK");
        QVERIFY2(!ok, "La BD aceptó un ticket con descuento mayor al subtotal (total negativo)");
    }
    void tickets_empleado_de_otra_sucursal() {
        rechaza("Ticket de la sucursal 2 vendido por un empleado de la sucursal 1",
                "INSERT INTO Tickets(id_sucursal,id_empleado) VALUES (2,1)");
    }

    // ---- devoluciones --------------------------------------------------------------------------------------
    void devolucion_mayor_a_lo_vendido() {
        QSqlQuery q;
        q.exec("START TRANSACTION");
        q.exec("INSERT INTO Tickets(id_sucursal,id_empleado) VALUES (2,2)");
        const int t = q.lastInsertId().toInt();
        q.exec(QString("INSERT INTO Ventas(id_ticket,id_lote,cantidad,precio_unitario) VALUES (%1,2,1,10)").arg(t));
        const int v = q.lastInsertId().toInt();
        q.exec(QString("INSERT INTO Devoluciones(id_sucursal,id_ticket,id_empleado,tipo_reembolso) VALUES (2,%1,2,'efectivo')").arg(t));
        const int d = q.lastInsertId().toInt();
        const bool ok = q.exec(QString("INSERT INTO Detalle_Devolucion(id_devolucion,id_venta,cantidad,importe,estado_producto,reintegrar_stock) "
                                       "VALUES (%1,%2,500,5000,'sellado',TRUE)").arg(d).arg(v));
        const int stock = T::escalar("SELECT cantidad_disponible FROM Lotes WHERE id_lote=2").toInt();
        q.exec("ROLLBACK");
        QVERIFY2(!ok, qPrintable(QString("La BD aceptó devolver 500 unidades de una venta de 1; el stock subió a %1").arg(stock)));
    }
    void devolucion_importe_negativo() {
        QSqlQuery q;
        q.exec("START TRANSACTION");
        q.exec("INSERT INTO Tickets(id_sucursal,id_empleado) VALUES (2,2)");
        const int t = q.lastInsertId().toInt();
        q.exec(QString("INSERT INTO Ventas(id_ticket,id_lote,cantidad,precio_unitario) VALUES (%1,2,1,10)").arg(t));
        const int v = q.lastInsertId().toInt();
        q.exec(QString("INSERT INTO Devoluciones(id_sucursal,id_ticket,id_empleado,tipo_reembolso) VALUES (2,%1,2,'efectivo')").arg(t));
        const int d = q.lastInsertId().toInt();
        const bool ok = q.exec(QString("INSERT INTO Detalle_Devolucion(id_devolucion,id_venta,cantidad,importe,estado_producto) "
                                       "VALUES (%1,%2,-1,-10,'sellado')").arg(d).arg(v));
        q.exec("ROLLBACK");
        QVERIFY2(!ok, "La BD aceptó una devolución con cantidad e importe negativos");
    }

    // ---- cuentas ------------------------------------------------------------------------------------------
    void usuario_duplicado() {
        rechaza("Usuario duplicado",
                "INSERT INTO Usuarios(nombre_usuario,contrasena_hash,sal,rol) VALUES ('admin','x','y','admin')");
    }
    void usuario_duplicado_con_otra_capitalizacion() {
        rechaza("Usuario 'ADMIN' junto a 'admin' (suplantación por mayúsculas)",
                "INSERT INTO Usuarios(nombre_usuario,contrasena_hash,sal,rol) VALUES ('ADMIN','x','y','empleado')");
    }
    void usuario_rol_invalido() {
        rechaza("Rol inexistente", "INSERT INTO Usuarios(nombre_usuario,contrasena_hash,sal,rol) VALUES ('zz9','x','y','superadmin')");
    }
    void usuario_hash_vacio() {
        rechaza("Usuario con hash de contraseña vacío",
                "INSERT INTO Usuarios(nombre_usuario,contrasena_hash,sal,rol) VALUES ('sinclave','','','empleado')");
    }
    void dos_usuarios_para_el_mismo_empleado() {
        rechaza("Segunda cuenta para el empleado 2",
                "INSERT INTO Usuarios(id_empleado,nombre_usuario,contrasena_hash,sal,rol) VALUES (2,'luis2','x','y','empleado')");
    }

    // ---- tamaños ------------------------------------------------------------------------------------------
    void texto_mas_largo_que_la_columna() {
        rechaza("Producto con nombre de 500 caracteres",
                QString("INSERT INTO Productos(nombre,precio_sugerido) VALUES ('%1',1)").arg(QString("a").repeated(500)));
    }
    void precio_fuera_de_rango_decimal() {
        rechaza("Precio 99999999999", "UPDATE Productos SET precio_sugerido=99999999999 WHERE id_producto=1");
    }

    // ---- integridad referencial -----------------------------------------------------------------------------
    void borrar_producto_con_ventas() {
        QSqlQuery q;
        q.exec("START TRANSACTION");
        q.exec("INSERT INTO Tickets(id_sucursal,id_empleado) VALUES (2,2)");
        const int t = q.lastInsertId().toInt();
        q.exec(QString("INSERT INTO Ventas(id_ticket,id_lote,cantidad,precio_unitario) VALUES (%1,2,1,10)").arg(t));
        const bool ok = q.exec("DELETE FROM Lotes WHERE id_lote=2");
        q.exec("ROLLBACK");
        QVERIFY2(!ok, "La BD permitió borrar un lote que ya tiene ventas");
    }
    void venta_a_lote_inexistente() {
        rechaza("Venta sobre un lote inexistente",
                "INSERT INTO Ventas(id_ticket,id_lote,cantidad,precio_unitario) VALUES (1,999999,1,1)");
    }
};

FARMACIA_TEST_MAIN(TestBd)
#include "test_bd.moc"
