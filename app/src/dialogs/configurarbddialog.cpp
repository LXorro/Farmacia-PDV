#include "configurarbddialog.h"
#include "ui_configurarbddialog.h"
#include "../database/databasemanager.h"
#include "../database/dbconfig.h"
#include "../database/dbinicializador.h"

#include <QApplication>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QInputDialog>
#include <QLineEdit>

ConfigurarBdDialog::ConfigurarBdDialog(QWidget *parent) : QDialog(parent), ui(new Ui::ConfigurarBdDialog) {
    ui->setupUi(this);
    ui->spinPuerto->setValue(DbConfig::kPort);
    connect(ui->btnConfigurar, &QPushButton::clicked, this, &ConfigurarBdDialog::configurar);
    connect(ui->btnCancelar, &QPushButton::clicked, this, &QDialog::reject);
    connect(ui->comboFuncion, qOverload<int>(&QComboBox::currentIndexChanged), this, &ConfigurarBdDialog::actualizarFuncion);
    connect(ui->btnCertificadoCentral, &QPushButton::clicked, this, &ConfigurarBdDialog::elegirCertificadoCentral);
    ui->spinPuertoCentral->setValue(DbConfig::kPort);
    actualizarFuncion();
    ui->lineContrasenaAdmin->setFocus();
}

ConfigurarBdDialog::~ConfigurarBdDialog() {
    delete ui;
}

// 0 = equipo único, 1 = central, 2 = sucursal.
void ConfigurarBdDialog::actualizarFuncion() {
    const int funcion = ui->comboFuncion->currentIndex();
    const bool sucursal = funcion == 2;
    ui->labelHostCentral->setVisible(sucursal);
    ui->lineHostCentral->setVisible(sucursal);
    ui->labelPuertoCentral->setVisible(sucursal);
    ui->spinPuertoCentral->setVisible(sucursal);
    ui->labelCertificadoCentral->setVisible(sucursal);
    ui->lineCertificadoCentral->setVisible(sucursal);
    ui->btnCertificadoCentral->setVisible(sucursal);

    static const char *ayuda[] = {
        "Una sola base de datos local, sin conexión con otros equipos.",
        "Aquí se administran usuarios, empleados, sucursales y catálogo. Se creará una cuenta de "
        "administración remota para que las sucursales puedan dar de alta usuarios.",
        "Recibe usuarios y catálogo del central. Para crear usuarios se conecta al central "
        "(se pedirá su contraseña de administración al hacerlo; no se guarda)."};
    ui->labelAyudaFuncion->setText(ayuda[funcion]);
}

void ConfigurarBdDialog::elegirCertificadoCentral() {
    const QString ruta = QFileDialog::getOpenFileName(this, "Certificado TLS del central", QString(),
                                                        "Certificados (*.pem *.crt *.cer);;Todos los archivos (*)");
    if (ruta.isEmpty())
        return;

    QFile archivo(ruta);
    if (!archivo.open(QIODevice::ReadOnly)) {
        ui->labelError->setStyleSheet("color: #b00020;");
        ui->labelError->setText("No se pudo leer el certificado elegido.");
        return;
    }
    m_certificadoCentralPem = QString::fromUtf8(archivo.readAll());
    ui->lineCertificadoCentral->setText(QFileInfo(ruta).fileName());
}

void ConfigurarBdDialog::configurar() {
    const int funcion = ui->comboFuncion->currentIndex();
    if (funcion == 2 && ui->lineHostCentral->text().trimmed().isEmpty()) {
        ui->labelError->setStyleSheet("color: #b00020;");
        ui->labelError->setText("Indica la dirección del central.");
        return;
    }

    ui->labelError->clear();
    ui->btnConfigurar->setEnabled(false);
    ui->labelError->setStyleSheet("color: #555555;");
    ui->labelError->setText("Configurando…");
    QApplication::processEvents();

    const auto resultado = DbInicializador::inicializar(
        DbConfig::kHost, ui->spinPuerto->value(), ui->lineUsuarioAdmin->text().trimmed(),
        ui->lineContrasenaAdmin->text(),
        funcion == 1 ? DbInicializador::Rol::Central
        : funcion == 2 ? DbInicializador::Rol::Sucursal : DbInicializador::Rol::Unico);

    // La contraseña de administrador no se conserva en ningún sitio.
    ui->lineContrasenaAdmin->clear();
    ui->btnConfigurar->setEnabled(true);

    if (!resultado.ok) {
        ui->labelError->setStyleSheet("color: #b00020;");
        ui->labelError->setText("No se pudo configurar:\n" + resultado.error);
        return;
    }

    DatabaseManager::instance().guardarCredenciales(DbConfig::kHost, ui->spinPuerto->value(), resultado.usuario,
                                                    resultado.contrasena);
    DatabaseManager::instance().guardarFuncion(
        funcion == 1 ? DatabaseManager::Funcion::Central
        : funcion == 2 ? DatabaseManager::Funcion::Sucursal : DatabaseManager::Funcion::Unico,
        funcion == 2 ? ui->lineHostCentral->text().trimmed() : QString(),
        funcion == 2 ? ui->spinPuertoCentral->value() : 0,
        funcion == 2 ? m_certificadoCentralPem : QString());

    if (funcion == 1) {
        // Se muestra una sola vez: las sucursales la piden al administrar usuarios.
        QInputDialog::getText(this, "Cuenta de administración remota",
                              "Anota esta contraseña (cuenta «farmacia_remoto»). Los administradores de las "
                              "sucursales la necesitarán para crear usuarios. No se volverá a mostrar.",
                              QLineEdit::Normal, resultado.contrasenaRemota);
    }
    accept();
}
