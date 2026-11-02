#pragma once

#include <QDialog>

class QCheckBox;
class QDoubleSpinBox;
class QSpinBox;

// Lo que una sucursal decide sobre un producto del catálogo: su precio de venta, si lo maneja y su
// mínimo de existencias. Escribe Productos_Sucursal (la tabla es de la sucursal; el catálogo, del central).
class ProductoSucursalDialog : public QDialog {
    Q_OBJECT
public:
    ProductoSucursalDialog(int idProducto, int idSucursal, const QString &nombreProducto, double precioSugerido,
                           QWidget *parent = nullptr);

private slots:
    void guardar();

private:
    int m_idProducto;
    int m_idSucursal;
    QDoubleSpinBox *m_precio;
    QCheckBox *m_activo;
    QSpinBox *m_minimo;
};
