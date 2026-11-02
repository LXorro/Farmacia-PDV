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
    void actualizarFuncion();
    void elegirCertificadoCentral();

private:
    Ui::ConfigurarBdDialog *ui;
    // Contenido del .pem elegido (no la ruta: se guarda el texto para no depender de que el
    // archivo original siga existiendo).
    QString m_certificadoCentralPem;
};
