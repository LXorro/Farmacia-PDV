#pragma once

#include <QDialog>

namespace Ui {
class ProveedorDialog;
}

class ProveedorDialog : public QDialog {
    Q_OBJECT
public:
    explicit ProveedorDialog(QWidget *parent = nullptr);
    ~ProveedorDialog();

    void setProveedorId(int id);

private slots:
    void guardar();

private:
    void cargarProveedor();

    int m_idProveedor = -1;
    Ui::ProveedorDialog *ui;
};
