#include "proveedordialog.h"
#include "ui_proveedordialog.h"

#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

ProveedorDialog::ProveedorDialog(QWidget *parent) : QDialog(parent), ui(new Ui::ProveedorDialog) {
    ui->setupUi(this);
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
    if (ui->lineNombreEmpresa->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Validación", "El nombre de la empresa es obligatorio.");
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

    query.bindValue(":nombre", ui->lineNombreEmpresa->text().trimmed());
    query.bindValue(":telefono", ui->lineTelefono->text().trimmed());
    query.bindValue(":correo", ui->lineCorreo->text().trimmed());
    query.bindValue(":direccion", ui->lineDireccion->text().trimmed());
    query.bindValue(":estado", ui->comboEstado->currentText());

    if (!query.exec()) {
        QMessageBox::critical(this, "Error", "No se pudo guardar el proveedor:\n" + query.lastError().text());
        return;
    }

    accept();
}
