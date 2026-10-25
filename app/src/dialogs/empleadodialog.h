#pragma once

#include <QDialog>

namespace Ui {
class EmpleadoDialog;
}

class EmpleadoDialog : public QDialog {
    Q_OBJECT
public:
    explicit EmpleadoDialog(QWidget *parent = nullptr);
    ~EmpleadoDialog();

    void setEmpleadoId(int id);

private slots:
    void guardar();

private:
    void cargarCombos();
    void cargarEmpleado();

    int m_idEmpleado = -1;
    Ui::EmpleadoDialog *ui;
};
