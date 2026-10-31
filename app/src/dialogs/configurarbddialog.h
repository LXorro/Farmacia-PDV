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

private:
    Ui::ConfigurarBdDialog *ui;
};
