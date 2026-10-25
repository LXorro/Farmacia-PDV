#pragma once

#include <QDialog>

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog {
    Q_OBJECT
public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();

private slots:
    void ingresar();
    void abrirRegistro();

private:
    Ui::LoginDialog *ui;
};
