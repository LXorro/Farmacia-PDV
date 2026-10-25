#include "usuariodialog.h"
#include "ui_usuariodialog.h"
#include "../util/passwordutil.h"

#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

UsuarioDialog::UsuarioDialog(QWidget *parent) : QDialog(parent), ui(new Ui::UsuarioDialog) {
    ui->setupUi(this);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &UsuarioDialog::guardar);
    cargarCombos();
}

UsuarioDialog::~UsuarioDialog() {
    delete ui;
}

void UsuarioDialog::cargarCombos() {
    ui->comboEmpleado->clear();
    QSqlQuery query;
    query.prepare(
        "SELECT e.id_empleado, e.nombre, e.apellido_paterno, s.nombre "
        "FROM Empleados e "
        "JOIN Sucursales s ON s.id_sucursal = e.id_sucursal "
        "LEFT JOIN Usuarios u ON u.id_empleado = e.id_empleado "
        "WHERE u.id_usuario IS NULL OR e.id_empleado = :actual "
        "ORDER BY e.nombre");
    query.bindValue(":actual", m_idEmpleadoActual);
    query.exec();
    while (query.next()) {
        QString etiqueta = QString("%1 %2 (%3)").arg(query.value(1).toString(), query.value(2).toString(), query.value(3).toString());
        ui->comboEmpleado->addItem(etiqueta, query.value(0));
    }
}

void UsuarioDialog::setUsuarioId(int id) {
    m_idUsuario = id;
    cargarUsuario();
}

void UsuarioDialog::cargarUsuario() {
    if (m_idUsuario < 0)
        return;

    QSqlQuery query;
    query.prepare("SELECT id_empleado, nombre_usuario, rol, estado FROM Usuarios WHERE id_usuario = :id");
    query.bindValue(":id", m_idUsuario);
    if (!query.exec() || !query.next())
        return;

    m_idEmpleadoActual = query.value(0).toInt();
    cargarCombos();

    ui->comboEmpleado->setCurrentIndex(ui->comboEmpleado->findData(m_idEmpleadoActual));
    ui->lineUsuario->setText(query.value(1).toString());
    ui->comboRol->setCurrentText(query.value(2).toString());
    ui->comboEstado->setCurrentText(query.value(3).toString());
}

void UsuarioDialog::guardar() {
    if (ui->comboEmpleado->currentIndex() < 0) {
        QMessageBox::warning(this, "Validación", "No hay empleados disponibles para ligar a una cuenta.");
        return;
    }
    QString usuario = ui->lineUsuario->text().trimmed();
    if (usuario.isEmpty()) {
        QMessageBox::warning(this, "Validación", "El nombre de usuario es obligatorio.");
        return;
    }
    QString contrasena = ui->lineContrasena->text();
    if (m_idUsuario < 0 && contrasena.isEmpty()) {
        QMessageBox::warning(this, "Validación", "La contraseña es obligatoria para un usuario nuevo.");
        return;
    }

    QSqlQuery query;
    if (m_idUsuario < 0) {
        QString sal = PasswordUtil::generarSal();
        QString hash = PasswordUtil::hashear(contrasena, sal);
        query.prepare(
            "INSERT INTO Usuarios (id_empleado, nombre_usuario, contrasena_hash, sal, rol, estado) "
            "VALUES (:empleado, :usuario, :hash, :sal, :rol, :estado)");
        query.bindValue(":hash", hash);
        query.bindValue(":sal", sal);
    } else {
        if (!contrasena.isEmpty()) {
            QString sal = PasswordUtil::generarSal();
            QString hash = PasswordUtil::hashear(contrasena, sal);
            query.prepare(
                "UPDATE Usuarios SET id_empleado = :empleado, nombre_usuario = :usuario, rol = :rol, "
                "estado = :estado, contrasena_hash = :hash, sal = :sal WHERE id_usuario = :id");
            query.bindValue(":hash", hash);
            query.bindValue(":sal", sal);
        } else {
            query.prepare(
                "UPDATE Usuarios SET id_empleado = :empleado, nombre_usuario = :usuario, rol = :rol, "
                "estado = :estado WHERE id_usuario = :id");
        }
        query.bindValue(":id", m_idUsuario);
    }

    query.bindValue(":empleado", ui->comboEmpleado->currentData());
    query.bindValue(":usuario", usuario);
    query.bindValue(":rol", ui->comboRol->currentText());
    query.bindValue(":estado", ui->comboEstado->currentText());

    if (!query.exec()) {
        QMessageBox::critical(this, "Error", "No se pudo guardar el usuario (¿nombre ya existe o empleado ya tiene cuenta?):\n" + query.lastError().text());
        return;
    }

    accept();
}
