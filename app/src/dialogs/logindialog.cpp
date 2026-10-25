#include "logindialog.h"
#include "ui_logindialog.h"
#include "registrodialog.h"
#include "../database/sesionactual.h"
#include "../util/passwordutil.h"
#include "../util/iconos.h"

#include <QIcon>
#include <QSqlQuery>

LoginDialog::LoginDialog(QWidget *parent) : QDialog(parent), ui(new Ui::LoginDialog) {
    ui->setupUi(this);
    connect(ui->btnIngresar, &QPushButton::clicked, this, &LoginDialog::ingresar);
    connect(ui->btnRegistrar, &QPushButton::clicked, this, &LoginDialog::abrirRegistro);

    // Logo (proporción 667x150 del SVG), renderizado al ancho útil del diálogo.
    const int anchoLogo = 420;
    ui->labelTitulo->setPixmap(QIcon(":/logo.svg").pixmap(QSize(anchoLogo, anchoLogo * 150 / 667)));

    ui->btnIngresar->setIcon(Iconos::ingresar());
    ui->btnRegistrar->setIcon(Iconos::registrarCuenta());

    ui->lineContrasena->setFocus();
}

LoginDialog::~LoginDialog() {
    delete ui;
}

void LoginDialog::ingresar() {
    QString usuario = ui->lineUsuario->text().trimmed();
    QString contrasena = ui->lineContrasena->text();

    if (usuario.isEmpty() || contrasena.isEmpty()) {
        ui->labelError->setText("Escribe usuario y contraseña.");
        return;
    }

    QSqlQuery query;
    query.prepare(
        "SELECT u.id_usuario, u.contrasena_hash, u.sal, u.rol, u.estado, u.id_empleado, "
        "e.id_sucursal, s.nombre "
        "FROM Usuarios u "
        "LEFT JOIN Empleados e ON e.id_empleado = u.id_empleado "
        "LEFT JOIN Sucursales s ON s.id_sucursal = e.id_sucursal "
        "WHERE u.nombre_usuario = :usuario");
    query.bindValue(":usuario", usuario);
    query.exec();

    if (!query.next()) {
        ui->labelError->setText("Usuario o contraseña incorrectos.");
        return;
    }

    QString hashGuardado = query.value(1).toString();
    QString sal = query.value(2).toString();
    QString estado = query.value(4).toString();

    if (!PasswordUtil::verificar(contrasena, sal, hashGuardado)) {
        ui->labelError->setText("Usuario o contraseña incorrectos.");
        return;
    }
    if (estado != "activo") {
        ui->labelError->setText("Esta cuenta está inactiva. Contacta a un administrador.");
        return;
    }

    int idUsuario = query.value(0).toInt();
    QString rol = query.value(3).toString();
    int idEmpleado = query.value(5).isNull() ? -1 : query.value(5).toInt();
    int idSucursal = query.value(6).isNull() ? -1 : query.value(6).toInt();
    QString nombreSucursal = query.value(7).toString();

    SesionActual::instance().iniciar(idUsuario, usuario, rol, idEmpleado, idSucursal, nombreSucursal);
    accept();
}

void LoginDialog::abrirRegistro() {
    RegistroDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        ui->lineUsuario->setText(dialog.nombreUsuarioRegistrado());
        ui->lineContrasena->clear();
        ui->lineContrasena->setFocus();
        ui->labelError->setStyleSheet("color: #2e7d32;");
        ui->labelError->setText("Cuenta creada. Ya puedes iniciar sesión.");
    }
}
