// Punto de venta manejado como lo haría un usuario (campos, combos y botones reales) y verificado en la BD.
#include "testutil.h"

#include "pages/puntoventapage.h"

#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTableWidget>
#include <memory>

class TestVentas : public QObject {
    Q_OBJECT
    T::Modales *m = nullptr;
    std::unique_ptr<PuntoVentaPage> pv;

    template <class W> W *w(const char *nombre) {
        auto *x = pv->findChild<W *>(nombre);
        if (!x) qFatal("No existe el widget %s", nombre);
        return x;
    }
    void abrir(int idSucursal) {
        pv.reset(new PuntoVentaPage);
        pv->resize(1100, 700);
        pv->show();
        auto *c = w<QComboBox>("comboSucursal");
        const int i = c->findData(idSucursal);
        QVERIFY2(i >= 0, "La sucursal no aparece en el combo");
        c->setCurrentIndex(i);
        QCoreApplication::processEvents();
    }
    void agregar(const QString &texto, int cantidad) {
        w<QLineEdit>("lineBuscarProducto")->setText(texto);
        w<QSpinBox>("spinCantidad")->setValue(cantidad);
        QTest::mouseClick(w<QPushButton>("btnAgregar"), Qt::LeftButton);
    }
    void cobrar() { QTest::mouseClick(w<QPushButton>("btnCobrar"), Qt::LeftButton); }
    int filasCarrito() { return pv->findChild<QTableWidget *>("tableCarrito")->rowCount(); }
    int stock(int idLote) { return T::escalar("SELECT cantidad_disponible FROM Lotes WHERE id_lote=?", {idLote}).toInt(); }

    static void restaurar() {
        QSqlQuery q;
        q.exec("SET FOREIGN_KEY_CHECKS=0");
        for (auto t : {"Detalle_Devolucion", "Devoluciones", "Pagos", "Ventas", "Inventario", "Tickets"})
            q.exec(QString("DELETE FROM %1").arg(t));
        q.exec("SET FOREIGN_KEY_CHECKS=1");
        q.exec("DELETE FROM Lotes WHERE numero_lote='EXTRA'");
        q.exec("UPDATE Lotes SET cantidad_disponible=cantidad_inicial, estado='disponible', "
               "fecha_caducidad=DATE_ADD(CURDATE(), INTERVAL 18 MONTH)");
        q.exec("UPDATE Productos SET precio_sugerido=28.00 WHERE id_producto=1");
    }

private slots:
    void initTestCase() {
        QString e;
        QVERIFY2(T::conectarBd(&e), qPrintable(e));
        T::sembrarUsuarios();
        m = new T::Modales;
    }
    void init() {
        restaurar();
        m->limpiar();
        T::iniciarSesionAdmin();
    }
    void cleanup() { pv.reset(); }
    void cleanupTestCase() { delete m; }

    // ---- flujo normal ---------------------------------------------------------------------------------
    void venta_normal_en_efectivo() {
        abrir(2);
        agregar("7501000000011", 2);   // Paracetamol, $28
        QCOMPARE(filasCarrito(), 1);
        cobrar();
        QCOMPARE(T::contar("Tickets"), 1);
        QCOMPARE(stock(2), 48);
        QCOMPARE(T::escalar("SELECT total FROM Tickets").toDouble(), 56.0);
        QCOMPARE(T::escalar("SELECT SUM(monto) FROM Pagos").toDouble(), 56.0);
        QCOMPARE(filasCarrito(), 0);
    }

    // ---- límites de cantidad -------------------------------------------------------------------------
    void cantidad_mayor_al_stock_se_rechaza() {
        abrir(2);
        agregar("7501000000011", 51);
        QCOMPARE(filasCarrito(), 0);
        QVERIFY2(m->avisos.value(0).titulo.contains("insuficiente", Qt::CaseInsensitive), qPrintable(m->todosLosTextos()));
    }
    void cantidad_maxima_del_spin() {
        abrir(2);
        auto *s = w<QSpinBox>("spinCantidad");
        s->setValue(s->maximum());
        qInfo() << "  spinCantidad rango:" << s->minimum() << "…" << s->maximum();
        agregar("7501000000011", s->maximum());
        QCOMPARE(filasCarrito(), 0);
        QCOMPARE(T::contar("Tickets"), 0);
    }
    void cantidad_cero_o_negativa_en_el_spin() {
        abrir(2);
        auto *s = w<QSpinBox>("spinCantidad");
        s->setValue(-5);
        QVERIFY2(s->value() >= 1, qPrintable(QString("El spin permite cantidad %1").arg(s->value())));
    }

    // ---- sobreventa: el mismo producto agregado varias veces ------------------------------------------
    void sobreventa_agregando_el_mismo_producto_varias_veces() {
        abrir(2);                       // LOTE-A2 tiene 50 unidades
        agregar("7501000000011", 40);
        agregar("7501000000011", 40);   // 80 > 50 en total
        const int enCarrito = filasCarrito();
        cobrar();
        const int disponible = stock(2);
        QVERIFY2(disponible >= 0,
                 qPrintable(QString("SOBREVENTA: el lote quedó con %1 unidades (carrito con %2 líneas de 40 sobre un stock de 50)")
                                .arg(disponible).arg(enCarrito)));
    }
    void carrito_usa_otro_lote_cuando_el_primero_ya_esta_apartado() {
        QSqlQuery q;   // segundo lote del mismo producto, caduca después: 30 unidades
        q.exec("INSERT INTO Lotes (id_producto,id_sucursal,numero_lote,fecha_caducidad,cantidad_inicial,cantidad_disponible,costo_unitario) "
               "VALUES (1,2,'EXTRA',DATE_ADD(CURDATE(), INTERVAL 30 MONTH),30,30,5)");
        abrir(2);
        agregar("7501000000011", 40);   // lote A2 (quedan 10 libres)
        agregar("7501000000011", 40);   // no cabe en ningún lote: A2 libre 10, EXTRA 30
        QCOMPARE(filasCarrito(), 1);
        agregar("7501000000011", 30);   // cabe en EXTRA
        QCOMPARE(filasCarrito(), 2);
        agregar("7501000000011", 11);   // A2 libre 10, EXTRA libre 0
        QCOMPARE(filasCarrito(), 2);
        cobrar();
        QCOMPARE(stock(2), 10);
        QCOMPARE(T::escalar("SELECT cantidad_disponible FROM Lotes WHERE numero_lote='EXTRA'").toInt(), 0);
    }
    void sobreventa_tres_lineas_justo_en_el_limite() {
        abrir(2);
        agregar("7501000000011", 25);
        agregar("7501000000011", 25);   // 50 exacto: debe permitirse
        cobrar();
        QCOMPARE(stock(2), 0);
        QCOMPARE(T::escalar("SELECT estado FROM Lotes WHERE id_lote=2").toString(), QString("agotado"));
    }

    // ---- productos no vendibles -------------------------------------------------------------------------
    void producto_caducado_no_se_vende() {
        QSqlQuery q;
        q.exec("UPDATE Lotes SET fecha_caducidad=DATE_SUB(CURDATE(), INTERVAL 1 DAY) WHERE id_lote=2");
        abrir(2);
        agregar("7501000000011", 1);
        QCOMPARE(filasCarrito(), 0);
    }
    void producto_inexistente() {
        abrir(2);
        agregar("0000000000000", 1);
        QCOMPARE(filasCarrito(), 0);
        QVERIFY(m->huboAviso());
    }
    void producto_descontinuado_no_se_vende() {
        QSqlQuery q;
        q.exec("UPDATE Productos SET estado='descontinuado' WHERE id_producto=1");
        abrir(2);
        agregar("7501000000011", 1);
        QCOMPARE(filasCarrito(), 0);
        q.exec("UPDATE Productos SET estado='activo' WHERE id_producto=1");
    }
    // DEFENSA EN PROFUNDIDAD: desde la interfaz no se llega a precio $0 (ProductoDialog exige >= 0.01 y un 0 en
    // el precio por sucursal se guarda como «usar el sugerido»), pero el POS no lo revalida y la BD no lo impide.
    void producto_con_precio_cero() {
        QSqlQuery q;
        q.exec("UPDATE Productos SET precio_sugerido=0 WHERE id_producto=1");
        abrir(2);
        agregar("7501000000011", 1);
        cobrar();
        QVERIFY2(T::contar("Tickets") == 0, "Se registró una venta de un producto con precio $0.00");
    }

    // ---- búsqueda hostil -------------------------------------------------------------------------------
    void busqueda_hostil_data() {
        QTest::addColumn<QString>("texto");
        QTest::newRow("sql 1") << "' OR '1'='1";
        QTest::newRow("sql 2") << "'; DROP TABLE Productos; --";
        QTest::newRow("sql 3") << "\" OR \"\"=\"";
        QTest::newRow("comodin %") << "%";
        QTest::newRow("comodin _") << "_";
        QTest::newRow("backslash") << "\\";
        QTest::newRow("html") << "<script>alert(1)</script>";
        QTest::newRow("NUL") << QString("Para") + QChar(0) + "cetamol";
        QTest::newRow("unicode raro") << QString("‮​💊");
        QTest::newRow("largo") << QString("a").repeated(100000);
    }
    void busqueda_hostil() {
        QFETCH(QString, texto);
        abrir(2);
        agregar(texto, 1);
        QCoreApplication::processEvents();
        QVERIFY2(T::contar("Productos") == 4, "La búsqueda alteró la tabla Productos");
        QVERIFY2(T::contar("Usuarios") >= 2, "La búsqueda alteró la tabla Usuarios");
    }
    void comodin_en_busqueda_no_debe_coincidir_con_todo() {
        abrir(2);
        agregar("%", 1);
        QVERIFY2(filasCarrito() == 0, "Buscar '%' agregó un producto cualquiera al carrito (el comodín LIKE no se escapa)");
    }

    // ---- validaciones de cobro ---------------------------------------------------------------------------
    void cobrar_con_carrito_vacio() {
        abrir(2);
        cobrar();
        QCOMPARE(T::contar("Tickets"), 0);
        QVERIFY(m->huboAviso());
    }
    void diferido_sin_cliente_se_rechaza() {
        abrir(2);
        agregar("7501000000011", 1);
        w<QComboBox>("comboModalidadPago")->setCurrentText("diferido");
        auto *cc = w<QComboBox>("comboCliente");
        cc->setCurrentIndex(0);   // mostrador
        cobrar();
        QCOMPARE(T::contar("Tickets"), 0);
        QVERIFY2(stock(2) == 50, "El stock cambió aunque la venta se rechazó");
    }
    void mixto_con_diferido_se_rechaza() {
        abrir(2);
        agregar("7501000000011", 1);
        w<QComboBox>("comboModalidadPago")->setCurrentText("diferido");
        w<QComboBox>("comboMetodoPago")->setCurrentText("mixto");
        cobrar();
        QCOMPARE(T::contar("Tickets"), 0);
    }
    void quitar_linea_sin_seleccion_y_cancelar_venta() {
        abrir(2);
        QTest::mouseClick(w<QPushButton>("btnQuitarLinea"), Qt::LeftButton);
        QTest::mouseClick(w<QPushButton>("btnCancelarVenta"), Qt::LeftButton);
        agregar("7501000000011", 1);
        QTest::mouseClick(w<QPushButton>("btnCancelarVenta"), Qt::LeftButton);   // confirma Sí
        QCOMPARE(filasCarrito(), 0);
    }
    void doble_cobro_seguido() {
        abrir(2);
        agregar("7501000000011", 1);
        cobrar();
        cobrar();   // el carrito ya está vacío
        QCOMPARE(T::contar("Tickets"), 1);
        QCOMPARE(stock(2), 49);
    }
};

FARMACIA_TEST_MAIN(TestVentas)
#include "test_ventas.moc"
