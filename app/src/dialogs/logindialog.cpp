#include "logindialog.h"
#include "ui_logindialog.h"
#include "registrodialog.h"
#include "../database/databasemanager.h"
#include "../database/sesionactual.h"
#include "../util/passwordutil.h"
#include "../util/iconos.h"

#include <QIcon>
#include <QInputDialog>
#include <QLineEdit>
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

    int idUsuario = -1, idEmpleado = -1, idSucursal = -1;
    QString rol, nombreSucursal;
    {
        // Las consultas deben haber terminado antes de poder cambiar de conexión (modo central).
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

        idUsuario = query.value(0).toInt();
        rol = query.value(3).toString();
        idEmpleado = query.value(5).isNull() ? -1 : query.value(5).toInt();
        idSucursal = query.value(6).isNull() ? -1 : query.value(6).toInt();
        nombreSucursal = query.value(7).toString();
    }

    // ¿Puede operar en este equipo? Y, de ser así, ¿en qué sucursal?
    QString motivo;
    if (!validarEquipo(rol, idEmpleado, idSucursal, nombreSucursal, motivo)) {
        ui->labelError->setText(motivo);
        return;
    }

    // Un administrador trabaja sobre el central (todas las sucursales); un empleado, sobre la base local.
    if (rol == "admin" && DatabaseManager::hayCentral() && !conectarAdministradorAlCentral())
        return;

    SesionActual::instance().iniciar(idUsuario, usuario, rol, idEmpleado, idSucursal, nombreSucursal);
    accept();
}

// Abre la conexión de administración con el central y la vuelve la conexión de la sesión. Si no se
// puede, deja elegir entre entrar en modo local (lo que el central ha replicado, sin editarlo) o cancelar.
bool LoginDialog::conectarAdministradorAlCentral() {
    auto &gestor = DatabaseManager::instance();
    bool aceptado = false;
    const QString contrasena = QInputDialog::getText(
        this, "Conexión con el central",
        "Los administradores trabajan sobre el central. Escribe la contraseña de administración remota "
        "(cuenta «farmacia_remoto»); no se guarda:", QLineEdit::Password, QString(), &aceptado);

    QString detalle;
    if (aceptado && !contrasena.isEmpty()) {
        if (gestor.conectarCentral(contrasena) && gestor.activarModoCentral())
            return true;
        detalle = "\n\n" + gestor.lastError();
    }
    const auto respuesta = QMessageBox::question(
        this, "Sin conexión con el central",
        "No se conectó con el central" + detalle + "\n\n¿Entrar en modo local? Verás la copia replicada en este "
        "equipo, pero no podrás editar el catálogo, el personal ni las cuentas, ni ver otras sucursales.");
    return respuesta == QMessageBox::Yes;
}

// Decide si este usuario puede operar en este equipo y fija la sucursal de la sesión: la del equipo
// (un empleado de otra sucursal solo entra con una asignación temporal vigente, es decir, estando
// físicamente aquí). Un administrador no necesita sucursal: trabaja sobre el central.
bool LoginDialog::validarEquipo(const QString &rol, int idEmpleado, int &idSucursal, QString &nombreSucursal,
                                QString &motivo) {
    if (!DatabaseManager::hayCentral())
        return true;   // equipo único

    const int idNodo = DatabaseManager::idSucursalNodo();
    if (idNodo < 0) {
        if (rol == "admin")
            return true;
        motivo = "Este equipo no tiene sucursal asignada. Vuelve a ejecutar el asistente de configuración.";
        return false;
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
