#include "ticketdetalledialog.h"
#include "ui_ticketdetalledialog.h"
#include "../database/databasemanager.h"
#include "devoluciondialog.h"
#include "pagotarjetadialog.h"
#include "vistaticketdialog.h"
#include "../util/iconos.h"
#include "../util/tablas.h"

#include <QDateTime>
#include <QLineEdit>
#include <QHeaderView>
#include <QInputDialog>
#include <QMessageBox>
#include <QPushButton>
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
    connect(ui->btnCompletarReembolso, &QPushButton::clicked, this, &TicketDetalleDialog::completarReembolso);
    {
        auto *h = ui->tableDevoluciones->horizontalHeader();
        h->setStretchLastSection(true);
        h->setSectionResizeMode(QHeaderView::ResizeToContents);
    }
    ui->tableDevoluciones->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableDevoluciones->setAlternatingRowColors(true);
    ui->tableDevoluciones->verticalHeader()->setVisible(false);
    Tablas::aplicar(ui->tableDevoluciones, "Sin devoluciones.");

    // El central consulta lo que venden las sucursales, no registra abonos ni devoluciones por ellas.
    if (DatabaseManager::funcion() == DatabaseManager::Funcion::Central) {
        ui->btnRegistrarAbono->setEnabled(false);
        ui->btnRegistrarDevolucion->setEnabled(false);
        ui->btnCompletarReembolso->setEnabled(false);
    }

    // «Ver ticket»: el ticket tal como saldría impreso (también sirve sin impresora, en PDF).
    auto *btnVerTicket = new QPushButton("Ver / imprimir ticket", this);
    btnVerTicket->setObjectName("btnVerTicket");
    btnVerTicket->setIcon(Iconos::tickets());
    ui->accionesLayout->insertWidget(0, btnVerTicket);
    connect(btnVerTicket, &QPushButton::clicked, this, [this] {
        VistaTicketDialog vista(m_idTicket, -1, 0, this);
        vista.exec();
    });

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
    pagos.prepare("SELECT fecha_pago, monto, metodo_pago, numero_referencia, ultimos_digitos FROM Pagos "
                  "WHERE id_ticket = :id ORDER BY fecha_pago");
    pagos.bindValue(":id", m_idTicket);
    pagos.exec();

    ui->tablePagos->setRowCount(0);
    fila = 0;
    while (pagos.next()) {
        ui->tablePagos->insertRow(fila);
        ui->tablePagos->setItem(fila, 0, new QTableWidgetItem(pagos.value(0).toDateTime().toString("dd/MM/yyyy HH:mm")));
        ui->tablePagos->setItem(fila, 1, new QTableWidgetItem(QString::number(pagos.value(1).toDouble(), 'f', 2)));
        ui->tablePagos->setItem(fila, 2, new QTableWidgetItem(pagos.value(2).toString()));
        QString referencia = pagos.value(3).toString();
        if (!pagos.value(4).isNull())
            referencia += (referencia.isEmpty() ? "" : " · ") + QString("•••• %1").arg(pagos.value(4).toString());
        ui->tablePagos->setItem(fila, 3, new QTableWidgetItem(referencia));
        ++fila;
    }

    QSqlQuery devoluciones;
    devoluciones.prepare("SELECT d.fecha_devolucion, d.total_devuelto, d.tipo_reembolso, d.modalidad_tarjeta, d.estado, "
                         "d.referencia_reembolso, "
                         "(SELECT GROUP_CONCAT(CONCAT(dd.cantidad, ' × ', p.nombre) SEPARATOR ', ') "
                         " FROM Detalle_Devolucion dd JOIN Ventas v ON v.id_venta = dd.id_venta "
                         " JOIN Lotes l ON l.id_lote = v.id_lote JOIN Productos p ON p.id_producto = l.id_producto "
                         " WHERE dd.id_devolucion = d.id_devolucion) AS productos "
                         "FROM Devoluciones d WHERE d.id_ticket = :id ORDER BY d.fecha_devolucion");
    devoluciones.bindValue(":id", m_idTicket);
    devoluciones.exec();
    ui->tableDevoluciones->setRowCount(0);
    fila = 0;
    int pendientes = 0;
    while (devoluciones.next()) {
        ui->tableDevoluciones->insertRow(fila);
        ui->tableDevoluciones->setItem(fila, 0, new QTableWidgetItem(devoluciones.value(0).toDateTime().toString("dd/MM/yyyy HH:mm")));
        ui->tableDevoluciones->setItem(fila, 1, new QTableWidgetItem(devoluciones.value(6).toString()));
        ui->tableDevoluciones->setItem(fila, 2, new QTableWidgetItem(QString::number(devoluciones.value(1).toDouble(), 'f', 2)));
        QString reembolso = devoluciones.value(2).toString();
        if (!devoluciones.value(3).isNull())
            reembolso += " (" + devoluciones.value(3).toString() + ")";
        ui->tableDevoluciones->setItem(fila, 3, new QTableWidgetItem(reembolso));
        const QString estado = devoluciones.value(4).toString();
        ui->tableDevoluciones->setItem(fila, 4, new QTableWidgetItem(estado == "solicitada" ? "pendiente en terminal" : estado));
        ui->tableDevoluciones->setItem(fila, 5, new QTableWidgetItem(devoluciones.value(5).toString()));
        if (estado == "solicitada" && devoluciones.value(2).toString() == "tarjeta")
            ++pendientes;
        ++fila;
    }
    ui->labelDevoluciones->setText(pendientes > 0
        ? QString("Devoluciones: <b>%1 reembolso(s) a tarjeta pendiente(s)</b> — haz el reembolso en la terminal y registra el folio.").arg(pendientes)
        : "Devoluciones:");
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

    QString autorizacion, ultimosDigitos;
    if (metodo == "tarjeta") {
        PagoTarjetaDialog dialogoTarjeta(monto, this);
        if (dialogoTarjeta.exec() != QDialog::Accepted)
            return;
        autorizacion = dialogoTarjeta.autorizacion();
        ultimosDigitos = dialogoTarjeta.ultimosDigitos();
    }

    QSqlQuery insertarPago;
    insertarPago.prepare("INSERT INTO Pagos (id_ticket, monto, metodo_pago, numero_referencia, ultimos_digitos) "
                         "VALUES (:ticket, :monto, :metodo, :referencia, :ultimos)");
    insertarPago.bindValue(":ticket", m_idTicket);
    insertarPago.bindValue(":monto", monto);
    insertarPago.bindValue(":metodo", metodo);
    insertarPago.bindValue(":referencia", autorizacion.isEmpty() ? QVariant(QMetaType(QMetaType::QString)) : QVariant(autorizacion));
    insertarPago.bindValue(":ultimos", ultimosDigitos.isEmpty() ? QVariant(QMetaType(QMetaType::QString)) : QVariant(ultimosDigitos));
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

// Un reembolso a tarjeta se hace en la terminal; aquí se captura el folio que imprime para darlo por completado.
void TicketDetalleDialog::completarReembolso() {
    QSqlQuery pendientes;
    pendientes.prepare("SELECT id_devolucion, total_devuelto, modalidad_tarjeta FROM Devoluciones "
                       "WHERE id_ticket = :id AND tipo_reembolso = 'tarjeta' AND estado = 'solicitada' "
                       "ORDER BY fecha_devolucion");
    pendientes.bindValue(":id", m_idTicket);
    if (!pendientes.exec())
        return;

    QStringList opciones;
    QList<int> ids;
    while (pendientes.next()) {
        ids << pendientes.value(0).toInt();
        opciones << QString("$%1 — %2").arg(pendientes.value(1).toDouble(), 0, 'f', 2).arg(pendientes.value(2).toString());
    }
    if (ids.isEmpty()) {
        QMessageBox::information(this, "Reembolso a tarjeta", "Este ticket no tiene reembolsos a tarjeta pendientes.");
        return;
    }

    int indice = 0;
    if (ids.size() > 1) {
        bool ok = false;
        const QString elegida = QInputDialog::getItem(this, "Reembolso a tarjeta", "¿Cuál reembolso ya hiciste en la terminal?",
                                                      opciones, 0, false, &ok);
        if (!ok)
            return;
        indice = opciones.indexOf(elegida);
    }

    QString folio;
    while (true) {
        bool ok = false;
        folio = QInputDialog::getText(this, "Folio del reembolso",
                                      "Folio o autorización que imprimió la terminal al cancelar/devolver:",
                                      QLineEdit::Normal, folio, &ok).trimmed().toUpper();
        if (!ok)
            return;
        if (PagoTarjetaDialog::folioValido(folio))
            break;
        QMessageBox::warning(this, "Folio", "Debe tener de 4 a 20 letras o números, sin espacios.");
    }

    QSqlQuery actualizar;
    actualizar.prepare("UPDATE Devoluciones SET estado = 'completada', referencia_reembolso = :folio "
                       "WHERE id_devolucion = :id AND estado = 'solicitada'");
    actualizar.bindValue(":folio", folio);
    actualizar.bindValue(":id", ids.at(indice));
    if (!actualizar.exec()) {
        QMessageBox::critical(this, "Error", "No se pudo registrar el folio:\n" + actualizar.lastError().text());
        return;
    }
    cargarDatos();
}
