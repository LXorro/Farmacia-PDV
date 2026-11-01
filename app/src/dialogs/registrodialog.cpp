#include "registrodialog.h"
#include "ui_registrodialog.h"
#include "../util/passwordutil.h"

#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

RegistroDialog::RegistroDialog(QWidget *parent) : QDialog(parent), ui(new Ui::RegistroDialog) {
    ui->setupUi(this);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &RegistroDialog::guardar);
    // El administrador inicial no está ligado a un empleado (aún no existe ninguno).
    ui->labelEmpleado->setVisible(false);
    ui->comboEmpleado->setVisible(false);
}

RegistroDialog::~RegistroDialog() {
    delete ui;
}

void RegistroDialog::guardar() {
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

    // Solo vale mientras no exista ninguna cuenta; después las crea un administrador.
    QSqlQuery conteo("SELECT COUNT(*) FROM Usuarios");
    conteo.next();
    if (conteo.value(0).toInt() != 0) {
        QMessageBox::warning(this, "Registro cerrado",
                              "Ya existen usuarios. Pide a un administrador que cree tu cuenta.");
        reject();
        return;
    }

    QString sal = PasswordUtil::generarSal();
    QString hash = PasswordUtil::hashear(contrasena, sal);

    QSqlQuery insertar;
    insertar.prepare(
        "INSERT INTO Usuarios (nombre_usuario, contrasena_hash, sal, rol) "
        "VALUES (:usuario, :hash, :sal, 'admin')");
    insertar.bindValue(":usuario", usuario);
    insertar.bindValue(":hash", hash);
    insertar.bindValue(":sal", sal);

    if (!insertar.exec()) {
        QMessageBox::critical(this, "Error", "No se pudo registrar el usuario (¿el nombre de usuario ya existe?):\n" + insertar.lastError().text());
        return;
    }

    m_nombreUsuarioRegistrado = usuario;
    accept();
}
