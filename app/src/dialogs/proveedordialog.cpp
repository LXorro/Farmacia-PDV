#include "proveedordialog.h"
#include "../util/validacion.h"
#include "ui_proveedordialog.h"

#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

ProveedorDialog::ProveedorDialog(QWidget *parent) : QDialog(parent), ui(new Ui::ProveedorDialog) {
    ui->setupUi(this);
    Validacion::filtrarTextoComercial(ui->lineNombreEmpresa, 100);
    Validacion::filtrarTelefono(ui->lineTelefono);
    Validacion::filtrarCorreo(ui->lineCorreo);
    Validacion::filtrarDireccion(ui->lineDireccion);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &ProveedorDialog::guardar);
}

ProveedorDialog::~ProveedorDialog() {
    delete ui;
}

void ProveedorDialog::setProveedorId(int id) {
    m_idProveedor = id;
    cargarProveedor();
}

void ProveedorDialog::cargarProveedor() {
    if (m_idProveedor < 0)
        return;

    QSqlQuery query;
    query.prepare("SELECT nombre_empresa, telefono, correo, direccion, estado FROM Proveedores WHERE id_proveedor = :id");
    query.bindValue(":id", m_idProveedor);
    if (!query.exec() || !query.next())
        return;

    ui->lineNombreEmpresa->setText(query.value(0).toString());
    ui->lineTelefono->setText(query.value(1).toString());
    ui->lineCorreo->setText(query.value(2).toString());
    ui->lineDireccion->setText(query.value(3).toString());
    ui->comboEstado->setCurrentText(query.value(4).toString());
}

void ProveedorDialog::guardar() {
    using namespace Validacion;
    if (rechazar(this, ui->lineNombreEmpresa, textoComercial(ui->lineNombreEmpresa->text(), "El nombre de la empresa", true, 100))
        || rechazar(this, ui->lineTelefono, telefono(ui->lineTelefono->text()))
        || rechazar(this, ui->lineCorreo, correo(ui->lineCorreo->text()))
        || rechazar(this, ui->lineDireccion, direccion(ui->lineDireccion->text())))
        return;
    QSqlQuery repetido;
    repetido.prepare("SELECT COUNT(*) FROM Proveedores WHERE nombre_empresa = :n AND id_proveedor <> :id");
    repetido.bindValue(":n", normalizar(ui->lineNombreEmpresa->text()));
    repetido.bindValue(":id", m_idProveedor);
    if (repetido.exec() && repetido.next() && repetido.value(0).toInt() > 0) {
        QMessageBox::warning(this, "Proveedor repetido", "Ya existe un proveedor con ese nombre.");
        return;
    }

    QSqlQuery query;
    if (m_idProveedor < 0) {
        query.prepare("INSERT INTO Proveedores (nombre_empresa, telefono, correo, direccion, estado) "
                      "VALUES (:nombre, :telefono, :correo, :direccion, :estado)");
    } else {
        query.prepare("UPDATE Proveedores SET nombre_empresa = :nombre, telefono = :telefono, "
                      "correo = :correo, direccion = :direccion, estado = :estado WHERE id_proveedor = :id");
        query.bindValue(":id", m_idProveedor);
    }

    query.bindValue(":nombre", normalizar(ui->lineNombreEmpresa->text()));
    query.bindValue(":telefono", ui->lineTelefono->text().trimmed());
    query.bindValue(":correo", ui->lineCorreo->text().trimmed());
    query.bindValue(":direccion", normalizar(ui->lineDireccion->text()));
    query.bindValue(":estado", ui->comboEstado->currentText());

    if (!query.exec()) {
        QMessageBox::critical(this, "Error", "No se pudo guardar el proveedor:\n" + query.lastError().text());
        return;
    }

    accept();
}
