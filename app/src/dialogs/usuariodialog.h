#pragma once

#include <QDialog>

namespace Ui {
class UsuarioDialog;
}

class UsuarioDialog : public QDialog {
    Q_OBJECT
public:
    explicit UsuarioDialog(QWidget *parent = nullptr);
    ~UsuarioDialog();

    void setUsuarioId(int id);

private slots:
    void guardar();

private:
    void cargarCombos();
    void cargarUsuario();

    int m_idUsuario = -1;
    int m_idEmpleadoActual = -1;
    Ui::UsuarioDialog *ui;
};
