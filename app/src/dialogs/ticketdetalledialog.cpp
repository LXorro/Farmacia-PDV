#include "ticketdetalledialog.h"
#include "ui_ticketdetalledialog.h"
#include "devoluciondialog.h"
#include "../util/iconos.h"
#include "../util/tablas.h"

#include <QDateTime>
#include <QHeaderView>
#include <QInputDialog>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

TicketDetalleDialog::TicketDetalleDialog(QWidget *parent) : QDialog(parent), ui(new Ui::TicketDetalleDialog) {
    ui->setupUi(this);
    {
        // Producto se expande; el resto se ajusta al contenido (que no se corten los encabezados).
        auto *h = ui->tableVentas->horizontalHeader();
        h->setStretchLastSection(false);
        h->setSectionResizeMode(QHeaderView::ResizeToContents);
        h->setSectionResizeMode(0, QHeaderView::Stretch);
    }
    ui->tableVentas->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableVentas->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableVentas->setAlternatingRowColors(true);
    ui->tableVentas->verticalHeader()->setVisible(false);
    {
        // Producto se expande; el resto se ajusta al contenido (que no se corten los encabezados).
        auto *h = ui->tablePagos->horizontalHeader();
        h->setStretchLastSection(false);
        h->setSectionResizeMode(QHeaderView::ResizeToContents);
        h->setSectionResizeMode(0, QHeaderView::Stretch);
    }
    ui->tablePagos->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tablePagos->setAlternatingRowColors(true);
    ui->tablePagos->verticalHeader()->setVisible(false);
    Tablas::aplicar(ui->tableVentas, "Este ticket no tiene líneas.");
    Tablas::aplicar(ui->tablePagos, "Sin pagos registrados.");

    connect(ui->btnRegistrarAbono, &QPushButton::clicked, this, &TicketDetalleDialog::registrarAbono);
    connect(ui->btnRegistrarDevolucion, &QPushButton::clicked, this, &TicketDetalleDialog::registrarDevolucion);

    ui->btnRegistrarAbono->setIcon(Iconos::nuevo());
    ui->btnRegistrarDevolucion->setIcon(Iconos::cancelar());
    ui->btnCerrar->setIcon(Iconos::cerrar());
}

TicketDetalleDialog::~TicketDetalleDialog() {
    delete ui;
}

void TicketDetalleDialog::setTicketId(int id) {
    m_idTicket = id;
    cargarDatos();
}

void TicketDetalleDialog::cargarDatos() {
    QSqlQuery encabezado;
    encabezado.prepare(
        "SELECT t.id_sucursal, s.nombre, t.fecha_venta, t.total, t.modalidad_pago, t.estado_pago, t.estado_ticket "
        "FROM Tickets t JOIN Sucursales s ON s.id_sucursal = t.id_sucursal WHERE t.id_ticket = :id");
    encabezado.bindValue(":id", m_idTicket);
    if (encabezado.exec() && encabezado.next()) {
        m_idSucursal = encabezado.value(0).toInt();
        ui->labelEncabezado->setText(
            QString("Ticket #%1 — %2 — %3 — Total: $%4 — %5 (%6)")
                .arg(m_idTicket)
                .arg(encabezado.value(1).toString())
                .arg(encabezado.value(2).toDateTime().toString("dd/MM/yyyy HH:mm"))
                .arg(encabezado.value(3).toDouble(), 0, 'f', 2)
                .arg(encabezado.value(5).toString())
                .arg(encabezado.value(4).toString()));
    }

    QSqlQuery ventas;
    ventas.prepare(
        "SELECT v.id_venta, p.nombre, v.cantidad, v.precio_unitario, v.subtotal, "
        "COALESCE((SELECT SUM(dd.cantidad) FROM Detalle_Devolucion dd WHERE dd.id_venta = v.id_venta), 0) AS devuelto "
        "FROM Ventas v JOIN Lotes l ON l.id_lote = v.id_lote JOIN Productos p ON p.id_producto = l.id_producto "
        "WHERE v.id_ticket = :id");
    ventas.bindValue(":id", m_idTicket);
    ventas.exec();

    ui->tableVentas->setRowCount(0);
    int fila = 0;
    while (ventas.next()) {
        ui->tableVentas->insertRow(fila);

        auto *itemProducto = new QTableWidgetItem(ventas.value(1).toString());
        itemProducto->setData(Qt::UserRole, ventas.value(0));          // id_venta
        itemProducto->setData(Qt::UserRole + 1, ventas.value(2));      // cantidad vendida
        itemProducto->setData(Qt::UserRole + 2, ventas.value(3));      // precio unitario
        itemProducto->setData(Qt::UserRole + 3, ventas.value(5));      // cantidad ya devuelta
        ui->tableVentas->setItem(fila, 0, itemProducto);
        ui->tableVentas->setItem(fila, 1, new QTableWidgetItem(ventas.value(2).toString()));
        ui->tableVentas->setItem(fila, 2, new QTableWidgetItem(QString::number(ventas.value(3).toDouble(), 'f', 2)));
        ui->tableVentas->setItem(fila, 3, new QTableWidgetItem(QString::number(ventas.value(4).toDouble(), 'f', 2)));
        ui->tableVentas->setItem(fila, 4, new QTableWidgetItem(ventas.value(5).toString()));
        ++fila;
    }

    QSqlQuery pagos;
    pagos.prepare("SELECT fecha_pago, monto, metodo_pago FROM Pagos WHERE id_ticket = :id ORDER BY fecha_pago");
    pagos.bindValue(":id", m_idTicket);
    pagos.exec();

    ui->tablePagos->setRowCount(0);
    fila = 0;
    while (pagos.next()) {
        ui->tablePagos->insertRow(fila);
        ui->tablePagos->setItem(fila, 0, new QTableWidgetItem(pagos.value(0).toDateTime().toString("dd/MM/yyyy HH:mm")));
        ui->tablePagos->setItem(fila, 1, new QTableWidgetItem(QString::number(pagos.value(1).toDouble(), 'f', 2)));
        ui->tablePagos->setItem(fila, 2, new QTableWidgetItem(pagos.value(2).toString()));
        ++fila;
    }
}

void TicketDetalleDialog::registrarAbono() {
    QSqlQuery saldoQuery;
    saldoQuery.prepare("SELECT saldo_pendiente, estado_pago FROM Tickets WHERE id_ticket = :id");
    saldoQuery.bindValue(":id", m_idTicket);
    if (!saldoQuery.exec() || !saldoQuery.next())
        return;

    double saldoPendiente = saldoQuery.value(0).toDouble();
    if (saldoQuery.value(1).toString() == "pagado" || saldoPendiente <= 0) {
        QMessageBox::information(this, "Abono", "Este ticket ya está pagado por completo.");
        return;
    }

    bool ok = false;
    double monto = QInputDialog::getDouble(this, "Registrar abono",
                                            QString("Saldo pendiente: $%1\nMonto a abonar:").arg(saldoPendiente, 0, 'f', 2),
                                            saldoPendiente, 0.01, saldoPendiente, 2, &ok);
    if (!ok)
        return;

    QStringList metodos{"efectivo", "tarjeta", "transferencia"};
    QString metodo = QInputDialog::getItem(this, "Registrar abono", "Método de pago:", metodos, 0, false, &ok);
    if (!ok)
        return;

    QSqlQuery insertarPago;
    insertarPago.prepare("INSERT INTO Pagos (id_ticket, monto, metodo_pago) VALUES (:ticket, :monto, :metodo)");
    insertarPago.bindValue(":ticket", m_idTicket);
    insertarPago.bindValue(":monto", monto);
    insertarPago.bindValue(":metodo", metodo);
    if (!insertarPago.exec()) {
        QMessageBox::critical(this, "Error", "No se pudo registrar el abono:\n" + insertarPago.lastError().text());
        return;
    }

    cargarDatos();
}

void TicketDetalleDialog::registrarDevolucion() {
    auto seleccion = ui->tableVentas->selectionModel()->selectedRows();
    if (seleccion.isEmpty()) {
        QMessageBox::information(this, "Devolución", "Selecciona una línea de producto del ticket.");
        return;
    }

    auto *item = ui->tableVentas->item(seleccion.first().row(), 0);
    int idVenta = item->data(Qt::UserRole).toInt();
    int cantidadVendida = item->data(Qt::UserRole + 1).toInt();
    double precioUnitario = item->data(Qt::UserRole + 2).toDouble();
    int cantidadYaDevuelta = item->data(Qt::UserRole + 3).toInt();
    QString nombreProducto = item->text();

    int disponibleParaDevolver = cantidadVendida - cantidadYaDevuelta;
    if (disponibleParaDevolver <= 0) {
        QMessageBox::information(this, "Devolución", "Esta línea ya fue devuelta por completo.");
        return;
    }

    DevolucionDialog dialog(this);
    dialog.setVenta(idVenta, m_idTicket, m_idSucursal, nombreProducto, disponibleParaDevolver, precioUnitario);
    if (dialog.exec() == QDialog::Accepted)
        cargarDatos();
}
