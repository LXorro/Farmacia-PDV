#include "devoluciondialog.h"
#include "ui_devoluciondialog.h"
#include "../database/sesionactual.h"

#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

DevolucionDialog::DevolucionDialog(QWidget *parent) : QDialog(parent), ui(new Ui::DevolucionDialog) {
    ui->setupUi(this);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &DevolucionDialog::guardar);
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
}

void DevolucionDialog::cargarEmpleados(int idSucursal) {
    ui->comboEmpleado->clear();
    QSqlQuery query;
    query.prepare("SELECT id_empleado, nombre, apellido_paterno FROM Empleados "
                  "WHERE id_sucursal = :suc AND estado = 'activo' ORDER BY nombre");
    query.bindValue(":suc", idSucursal);
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

    QSqlDatabase db = QSqlDatabase::database();
    if (!db.transaction()) {
        QMessageBox::critical(this, "Error", "No se pudo iniciar la transacción:\n" + db.lastError().text());
        return;
    }

    QSqlQuery insertarDevolucion;
    insertarDevolucion.prepare(
        "INSERT INTO Devoluciones (id_sucursal, id_ticket, id_empleado, motivo, total_devuelto, tipo_reembolso, estado) "
        "SELECT id_sucursal, :ticket, :empleado, :motivo, :total, :tipo, 'completada' FROM Tickets WHERE id_ticket = :ticket");
    insertarDevolucion.bindValue(":ticket", m_idTicket);
    insertarDevolucion.bindValue(":empleado", ui->comboEmpleado->currentData());
    insertarDevolucion.bindValue(":motivo", ui->lineMotivo->text().trimmed());
    insertarDevolucion.bindValue(":total", importe);
    insertarDevolucion.bindValue(":tipo", ui->comboTipoReembolso->currentText());
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
