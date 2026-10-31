#include "configurarbddialog.h"
#include "ui_configurarbddialog.h"
#include "../database/databasemanager.h"
#include "../database/dbconfig.h"
#include "../database/dbinicializador.h"

#include <QApplication>

ConfigurarBdDialog::ConfigurarBdDialog(QWidget *parent) : QDialog(parent), ui(new Ui::ConfigurarBdDialog) {
    ui->setupUi(this);
    ui->spinPuerto->setValue(DbConfig::kPort);
    connect(ui->btnConfigurar, &QPushButton::clicked, this, &ConfigurarBdDialog::configurar);
    connect(ui->btnCancelar, &QPushButton::clicked, this, &QDialog::reject);
    ui->lineContrasenaAdmin->setFocus();
}

ConfigurarBdDialog::~ConfigurarBdDialog() {
    delete ui;
}

void ConfigurarBdDialog::configurar() {
    ui->labelError->clear();
    ui->btnConfigurar->setEnabled(false);
    ui->labelError->setStyleSheet("color: #555555;");
    ui->labelError->setText("Configurando…");
    QApplication::processEvents();

    const auto resultado = DbInicializador::inicializar(
        DbConfig::kHost, ui->spinPuerto->value(), ui->lineUsuarioAdmin->text().trimmed(),
        ui->lineContrasenaAdmin->text());

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
    accept();
}
