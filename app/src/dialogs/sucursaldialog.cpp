#include "sucursaldialog.h"
#include "../util/validacion.h"
#include "ui_sucursaldialog.h"

#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

SucursalDialog::SucursalDialog(QWidget *parent) : QDialog(parent), ui(new Ui::SucursalDialog) {
    ui->setupUi(this);
    Validacion::filtrarTextoComercial(ui->lineNombre, 100);
    Validacion::filtrarDireccion(ui->lineDireccion);
    Validacion::filtrarTelefono(ui->lineTelefono);
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
    using namespace Validacion;
    if (rechazar(this, ui->lineNombre, textoComercial(ui->lineNombre->text(), "El nombre de la sucursal", true, 100))
        || rechazar(this, ui->lineDireccion, direccion(ui->lineDireccion->text()))
        || rechazar(this, ui->lineTelefono, telefono(ui->lineTelefono->text())))
        return;
    QSqlQuery repetido;
    repetido.prepare("SELECT COUNT(*) FROM Sucursales WHERE nombre = :n AND id_sucursal <> :id");
    repetido.bindValue(":n", normalizar(ui->lineNombre->text()));
    repetido.bindValue(":id", m_idSucursal);
    if (repetido.exec() && repetido.next() && repetido.value(0).toInt() > 0) {
        QMessageBox::warning(this, "Sucursal repetida", "Ya existe una sucursal con ese nombre.");
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
