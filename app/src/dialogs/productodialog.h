#pragma once

#include <QDialog>

namespace Ui {
class ProductoDialog;
}

class ProductoDialog : public QDialog {
    Q_OBJECT
public:
    explicit ProductoDialog(QWidget *parent = nullptr);
    ~ProductoDialog();

    void setProductoId(int id);

private slots:
    void guardar();

private:
    void cargarCombos();
    void cargarProducto();

    int m_idProducto = -1;
    Ui::ProductoDialog *ui;
};
