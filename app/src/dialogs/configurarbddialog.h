#pragma once

#include <QDialog>

namespace Ui {
class ConfigurarBdDialog;
}

// Asistente de primer arranque: prepara el MariaDB local con una cuenta administradora.
class ConfigurarBdDialog : public QDialog {
    Q_OBJECT
public:
    explicit ConfigurarBdDialog(QWidget *parent = nullptr);
    ~ConfigurarBdDialog();

private slots:
    void configurar();
    void actualizarCentral();
    void consultarSucursales();

private:
    bool datosCentralValidos();
    void error(const QString &texto);
    void mostrarInstrucciones(const QString &texto);
    Ui::ConfigurarBdDialog *ui;
};
