#include "clientedialog.h"
#include "ui_clientedialog.h"

#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

ClienteDialog::ClienteDialog(QWidget *parent) : QDialog(parent), ui(new Ui::ClienteDialog) {
    ui->setupUi(this);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &ClienteDialog::guardar);
    cargarCombos();
}

ClienteDialog::~ClienteDialog() {
    delete ui;
}

void ClienteDialog::cargarCombos() {
    QSqlQuery sucursales("SELECT id_sucursal, nombre FROM Sucursales ORDER BY nombre");
    while (sucursales.next())
        ui->comboSucursal->addItem(sucursales.value(1).toString(), sucursales.value(0));
}

void ClienteDialog::setClienteId(int id) {
    m_idCliente = id;
    cargarCliente();
}

void ClienteDialog::cargarCliente() {
    if (m_idCliente < 0)
        return;

    QSqlQuery query;
    query.prepare("SELECT id_sucursal, nombre, apellido_paterno, apellido_materno, telefono, "
                  "correo, direccion, estado FROM Clientes WHERE id_cliente = :id");
    query.bindValue(":id", m_idCliente);
    if (!query.exec() || !query.next())
        return;

    ui->comboSucursal->setCurrentIndex(ui->comboSucursal->findData(query.value(0)));
    ui->lineNombre->setText(query.value(1).toString());
    ui->lineApellidoPaterno->setText(query.value(2).toString());
    ui->lineApellidoMaterno->setText(query.value(3).toString());
    ui->lineTelefono->setText(query.value(4).toString());
    ui->lineCorreo->setText(query.value(5).toString());
    ui->lineDireccion->setText(query.value(6).toString());
    ui->comboEstado->setCurrentText(query.value(7).toString());
}

void ClienteDialog::guardar() {
    if (ui->lineNombre->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Validación", "El nombre es obligatorio.");
        return;
    }
    if (ui->comboSucursal->currentIndex() < 0) {
        QMessageBox::warning(this, "Validación", "Selecciona una sucursal.");
        return;
    }

    QSqlQuery query;
    if (m_idCliente < 0) {
        query.prepare(
            "INSERT INTO Clientes (id_sucursal, nombre, apellido_paterno, apellido_materno, "
            "telefono, correo, direccion, estado) "
            "VALUES (:sucursal, :nombre, :apPaterno, :apMaterno, :telefono, :correo, :direccion, :estado)");
    } else {
        query.prepare(
            "UPDATE Clientes SET id_sucursal = :sucursal, nombre = :nombre, "
            "apellido_paterno = :apPaterno, apellido_materno = :apMaterno, telefono = :telefono, "
            "correo = :correo, direccion = :direccion, estado = :estado WHERE id_cliente = :id");
        query.bindValue(":id", m_idCliente);
    }

    query.bindValue(":sucursal", ui->comboSucursal->currentData());
    query.bindValue(":nombre", ui->lineNombre->text().trimmed());
    query.bindValue(":apPaterno", ui->lineApellidoPaterno->text().trimmed());
    query.bindValue(":apMaterno", ui->lineApellidoMaterno->text().trimmed());
    query.bindValue(":telefono", ui->lineTelefono->text().trimmed());
    query.bindValue(":correo", ui->lineCorreo->text().trimmed());
    query.bindValue(":direccion", ui->lineDireccion->text().trimmed());
    query.bindValue(":estado", ui->comboEstado->currentText());

    if (!query.exec()) {
        QMessageBox::critical(this, "Error", "No se pudo guardar el cliente:\n" + query.lastError().text());
        return;
    }

    accept();
}
