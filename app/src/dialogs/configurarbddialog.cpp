#include "configurarbddialog.h"
#include "ui_configurarbddialog.h"
#include "../database/databasemanager.h"
#include "../database/dbconfig.h"
#include "../database/dbinicializador.h"

#include <QApplication>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QVBoxLayout>

ConfigurarBdDialog::ConfigurarBdDialog(QWidget *parent) : QDialog(parent), ui(new Ui::ConfigurarBdDialog) {
    ui->setupUi(this);
    ui->spinPuerto->setValue(DbConfig::kPort);
    ui->spinPuertoCentral->setValue(DbConfig::kPort);
    connect(ui->btnConfigurar, &QPushButton::clicked, this, &ConfigurarBdDialog::configurar);
    connect(ui->btnCancelar, &QPushButton::clicked, this, &QDialog::reject);
    connect(ui->btnConsultar, &QPushButton::clicked, this, &ConfigurarBdDialog::consultarSucursales);
    connect(ui->checkCentral, &QCheckBox::toggled, this, &ConfigurarBdDialog::actualizarCentral);
    actualizarCentral();
    ui->lineContrasenaAdmin->setFocus();
}

ConfigurarBdDialog::~ConfigurarBdDialog() {
    delete ui;
}

void ConfigurarBdDialog::actualizarCentral() {
    const bool conCentral = ui->checkCentral->isChecked();
    for (QWidget *w : {static_cast<QWidget *>(ui->labelHostCentral), static_cast<QWidget *>(ui->lineHostCentral),
                       static_cast<QWidget *>(ui->labelPuertoCentral), static_cast<QWidget *>(ui->spinPuertoCentral),
                       static_cast<QWidget *>(ui->labelUsuarioRegistro), static_cast<QWidget *>(ui->lineUsuarioRegistro),
                       static_cast<QWidget *>(ui->labelContrasenaRegistro), static_cast<QWidget *>(ui->lineContrasenaRegistro),
                       static_cast<QWidget *>(ui->labelSucursal), static_cast<QWidget *>(ui->comboSucursal),
                       static_cast<QWidget *>(ui->btnConsultar)})
        w->setVisible(conCentral);
    ui->labelAyudaFuncion->setText(
        conCentral
            ? "Los empleados trabajan sobre la base de este equipo, que se sincroniza con el central. Los "
              "administradores, al iniciar sesión, se conectan al central y ven todas las sucursales."
            : "Una sola base de datos local, sin conexión con otros equipos.");
}

void ConfigurarBdDialog::error(const QString &texto) {
    ui->labelError->setStyleSheet("color: #b00020;");
    ui->labelError->setText(texto);
}

static DbInicializador::Central centralDesdeFormulario(Ui::ConfigurarBdDialog *ui) {
    DbInicializador::Central central;
    central.host = ui->lineHostCentral->text().trimmed();
    central.puerto = ui->spinPuertoCentral->value();
    central.usuario = ui->lineUsuarioRegistro->text().trimmed();
    central.contrasena = ui->lineContrasenaRegistro->text();
    return central;
}

bool ConfigurarBdDialog::datosCentralValidos() {
    if (ui->lineHostCentral->text().trimmed().isEmpty()) {
        error("Indica la dirección del central.");
        return false;
    }
    if (ui->lineUsuarioRegistro->text().trimmed().isEmpty() || ui->lineContrasenaRegistro->text().isEmpty()) {
        error("Indica la cuenta y la contraseña de registro del central.");
        return false;
    }
    return true;
}

void ConfigurarBdDialog::consultarSucursales() {
    if (!datosCentralValidos())
        return;
    ui->labelError->setStyleSheet("color: #555555;");
    ui->labelError->setText("Consultando el central…");
    QApplication::processEvents();

    QList<DbInicializador::SucursalInfo> sucursales;
    QString mensaje;
    if (!DbInicializador::listarSucursales(centralDesdeFormulario(ui), sucursales, mensaje)) {
        error("No se pudo consultar el central:\n" + mensaje);
        return;
    }
    ui->comboSucursal->clear();
    for (const auto &sucursal : sucursales)
        ui->comboSucursal->addItem(sucursal.nombre, sucursal.id);
    if (sucursales.isEmpty()) {
        error("El central no tiene sucursales. Un administrador debe darlas de alta primero.");
        return;
    }
    ui->labelError->clear();
}

void ConfigurarBdDialog::configurar() {
    const bool conCentral = ui->checkCentral->isChecked();
    if (conCentral) {
        if (!datosCentralValidos())
            return;
        if (ui->comboSucursal->count() == 0)
            consultarSucursales();
        if (ui->comboSucursal->count() == 0)
            return;
    }
    const int idSucursal = conCentral ? ui->comboSucursal->currentData().toInt() : -1;
    const DbInicializador::Central central = centralDesdeFormulario(ui);
    const QString usuarioAdmin = ui->lineUsuarioAdmin->text().trimmed();
    const QString contrasenaAdmin = ui->lineContrasenaAdmin->text();
    const int puerto = ui->spinPuerto->value();

    ui->labelError->clear();
    ui->btnConfigurar->setEnabled(false);
    ui->labelError->setStyleSheet("color: #555555;");
    auto avance = [this](const QString &texto) {
        ui->labelError->setText(texto);
        QApplication::processEvents();
    };

    if (conCentral) {
        avance("Comprobando la configuración de MariaDB de este equipo…");
        QString instrucciones;
        if (!DbInicializador::verificarNodoLocal(DbConfig::kHost, puerto, usuarioAdmin, contrasenaAdmin,
                                                 DbInicializador::ranuraDeSucursal(idSucursal), instrucciones)) {
            ui->btnConfigurar->setEnabled(true);
            ui->labelError->clear();
            mostrarInstrucciones(instrucciones);
            return;
        }
    }

    avance("Preparando la base de datos local…");
    const auto resultado = DbInicializador::inicializar(
        DbConfig::kHost, puerto, usuarioAdmin, contrasenaAdmin,
        conCentral ? DbInicializador::Rol::Sucursal : DbInicializador::Rol::Unico);
    if (!resultado.ok) {
        ui->btnConfigurar->setEnabled(true);
        error("No se pudo configurar:\n" + resultado.error);
        return;
    }

    if (conCentral) {
        avance("Configurando la replicación con el central…");
        QString mensaje;
        if (!DbInicializador::enlazarConCentral(DbConfig::kHost, puerto, usuarioAdmin, contrasenaAdmin, central,
                                                idSucursal, mensaje)) {
            ui->btnConfigurar->setEnabled(true);
            error("No se pudo enlazar con el central:\n" + mensaje);
            return;
        }
    }

    // Las contraseñas de administración y de registro no se conservan en ningún sitio.
    ui->lineContrasenaAdmin->clear();
    ui->lineContrasenaRegistro->clear();
    ui->btnConfigurar->setEnabled(true);

    auto &gestor = DatabaseManager::instance();
    gestor.guardarCredenciales(DbConfig::kHost, puerto, resultado.usuario, resultado.contrasena);
    if (conCentral) {
        gestor.guardarCentral(central.host, central.puerto);
        gestor.guardarSucursalNodo(idSucursal);
    } else {
        gestor.guardarCentral(QString(), 0);
        gestor.guardarSucursalNodo(-1);
    }
    accept();
}

void ConfigurarBdDialog::mostrarInstrucciones(const QString &texto) {
    QDialog dialogo(this);
    dialogo.setWindowTitle("Falta preparar MariaDB");
    dialogo.resize(640, 460);
    auto *capa = new QVBoxLayout(&dialogo);
    auto *editor = new QPlainTextEdit(texto, &dialogo);
    editor->setReadOnly(true);
    capa->addWidget(editor);
    auto *cerrar = new QPushButton("Entendido", &dialogo);
    connect(cerrar, &QPushButton::clicked, &dialogo, &QDialog::accept);
    capa->addWidget(cerrar);
    dialogo.exec();
}
