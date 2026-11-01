#include "logindialog.h"
#include "ui_logindialog.h"
#include "registrodialog.h"
#include "../database/databasemanager.h"
#include "../database/sesionactual.h"
#include "../util/passwordutil.h"
#include "../util/iconos.h"

#include <QIcon>
#include <QInputDialog>
#include <QMessageBox>
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

    // El registro solo existe para crear el administrador inicial. Una sucursal nunca crea
    // cuentas aquí: las recibe del central por replicación.
    QSqlQuery conteo("SELECT COUNT(*) FROM Usuarios");
    const bool sinUsuarios = conteo.next() && conteo.value(0).toInt() == 0;
    const bool sucursal = DatabaseManager::esSucursal();
    ui->btnRegistrar->setText("Crear administrador inicial");
    ui->btnRegistrar->setVisible(sinUsuarios && !sucursal);
    if (sinUsuarios && sucursal)
        ui->labelError->setText("Aún no se han recibido usuarios del central. Verifica la conexión y vuelve a intentar.");

    ui->lineUsuario->setFocus();
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

    // Reglas según la función de este equipo (ver DatabaseManager::Funcion).
    QString motivo;
    if (!validarEquipo(rol, idEmpleado, idSucursal, nombreSucursal, motivo)) {
        ui->labelError->setText(motivo);
        return;
    }

    SesionActual::instance().iniciar(idUsuario, usuario, rol, idEmpleado, idSucursal, nombreSucursal);
    accept();
}

// Decide si este usuario puede operar en este equipo y, en una sucursal, fija la sucursal de la sesión:
// la del equipo (un empleado de otra sucursal solo entra con una asignación temporal vigente, es decir,
// estando físicamente aquí).
bool LoginDialog::validarEquipo(const QString &rol, int idEmpleado, int &idSucursal, QString &nombreSucursal,
                                QString &motivo) {
    const auto funcion = DatabaseManager::funcion();
    if (funcion == DatabaseManager::Funcion::Unico)
        return true;

    if (funcion == DatabaseManager::Funcion::Central) {
        if (rol == "admin")
            return true;
        motivo = "Este equipo es de administración: solo los administradores pueden entrar aquí.";
        return false;
    }

    // Sucursal
    int idNodo = DatabaseManager::idSucursalNodo();
    if (idNodo < 0) {
        if (rol != "admin") {
            motivo = "Este equipo aún no tiene sucursal asignada. Un administrador debe iniciar sesión primero.";
            return false;
        }
        QSqlQuery sucursales("SELECT id_sucursal, nombre FROM Sucursales WHERE rol = 'sucursal' AND estado = 'activa' ORDER BY nombre");
        QStringList nombres;
        QList<int> ids;
        while (sucursales.next()) {
            ids << sucursales.value(0).toInt();
            nombres << sucursales.value(1).toString();
        }
        if (ids.isEmpty()) {
            motivo = "Aún no se han recibido sucursales del central. Verifica la conexión y vuelve a intentar.";
            return false;
        }
        bool aceptado = false;
        const QString elegida = QInputDialog::getItem(this, "Sucursal de este equipo",
                                                      "¿A qué sucursal pertenece este equipo? No podrá cambiarse después.",
                                                      nombres, 0, false, &aceptado);
        if (!aceptado) {
            motivo = "Hay que elegir la sucursal de este equipo para continuar.";
            return false;
        }
        idNodo = ids.at(nombres.indexOf(elegida));
        DatabaseManager::instance().guardarSucursalNodo(idNodo);
    }

    if (rol != "admin" && idSucursal != idNodo) {
        QSqlQuery asignacion;
        asignacion.prepare("SELECT 1 FROM Asignaciones_Temporales WHERE id_empleado = :emp AND id_sucursal = :suc "
                           "AND estado = 'aprobada' AND CURDATE() BETWEEN fecha_inicio AND fecha_fin LIMIT 1");
        asignacion.bindValue(":emp", idEmpleado);
        asignacion.bindValue(":suc", idNodo);
        if (!asignacion.exec() || !asignacion.next()) {
            motivo = "No estás asignado a esta sucursal. Pide a un administrador que autorice una asignación temporal.";
            return false;
        }
    }

    QSqlQuery nombre;
    nombre.prepare("SELECT nombre FROM Sucursales WHERE id_sucursal = :id");
    nombre.bindValue(":id", idNodo);
    if (nombre.exec() && nombre.next())
        nombreSucursal = nombre.value(0).toString();
    idSucursal = idNodo;
    return true;
}

void LoginDialog::abrirRegistro() {
    RegistroDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        ui->lineUsuario->setText(dialog.nombreUsuarioRegistrado());
        ui->lineContrasena->clear();
        ui->lineContrasena->setFocus();
        ui->labelError->setStyleSheet("color: #2e7d32;");
        ui->labelError->setText("Administrador creado. Ya puedes iniciar sesión.");
        ui->btnRegistrar->setVisible(false);
    }
}
