#include "lotedialog.h"
#include "ui_lotedialog.h"
#include "../database/sesionactual.h"

#include <QDate>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

LoteDialog::LoteDialog(QWidget *parent) : QDialog(parent), ui(new Ui::LoteDialog) {
    ui->setupUi(this);
    ui->dateFechaCaducidad->setDate(QDate::currentDate().addYears(1));
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &LoteDialog::guardar);
    cargarCombos();

    if (!SesionActual::instance().esAdmin()) {
        int idx = ui->comboSucursal->findData(SesionActual::instance().idSucursal());
        if (idx >= 0)
            ui->comboSucursal->setCurrentIndex(idx);
        ui->comboSucursal->setEnabled(false);
    }
}

LoteDialog::~LoteDialog() {
    delete ui;
}

void LoteDialog::cargarCombos() {
    QSqlQuery productos("SELECT id_producto, nombre FROM Productos ORDER BY nombre");
    while (productos.next())
        ui->comboProducto->addItem(productos.value(1).toString(), productos.value(0));

    QSqlQuery sucursales("SELECT id_sucursal, nombre FROM Sucursales ORDER BY nombre");
    while (sucursales.next())
        ui->comboSucursal->addItem(sucursales.value(1).toString(), sucursales.value(0));
}

void LoteDialog::setLoteId(int id) {
    m_idLote = id;
    cargarLote();
}

void LoteDialog::cargarLote() {
    if (m_idLote < 0)
        return;

    QSqlQuery query;
    query.prepare("SELECT id_producto, id_sucursal, numero_lote, fecha_caducidad, cantidad_disponible, estado "
                  "FROM Lotes WHERE id_lote = :id");
    query.bindValue(":id", m_idLote);
    if (!query.exec() || !query.next())
        return;

    ui->comboProducto->setCurrentIndex(ui->comboProducto->findData(query.value(0)));
    ui->comboSucursal->setCurrentIndex(ui->comboSucursal->findData(query.value(1)));
    ui->lineNumeroLote->setText(query.value(2).toString());
    ui->dateFechaCaducidad->setDate(query.value(3).toDate());
    ui->spinCantidad->setValue(query.value(4).toInt());
    ui->comboEstado->setCurrentText(query.value(5).toString());

    // Al editar, el producto/sucursal de un lote ya existente no se cambian
    // (hay ventas/movimientos de inventario ligados a este lote por id).
    ui->comboProducto->setEnabled(false);
    ui->comboSucursal->setEnabled(false);
    ui->labelCantidad->setText("Cantidad disponible:");
}

void LoteDialog::guardar() {
    if (ui->comboProducto->currentIndex() < 0 || ui->comboSucursal->currentIndex() < 0) {
        QMessageBox::warning(this, "Validación", "Selecciona producto y sucursal.");
        return;
    }

    QSqlQuery query;
    if (m_idLote < 0) {
        query.prepare(
            "INSERT INTO Lotes (id_producto, id_sucursal, numero_lote, fecha_caducidad, "
            "cantidad_inicial, cantidad_disponible, estado) "
            "VALUES (:producto, :sucursal, :numero, :caducidad, :cantidad, :cantidad, :estado)");
    } else {
        query.prepare(
            "UPDATE Lotes SET numero_lote = :numero, fecha_caducidad = :caducidad, "
            "cantidad_disponible = :cantidad, estado = :estado WHERE id_lote = :id");
        query.bindValue(":id", m_idLote);
    }

    query.bindValue(":producto", ui->comboProducto->currentData());
    query.bindValue(":sucursal", ui->comboSucursal->currentData());
    query.bindValue(":numero", ui->lineNumeroLote->text().trimmed());
    query.bindValue(":caducidad", ui->dateFechaCaducidad->date());
    query.bindValue(":cantidad", ui->spinCantidad->value());
    query.bindValue(":estado", ui->comboEstado->currentText());

    if (!query.exec()) {
        QMessageBox::critical(this, "Error", "No se pudo guardar el lote:\n" + query.lastError().text());
        return;
    }

    accept();
}
