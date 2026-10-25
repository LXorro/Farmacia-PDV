#pragma once

#include <QDialog>
#include <QString>

namespace Ui {
class RegistroDialog;
}

class RegistroDialog : public QDialog {
    Q_OBJECT
public:
    explicit RegistroDialog(QWidget *parent = nullptr);
    ~RegistroDialog();

    QString nombreUsuarioRegistrado() const { return m_nombreUsuarioRegistrado; }

private slots:
    void guardar();

private:
    void cargarEmpleadosSinUsuario();

    QString m_nombreUsuarioRegistrado;
    Ui::RegistroDialog *ui;
};
