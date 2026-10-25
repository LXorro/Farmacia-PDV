#include "registrodialog.h"
#include "ui_registrodialog.h"
#include "../util/passwordutil.h"

#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

RegistroDialog::RegistroDialog(QWidget *parent) : QDialog(parent), ui(new Ui::RegistroDialog) {
    ui->setupUi(this);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &RegistroDialog::guardar);
    cargarEmpleadosSinUsuario();
}

RegistroDialog::~RegistroDialog() {
    delete ui;
}

void RegistroDialog::cargarEmpleadosSinUsuario() {
    ui->comboEmpleado->clear();
    QSqlQuery query(
        "SELECT e.id_empleado, e.nombre, e.apellido_paterno, s.nombre "
        "FROM Empleados e "
        "JOIN Sucursales s ON s.id_sucursal = e.id_sucursal "
        "LEFT JOIN Usuarios u ON u.id_empleado = e.id_empleado "
        "WHERE u.id_usuario IS NULL AND e.estado = 'activo' "
        "ORDER BY e.nombre");
    while (query.next()) {
        QString etiqueta = QString("%1 %2 (%3)").arg(query.value(1).toString(), query.value(2).toString(), query.value(3).toString());
        ui->comboEmpleado->addItem(etiqueta, query.value(0));
    }
}

void RegistroDialog::guardar() {
    if (ui->comboEmpleado->currentIndex() < 0) {
        QMessageBox::warning(this, "Validación",
                              "No hay empleados disponibles sin cuenta. Pide a un administrador que "
                              "dé de alta tu usuario en la pantalla de Empleados/Usuarios.");
        return;
    }

    QString usuario = ui->lineUsuario->text().trimmed();
    QString contrasena = ui->lineContrasena->text();
    QString confirmar = ui->lineConfirmar->text();

    if (usuario.isEmpty() || contrasena.isEmpty()) {
        QMessageBox::warning(this, "Validación", "Usuario y contraseña son obligatorios.");
        return;
    }
    if (contrasena.length() < 4) {
        QMessageBox::warning(this, "Validación", "La contraseña debe tener al menos 4 caracteres.");
        return;
    }
    if (contrasena != confirmar) {
        QMessageBox::warning(this, "Validación", "Las contraseñas no coinciden.");
        return;
    }

    QSqlQuery conteo("SELECT COUNT(*) FROM Usuarios");
    conteo.next();
    QString rol = conteo.value(0).toInt() == 0 ? "admin" : "empleado";

    QString sal = PasswordUtil::generarSal();
    QString hash = PasswordUtil::hashear(contrasena, sal);

    QSqlQuery insertar;
    insertar.prepare(
        "INSERT INTO Usuarios (id_empleado, nombre_usuario, contrasena_hash, sal, rol) "
        "VALUES (:empleado, :usuario, :hash, :sal, :rol)");
    insertar.bindValue(":empleado", ui->comboEmpleado->currentData());
    insertar.bindValue(":usuario", usuario);
    insertar.bindValue(":hash", hash);
    insertar.bindValue(":sal", sal);
    insertar.bindValue(":rol", rol);

    if (!insertar.exec()) {
        QMessageBox::critical(this, "Error", "No se pudo registrar el usuario (¿el nombre de usuario ya existe?):\n" + insertar.lastError().text());
        return;
    }

    m_nombreUsuarioRegistrado = usuario;
    accept();
}
