#include "empleadodialog.h"
#include "ui_empleadodialog.h"

#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

EmpleadoDialog::EmpleadoDialog(QWidget *parent) : QDialog(parent), ui(new Ui::EmpleadoDialog) {
    ui->setupUi(this);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &EmpleadoDialog::guardar);
    cargarCombos();
}

EmpleadoDialog::~EmpleadoDialog() {
    delete ui;
}

void EmpleadoDialog::cargarCombos() {
    QSqlQuery sucursales("SELECT id_sucursal, nombre FROM Sucursales ORDER BY nombre");
    while (sucursales.next())
        ui->comboSucursal->addItem(sucursales.value(1).toString(), sucursales.value(0));
}

void EmpleadoDialog::setEmpleadoId(int id) {
    m_idEmpleado = id;
    cargarEmpleado();
}

void EmpleadoDialog::cargarEmpleado() {
    if (m_idEmpleado < 0)
        return;

    QSqlQuery query;
    query.prepare("SELECT id_sucursal, nombre, apellido_paterno, apellido_materno, puesto, "
                  "telefono, correo, estado FROM Empleados WHERE id_empleado = :id");
    query.bindValue(":id", m_idEmpleado);
    if (!query.exec() || !query.next())
        return;

    ui->comboSucursal->setCurrentIndex(ui->comboSucursal->findData(query.value(0)));
    ui->lineNombre->setText(query.value(1).toString());
    ui->lineApellidoPaterno->setText(query.value(2).toString());
    ui->lineApellidoMaterno->setText(query.value(3).toString());
    ui->linePuesto->setText(query.value(4).toString());
    ui->lineTelefono->setText(query.value(5).toString());
    ui->lineCorreo->setText(query.value(6).toString());
    ui->comboEstado->setCurrentText(query.value(7).toString());
}

void EmpleadoDialog::guardar() {
    if (ui->lineNombre->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Validación", "El nombre es obligatorio.");
        return;
    }
    if (ui->comboSucursal->currentIndex() < 0) {
        QMessageBox::warning(this, "Validación", "Selecciona una sucursal.");
        return;
    }

    QSqlQuery query;
    if (m_idEmpleado < 0) {
        query.prepare(
            "INSERT INTO Empleados (id_sucursal, nombre, apellido_paterno, apellido_materno, puesto, "
            "telefono, correo, estado) "
            "VALUES (:sucursal, :nombre, :apPaterno, :apMaterno, :puesto, :telefono, :correo, :estado)");
    } else {
        query.prepare(
            "UPDATE Empleados SET id_sucursal = :sucursal, nombre = :nombre, "
            "apellido_paterno = :apPaterno, apellido_materno = :apMaterno, puesto = :puesto, "
            "telefono = :telefono, correo = :correo, estado = :estado WHERE id_empleado = :id");
        query.bindValue(":id", m_idEmpleado);
    }

    query.bindValue(":sucursal", ui->comboSucursal->currentData());
    query.bindValue(":nombre", ui->lineNombre->text().trimmed());
    query.bindValue(":apPaterno", ui->lineApellidoPaterno->text().trimmed());
    query.bindValue(":apMaterno", ui->lineApellidoMaterno->text().trimmed());
    query.bindValue(":puesto", ui->linePuesto->text().trimmed());
    query.bindValue(":telefono", ui->lineTelefono->text().trimmed());
    query.bindValue(":correo", ui->lineCorreo->text().trimmed());
    query.bindValue(":estado", ui->comboEstado->currentText());

    if (!query.exec()) {
        QMessageBox::critical(this, "Error", "No se pudo guardar el empleado:\n" + query.lastError().text());
        return;
    }

    accept();
}
