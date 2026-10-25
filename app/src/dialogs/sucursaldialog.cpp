#include "sucursaldialog.h"
#include "ui_sucursaldialog.h"

#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

SucursalDialog::SucursalDialog(QWidget *parent) : QDialog(parent), ui(new Ui::SucursalDialog) {
    ui->setupUi(this);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &SucursalDialog::guardar);
}

SucursalDialog::~SucursalDialog() {
    delete ui;
}

void SucursalDialog::setSucursalId(int id) {
    m_idSucursal = id;
    cargarSucursal();
}

void SucursalDialog::cargarSucursal() {
    if (m_idSucursal < 0)
        return;

    QSqlQuery query;
    query.prepare("SELECT nombre, direccion, telefono, rol, estado FROM Sucursales WHERE id_sucursal = :id");
    query.bindValue(":id", m_idSucursal);
    if (!query.exec() || !query.next())
        return;

    ui->lineNombre->setText(query.value(0).toString());
    ui->lineDireccion->setText(query.value(1).toString());
    ui->lineTelefono->setText(query.value(2).toString());
    ui->comboRol->setCurrentText(query.value(3).toString());
    ui->comboEstado->setCurrentText(query.value(4).toString());
}

void SucursalDialog::guardar() {
    if (ui->lineNombre->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Validación", "El nombre es obligatorio.");
        return;
    }

    QSqlQuery query;
    if (m_idSucursal < 0) {
        query.prepare("INSERT INTO Sucursales (nombre, direccion, telefono, rol, estado) "
                      "VALUES (:nombre, :direccion, :telefono, :rol, :estado)");
    } else {
        query.prepare("UPDATE Sucursales SET nombre = :nombre, direccion = :direccion, "
                      "telefono = :telefono, rol = :rol, estado = :estado WHERE id_sucursal = :id");
        query.bindValue(":id", m_idSucursal);
    }

    query.bindValue(":nombre", ui->lineNombre->text().trimmed());
    query.bindValue(":direccion", ui->lineDireccion->text().trimmed());
    query.bindValue(":telefono", ui->lineTelefono->text().trimmed());
    query.bindValue(":rol", ui->comboRol->currentText());
    query.bindValue(":estado", ui->comboEstado->currentText());

    if (!query.exec()) {
        QMessageBox::critical(this, "Error", "No se pudo guardar la sucursal:\n" + query.lastError().text());
        return;
    }

    accept();
}
