#include "devoluciondialog.h"
#include "ui_devoluciondialog.h"
#include "../database/sesionactual.h"
#include "pagotarjetadialog.h"

#include <QDate>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

DevolucionDialog::DevolucionDialog(QWidget *parent) : QDialog(parent), ui(new Ui::DevolucionDialog) {
    ui->setupUi(this);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &DevolucionDialog::guardar);
    connect(ui->comboTipoReembolso, &QComboBox::currentTextChanged, this, &DevolucionDialog::actualizarTipo);
    actualizarTipo();
}

DevolucionDialog::~DevolucionDialog() {
    delete ui;
}

void DevolucionDialog::setVenta(int idVenta, int idTicket, int idSucursal, const QString &nombreProducto,
                                 int cantidadDisponibleParaDevolver, double precioUnitario) {
    m_idVenta = idVenta;
    m_idTicket = idTicket;
    m_cantidadMaxima = cantidadDisponibleParaDevolver;
    m_precioUnitario = precioUnitario;

    ui->labelProducto->setText(nombreProducto);
    ui->spinCantidad->setMaximum(cantidadDisponibleParaDevolver);
    cargarEmpleados(idSucursal);
    cargarPagosDelTicket();
}

// Un ticket pagado con tarjeta se devuelve a la tarjeta (desde la terminal): el efectivo no sustituye a un
// cargo que seguiría en la cuenta del cliente. Aquí se calcula cuánto puede salir por cada vía.
void DevolucionDialog::cargarPagosDelTicket() {
    QSqlQuery pagos;
    pagos.prepare("SELECT "
                  "COALESCE(SUM(CASE WHEN metodo_pago = 'tarjeta' THEN monto END), 0), "
                  "COALESCE(SUM(CASE WHEN metodo_pago <> 'tarjeta' THEN monto END), 0), "
                  "MAX(CASE WHEN metodo_pago = 'tarjeta' THEN DATE(fecha_pago) END) "
                  "FROM Pagos WHERE id_ticket = :t AND estado = 'aplicado'");
    pagos.bindValue(":t", m_idTicket);
    double otrosPagado = 0;
    if (pagos.exec() && pagos.next()) {
        m_tarjetaPagada = pagos.value(0).toDouble();
        otrosPagado = pagos.value(1).toDouble();
        m_pagoTarjetaHoy = pagos.value(2).toDate() == QDate::currentDate();
    }

    QSqlQuery devuelto;
    devuelto.prepare("SELECT "
                     "COALESCE(SUM(CASE WHEN tipo_reembolso = 'tarjeta' THEN total_devuelto END), 0), "
                     "COALESCE(SUM(CASE WHEN tipo_reembolso IN ('efectivo', 'transferencia') THEN total_devuelto END), 0) "
                     "FROM Devoluciones WHERE id_ticket = :t AND estado <> 'rechazada'");
    devuelto.bindValue(":t", m_idTicket);
    double devueltoTarjeta = 0, devueltoOtros = 0;
    if (devuelto.exec() && devuelto.next()) {
        devueltoTarjeta = devuelto.value(0).toDouble();
        devueltoOtros = devuelto.value(1).toDouble();
    }
    m_tarjetaDisponible = qMax(0.0, m_tarjetaPagada - devueltoTarjeta);
    m_otrosDisponible = qMax(0.0, otrosPagado - devueltoOtros);

    // «tarjeta» solo tiene sentido si el ticket se pagó con tarjeta; si es así, es lo que se propone.
    const int idx = ui->comboTipoReembolso->findText("tarjeta");
    if (m_tarjetaPagada <= 0.0) {
        if (idx >= 0)
            ui->comboTipoReembolso->removeItem(idx);
    } else if (m_otrosDisponible <= 0.0 && idx >= 0) {
        ui->comboTipoReembolso->setCurrentIndex(idx);
    }
    // Una cancelación solo es posible el mismo día del cobro (antes del cierre de lote).
    ui->comboModalidad->setCurrentIndex(m_pagoTarjetaHoy ? 0 : 1);
    actualizarTipo();
}

void DevolucionDialog::actualizarTipo() {
    const bool tarjeta = ui->comboTipoReembolso->currentText() == "tarjeta";
    for (QWidget *w : {static_cast<QWidget *>(ui->labelModalidad), static_cast<QWidget *>(ui->comboModalidad),
                       static_cast<QWidget *>(ui->labelFolio), static_cast<QWidget *>(ui->lineFolio),
                       static_cast<QWidget *>(ui->labelAvisoTarjeta)})
        w->setVisible(tarjeta);
    ui->labelAvisoTarjeta->setText(
        "El reembolso a tarjeta se hace en la <b>terminal</b>: cancelación si es el mismo día y no se ha cerrado el "
        "lote; devolución después. Captura el folio que imprime. Si todavía no lo tienes, déjalo vacío: la devolución "
        "quedará <i>solicitada</i> hasta que lo registres desde el detalle del ticket.");
}

void DevolucionDialog::cargarEmpleados(int idSucursal) {
    ui->comboEmpleado->clear();
    QSqlQuery query;
    // Personal de la sucursal más quien opera (puede ser un empleado con asignación temporal).
    query.prepare("SELECT id_empleado, nombre, apellido_paterno FROM Empleados "
                  "WHERE (id_sucursal = :suc OR id_empleado = :yo) AND estado = 'activo' ORDER BY nombre");
    query.bindValue(":suc", idSucursal);
    query.bindValue(":yo", SesionActual::instance().idEmpleado());
    query.exec();
    while (query.next())
        ui->comboEmpleado->addItem(query.value(1).toString() + " " + query.value(2).toString(), query.value(0));

    // Comodidad: preseleccionar al empleado logueado, pero cualquiera de la
    // sucursal puede atender la devolución aunque la venta sea de otro.
    int idx = ui->comboEmpleado->findData(SesionActual::instance().idEmpleado());
    if (idx >= 0)
        ui->comboEmpleado->setCurrentIndex(idx);
}

void DevolucionDialog::guardar() {
    if (m_cantidadMaxima <= 0) {
        QMessageBox::warning(this, "Devolución", "No queda cantidad disponible para devolver de esta línea.");
        return;
    }
    if (ui->comboEmpleado->currentIndex() < 0) {
        QMessageBox::warning(this, "Validación", "Selecciona el empleado que atiende la devolución.");
        return;
    }

    int cantidad = ui->spinCantidad->value();
    double importe = cantidad * m_precioUnitario;

    const QString tipo = ui->comboTipoReembolso->currentText();
    if (tipo == "tarjeta" && importe > m_tarjetaDisponible + 0.005) {
        QMessageBox::warning(this, "Reembolso a tarjeta",
                             QString("Solo quedan $%1 por reembolsar a la tarjeta en este ticket.")
                                 .arg(m_tarjetaDisponible, 0, 'f', 2));
        return;
    }
    // Ticket pagado (en parte) con tarjeta: el efectivo/transferencia solo alcanza para lo que se pagó así.
    if ((tipo == "efectivo" || tipo == "transferencia") && m_tarjetaPagada > 0.0 && importe > m_otrosDisponible + 0.005) {
        QMessageBox::warning(
            this, "Pagó con tarjeta",
            QString("Este ticket se pagó con tarjeta, así que el dinero se devuelve a la tarjeta desde la terminal, "
                    "no en %1.\n\nPor efectivo o transferencia solo se puede devolver $%2.\n"
                    "Elige «tarjeta», «cambio» o «saldo_a_favor».")
                .arg(tipo).arg(m_otrosDisponible, 0, 'f', 2));
        return;
    }
    QString folio = ui->lineFolio->text().trimmed().toUpper();
    if (tipo == "tarjeta" && !folio.isEmpty() && !PagoTarjetaDialog::folioValido(folio)) {
        QMessageBox::warning(this, "Folio", "El folio debe tener de 4 a 20 letras o números, sin espacios (o déjalo vacío).");
        return;
    }
    // Con folio la devolución queda completa; sin él, pendiente de reembolsar en la terminal.
    const QString estado = (tipo == "tarjeta" && folio.isEmpty()) ? "solicitada" : "completada";

    QSqlDatabase db = QSqlDatabase::database();
    if (!db.transaction()) {
        QMessageBox::critical(this, "Error", "No se pudo iniciar la transacción:\n" + db.lastError().text());
        return;
    }

    QSqlQuery insertarDevolucion;
    insertarDevolucion.prepare(
        "INSERT INTO Devoluciones (id_sucursal, id_ticket, id_empleado, motivo, total_devuelto, tipo_reembolso, estado, "
        "modalidad_tarjeta, referencia_reembolso) "
        "SELECT id_sucursal, :ticket, :empleado, :motivo, :total, :tipo, :estado, :modalidad, :referencia "
        "FROM Tickets WHERE id_ticket = :ticket");
    insertarDevolucion.bindValue(":ticket", m_idTicket);
    insertarDevolucion.bindValue(":empleado", ui->comboEmpleado->currentData());
    insertarDevolucion.bindValue(":motivo", ui->lineMotivo->text().trimmed());
    insertarDevolucion.bindValue(":total", importe);
    insertarDevolucion.bindValue(":tipo", tipo);
    insertarDevolucion.bindValue(":estado", estado);
    insertarDevolucion.bindValue(":modalidad", tipo == "tarjeta"
                                                   ? QVariant(ui->comboModalidad->currentIndex() == 0 ? "cancelacion" : "devolucion")
                                                   : QVariant(QMetaType(QMetaType::QString)));
    insertarDevolucion.bindValue(":referencia", tipo == "tarjeta" && !folio.isEmpty()
                                                    ? QVariant(folio) : QVariant(QMetaType(QMetaType::QString)));
    if (!insertarDevolucion.exec()) {
        db.rollback();
        QMessageBox::critical(this, "Error", "No se pudo registrar la devolución:\n" + insertarDevolucion.lastError().text());
        return;
    }
    int idDevolucion = insertarDevolucion.lastInsertId().toInt();

    QSqlQuery insertarDetalle;
    insertarDetalle.prepare(
        "INSERT INTO Detalle_Devolucion (id_devolucion, id_venta, cantidad, importe, estado_producto, reintegrar_stock) "
        "VALUES (:devolucion, :venta, :cantidad, :importe, :estadoProducto, :reintegrar)");
    insertarDetalle.bindValue(":devolucion", idDevolucion);
    insertarDetalle.bindValue(":venta", m_idVenta);
    insertarDetalle.bindValue(":cantidad", cantidad);
    insertarDetalle.bindValue(":importe", importe);
    insertarDetalle.bindValue(":estadoProducto", ui->comboEstadoProducto->currentText());
    insertarDetalle.bindValue(":reintegrar", ui->checkReintegrarStock->isChecked());
    if (!insertarDetalle.exec()) {
        db.rollback();
        QMessageBox::critical(this, "Error", "No se pudo registrar el detalle de la devolución:\n" + insertarDetalle.lastError().text());
        return;
    }

    if (!db.commit()) {
        db.rollback();
        QMessageBox::critical(this, "Error", "No se pudo confirmar la devolución:\n" + db.lastError().text());
        return;
    }

    accept();
}
