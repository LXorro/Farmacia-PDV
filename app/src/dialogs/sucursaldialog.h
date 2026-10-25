#pragma once

#include <QDialog>

namespace Ui {
class SucursalDialog;
}

class SucursalDialog : public QDialog {
    Q_OBJECT
public:
    explicit SucursalDialog(QWidget *parent = nullptr);
    ~SucursalDialog();

    void setSucursalId(int id);

private slots:
    void guardar();

private:
    void cargarSucursal();

    int m_idSucursal = -1;
    Ui::SucursalDialog *ui;
};
