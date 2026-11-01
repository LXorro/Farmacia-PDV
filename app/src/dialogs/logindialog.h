#pragma once

#include <QDialog>
#include <QString>

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
    bool validarEquipo(const QString &rol, int idEmpleado, int &idSucursal, QString &nombreSucursal, QString &motivo);

    Ui::LoginDialog *ui;
};
