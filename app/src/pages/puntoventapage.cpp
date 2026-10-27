#include "puntoventapage.h"
#include "ui_puntoventapage.h"
#include "../database/sesionactual.h"
#include "../util/iconos.h"
#include "../util/tablas.h"

#include <QHeaderView>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

PuntoVentaPage::PuntoVentaPage(QWidget *parent) : QWidget(parent), ui(new Ui::PuntoVentaPage) {
    ui->setupUi(this);
    {
        // Producto se expande; el resto se ajusta al contenido (que no se corten los encabezados).
        auto *h = ui->tableCarrito->horizontalHeader();
        h->setStretchLastSection(false);
        h->setSectionResizeMode(QHeaderView::ResizeToContents);
        h->setSectionResizeMode(0, QHeaderView::Stretch);
    }
    ui->tableCarrito->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableCarrito->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableCarrito->setAlternatingRowColors(true);
    ui->tableCarrito->verticalHeader()->setVisible(false);
    Tablas::aplicar(ui->tableCarrito, "El carrito está vacío.");

    connect(ui->comboSucursal, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &PuntoVentaPage::alCambiarSucursal);
    connect(ui->btnAgregar, &QPushButton::clicked, this, &PuntoVentaPage::agregarAlCarrito);
    connect(ui->btnQuitarLinea, &QPushButton::clicked, this, &PuntoVentaPage::quitarLineaSeleccionada);
    connect(ui->btnCancelarVenta, &QPushButton::clicked, this, &PuntoVentaPage::cancelarVenta);
    connect(ui->btnCobrar, &QPushButton::clicked, this, &PuntoVentaPage::cobrar);

    ui->btnAgregar->setIcon(Iconos::agregarAlCarrito());
    ui->btnQuitarLinea->setIcon(Iconos::quitarLinea());
    ui->btnCancelarVenta->setIcon(Iconos::cancelar());
    ui->btnCobrar->setIcon(Iconos::cobrar());

    cargarSucursales();
}

PuntoVentaPage::~PuntoVentaPage() {
    delete ui;
}

void PuntoVentaPage::refrescar() {
    cargarSucursales();
}

void PuntoVentaPage::cargarSucursales() {
    int idPrevio = ui->comboSucursal->currentData().toInt();
    bool esAdmin = SesionActual::instance().esAdmin();

    ui->comboSucursal->blockSignals(true);
    ui->comboSucursal->clear();
    QSqlQuery query;
    if (esAdmin) {
        query.exec("SELECT id_sucursal, nombre FROM Sucursales WHERE estado = 'activa' ORDER BY nombre");
    } else {
        query.prepare("SELECT id_sucursal, nombre FROM Sucursales WHERE estado = 'activa' AND id_sucursal = :suc");
        query.bindValue(":suc", SesionActual::instance().idSucursal());
        query.exec();
    }
    while (query.next())
        ui->comboSucursal->addItem(query.value(1).toString(), query.value(0));
    ui->comboSucursal->blockSignals(false);
    ui->comboSucursal->setEnabled(esAdmin);

    int idx = ui->comboSucursal->findData(idPrevio);
    ui->comboSucursal->setCurrentIndex(idx >= 0 ? idx : 0);
    alCambiarSucursal();
}

void PuntoVentaPage::alCambiarSucursal() {
    int idSucursal = ui->comboSucursal->currentData().toInt();
    cargarEmpleados(idSucursal);
    cargarClientes(idSucursal);
    limpiarCarrito();
}

void PuntoVentaPage::cargarEmpleados(int idSucursal) {
    ui->comboEmpleado->clear();
    QSqlQuery query;
    query.prepare("SELECT id_empleado, nombre, apellido_paterno FROM Empleados "
                  "WHERE id_sucursal = :suc AND estado = 'activo' ORDER BY nombre");
    query.bindValue(":suc", idSucursal);
    query.exec();
    while (query.next())
        ui->comboEmpleado->addItem(query.value(1).toString() + " " + query.value(2).toString(), query.value(0));

    // Comodidad: preseleccionar al empleado logueado, pero cualquiera de la
    // sucursal puede quedar como el que atendió la venta.
    int idx = ui->comboEmpleado->findData(SesionActual::instance().idEmpleado());
    if (idx >= 0)
        ui->comboEmpleado->setCurrentIndex(idx);
}

void PuntoVentaPage::cargarClientes(int idSucursal) {
    ui->comboCliente->clear();
    ui->comboCliente->addItem("Venta de mostrador (sin registrar)", -1);
    QSqlQuery query;
    query.prepare("SELECT id_cliente, nombre, apellido_paterno FROM Clientes "
                  "WHERE id_sucursal = :suc AND estado = 'activo' ORDER BY nombre");
    query.bindValue(":suc", idSucursal);
    query.exec();
    while (query.next())
        ui->comboCliente->addItem(query.value(1).toString() + " " + query.value(2).toString(), query.value(0));
}

void PuntoVentaPage::agregarAlCarrito() {
    QString texto = ui->lineBuscarProducto->text().trimmed();
    if (texto.isEmpty())
        return;

    int cantidad = ui->spinCantidad->value();
    int idSucursal = ui->comboSucursal->currentData().toInt();

    QSqlQuery buscar;
    buscar.prepare("SELECT id_producto, nombre, precio_venta FROM Productos "
                   "WHERE estado = 'disponible' AND (codigo_barras = :texto OR nombre LIKE :like) LIMIT 1");
    buscar.bindValue(":texto", texto);
    buscar.bindValue(":like", "%" + texto + "%");
    if (!buscar.exec() || !buscar.next()) {
        QMessageBox::warning(this, "Producto no encontrado", "No se encontró un producto disponible con ese código o nombre.");
        return;
    }

    int idProducto = buscar.value(0).toInt();
    QString nombreProducto = buscar.value(1).toString();
    double precioVenta = buscar.value(2).toDouble();

    QSqlQuery lote;
    lote.prepare("SELECT id_lote, numero_lote, cantidad_disponible FROM Lotes "
                 "WHERE id_producto = :pid AND id_sucursal = :suc AND cantidad_disponible > 0 "
                 "ORDER BY fecha_caducidad ASC LIMIT 1");
    lote.bindValue(":pid", idProducto);
    lote.bindValue(":suc", idSucursal);
    if (!lote.exec() || !lote.next()) {
        QMessageBox::warning(this, "Sin existencias", nombreProducto + " no tiene lotes disponibles en esta sucursal.");
        return;
    }

    int idLote = lote.value(0).toInt();
    QString numeroLote = lote.value(1).toString();
    int disponible = lote.value(2).toInt();
    if (disponible < cantidad) {
        QMessageBox::warning(this, "Stock insuficiente",
                              QString("Solo hay %1 unidades disponibles en el lote %2.").arg(disponible).arg(numeroLote));
        return;
    }

    LineaCarrito linea{idProducto, idLote, nombreProducto, numeroLote, cantidad, precioVenta};
    m_carrito.append(linea);

    ui->lineBuscarProducto->clear();
    ui->spinCantidad->setValue(1);
    ui->lineBuscarProducto->setFocus();

    refrescarTabla();
    actualizarTotal();
}

void PuntoVentaPage::quitarLineaSeleccionada() {
    auto seleccion = ui->tableCarrito->selectionModel()->selectedRows();
    if (seleccion.isEmpty())
        return;

    m_carrito.remove(seleccion.first().row());
    refrescarTabla();
    actualizarTotal();
}

void PuntoVentaPage::cancelarVenta() {
    if (m_carrito.isEmpty())
        return;
    if (QMessageBox::question(this, "Cancelar venta", "¿Vaciar el carrito actual?") == QMessageBox::Yes)
        limpiarCarrito();
}

void PuntoVentaPage::limpiarCarrito() {
    m_carrito.clear();
    refrescarTabla();
    actualizarTotal();
}

void PuntoVentaPage::refrescarTabla() {
    ui->tableCarrito->setRowCount(m_carrito.size());
    for (int fila = 0; fila < m_carrito.size(); ++fila) {
        const LineaCarrito &linea = m_carrito.at(fila);
        double subtotal = linea.cantidad * linea.precioUnitario;

        ui->tableCarrito->setItem(fila, 0, new QTableWidgetItem(linea.nombreProducto));
        ui->tableCarrito->setItem(fila, 1, new QTableWidgetItem(linea.numeroLote));
        ui->tableCarrito->setItem(fila, 2, new QTableWidgetItem(QString::number(linea.cantidad)));
        ui->tableCarrito->setItem(fila, 3, new QTableWidgetItem(QString::number(linea.precioUnitario, 'f', 2)));
        ui->tableCarrito->setItem(fila, 4, new QTableWidgetItem(QString::number(subtotal, 'f', 2)));
    }
}

void PuntoVentaPage::actualizarTotal() {
    double total = 0;
    for (const LineaCarrito &linea : m_carrito)
        total += linea.cantidad * linea.precioUnitario;
    ui->labelTotal->setText(QString("$%1").arg(total, 0, 'f', 2));
}

void PuntoVentaPage::cobrar() {
    if (m_carrito.isEmpty()) {
        QMessageBox::information(this, "Cobrar", "El carrito está vacío.");
        return;
    }

    int idSucursal = ui->comboSucursal->currentData().toInt();
    int idEmpleado = ui->comboEmpleado->currentData().toInt();
    int idCliente = ui->comboCliente->currentData().toInt();
    QString modalidad = ui->comboModalidadPago->currentText();
    QString metodo = ui->comboMetodoPago->currentText();

    if (modalidad == "diferido" && idCliente < 0) {
        QMessageBox::warning(this, "Validación", "El pago diferido requiere un cliente registrado (no de mostrador).");
        return;
    }

    QSqlDatabase db = QSqlDatabase::database();
    if (!db.transaction()) {
        QMessageBox::critical(this, "Error", "No se pudo iniciar la transacción:\n" + db.lastError().text());
        return;
    }

    // Verificación de stock justo antes de comprometer la venta: mitiga la
    // condición de carrera de dos ventas simultáneas sobre el mismo lote.
    // El caso equivalente entre sucursales (stock centralizado) se resuelve
    // en el plan con una transacción distribuida (XA) — ver Plan_Proyecto.md.
    for (const LineaCarrito &linea : m_carrito) {
        QSqlQuery verificar;
        verificar.prepare("SELECT cantidad_disponible FROM Lotes WHERE id_lote = :id FOR UPDATE");
        verificar.bindValue(":id", linea.idLote);
        if (!verificar.exec() || !verificar.next() || verificar.value(0).toInt() < linea.cantidad) {
            db.rollback();
            QMessageBox::warning(this, "Stock insuficiente",
                                  linea.nombreProducto + " ya no tiene existencias suficientes en el lote " + linea.numeroLote + ".");
            return;
        }
    }

    QSqlQuery insertarTicket;
    insertarTicket.prepare("INSERT INTO Tickets (id_sucursal, id_empleado, id_cliente, modalidad_pago) "
                            "VALUES (:sucursal, :empleado, :cliente, :modalidad)");
    insertarTicket.bindValue(":sucursal", idSucursal);
    insertarTicket.bindValue(":empleado", idEmpleado);
    insertarTicket.bindValue(":cliente", idCliente >= 0 ? QVariant(idCliente) : QVariant(QMetaType(QMetaType::Int)));
    insertarTicket.bindValue(":modalidad", modalidad);
    if (!insertarTicket.exec()) {
        db.rollback();
        QMessageBox::critical(this, "Error", "No se pudo crear el ticket:\n" + insertarTicket.lastError().text());
        return;
    }
    int idTicket = insertarTicket.lastInsertId().toInt();

    for (const LineaCarrito &linea : m_carrito) {
        QSqlQuery insertarVenta;
        insertarVenta.prepare("INSERT INTO Ventas (id_ticket, id_lote, cantidad, precio_unitario) "
                               "VALUES (:ticket, :lote, :cantidad, :precio)");
        insertarVenta.bindValue(":ticket", idTicket);
        insertarVenta.bindValue(":lote", linea.idLote);
        insertarVenta.bindValue(":cantidad", linea.cantidad);
        insertarVenta.bindValue(":precio", linea.precioUnitario);
        if (!insertarVenta.exec()) {
            db.rollback();
            QMessageBox::critical(this, "Error", "No se pudo registrar la venta:\n" + insertarVenta.lastError().text());
            return;
        }
    }

    if (modalidad == "contado") {
        QSqlQuery totalQuery;
        totalQuery.prepare("SELECT total FROM Tickets WHERE id_ticket = :id");
        totalQuery.bindValue(":id", idTicket);
        totalQuery.exec();
        totalQuery.next();
        double total = totalQuery.value(0).toDouble();

        QSqlQuery insertarPago;
        insertarPago.prepare("INSERT INTO Pagos (id_ticket, monto, metodo_pago) VALUES (:ticket, :monto, :metodo)");
        insertarPago.bindValue(":ticket", idTicket);
        insertarPago.bindValue(":monto", total);
        insertarPago.bindValue(":metodo", metodo);
        if (!insertarPago.exec()) {
            db.rollback();
            QMessageBox::critical(this, "Error", "No se pudo registrar el pago:\n" + insertarPago.lastError().text());
            return;
        }
    }

    if (!db.commit()) {
        db.rollback();
        QMessageBox::critical(this, "Error", "No se pudo confirmar la venta:\n" + db.lastError().text());
        return;
    }

    QMessageBox::information(this, "Venta registrada", QString("Ticket #%1 registrado correctamente.").arg(idTicket));
    limpiarCarrito();
}
