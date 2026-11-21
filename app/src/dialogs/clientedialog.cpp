#include "clientedialog.h"
#include "../util/validacion.h"
#include "ui_clientedialog.h"
#include "../database/sesionactual.h"

#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

ClienteDialog::ClienteDialog(QWidget *parent) : QDialog(parent), ui(new Ui::ClienteDialog) {
    ui->setupUi(this);
    Validacion::filtrarNombrePersona(ui->lineNombre);
    Validacion::filtrarNombrePersona(ui->lineApellidoPaterno);
    Validacion::filtrarNombrePersona(ui->lineApellidoMaterno);
    Validacion::filtrarTelefono(ui->lineTelefono);
    Validacion::filtrarCorreo(ui->lineCorreo);
    Validacion::filtrarDireccion(ui->lineDireccion);
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

    if (!SesionActual::instance().veTodasLasSucursales()) {
        ui->comboSucursal->setCurrentIndex(ui->comboSucursal->findData(SesionActual::instance().idSucursal()));
        ui->comboSucursal->setEnabled(false);
    }
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
    using namespace Validacion;
    if (rechazar(this, ui->lineNombre, nombrePersona(ui->lineNombre->text(), "El nombre", true))
        || rechazar(this, ui->lineApellidoPaterno, nombrePersona(ui->lineApellidoPaterno->text(), "El apellido paterno", false))
        || rechazar(this, ui->lineApellidoMaterno, nombrePersona(ui->lineApellidoMaterno->text(), "El apellido materno", false))
        || rechazar(this, ui->lineTelefono, telefono(ui->lineTelefono->text()))
        || rechazar(this, ui->lineCorreo, correo(ui->lineCorreo->text()))
        || rechazar(this, ui->lineDireccion, direccion(ui->lineDireccion->text())))
        return;
    if (ui->comboSucursal->currentIndex() < 0) {
        QMessageBox::warning(this, "Validación", "Selecciona una sucursal.");
        return;
    }

    // Mismo nombre completo y teléfono en la misma sucursal: casi seguro es un cliente repetido.
    QSqlQuery repetido;
    repetido.prepare("SELECT COUNT(*) FROM Clientes WHERE id_sucursal = :s AND nombre = :n "
                     "AND COALESCE(apellido_paterno, '') = :ap AND COALESCE(apellido_materno, '') = :am "
                     "AND COALESCE(telefono, '') = :t AND id_cliente <> :id");
    repetido.bindValue(":s", ui->comboSucursal->currentData());
    repetido.bindValue(":n", normalizar(ui->lineNombre->text()));
    repetido.bindValue(":ap", normalizar(ui->lineApellidoPaterno->text()));
    repetido.bindValue(":am", normalizar(ui->lineApellidoMaterno->text()));
    repetido.bindValue(":t", ui->lineTelefono->text().trimmed());
    repetido.bindValue(":id", m_idCliente);
    if (repetido.exec() && repetido.next() && repetido.value(0).toInt() > 0) {
        QMessageBox::warning(this, "Cliente repetido",
                             "Ya existe un cliente con ese nombre y ese teléfono en esta sucursal.");
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
    query.bindValue(":nombre", normalizar(ui->lineNombre->text()));
    query.bindValue(":apPaterno", normalizar(ui->lineApellidoPaterno->text()));
    query.bindValue(":apMaterno", normalizar(ui->lineApellidoMaterno->text()));
    query.bindValue(":telefono", ui->lineTelefono->text().trimmed());
    query.bindValue(":correo", ui->lineCorreo->text().trimmed());
    query.bindValue(":direccion", normalizar(ui->lineDireccion->text()));
    query.bindValue(":estado", ui->comboEstado->currentText());

    if (!query.exec()) {
        QMessageBox::critical(this, "Error", "No se pudo guardar el cliente:\n" + query.lastError().text());
        return;
    }

    accept();
}
