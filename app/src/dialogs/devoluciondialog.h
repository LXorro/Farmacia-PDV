#pragma once

#include <QDialog>
#include <QString>

namespace Ui {
class DevolucionDialog;
}

class DevolucionDialog : public QDialog {
    Q_OBJECT
public:
    explicit DevolucionDialog(QWidget *parent = nullptr);
    ~DevolucionDialog();

    // Datos de la línea de venta sobre la que se registra la devolución.
    void setVenta(int idVenta, int idTicket, int idSucursal, const QString &nombreProducto,
                  int cantidadDisponibleParaDevolver, double precioUnitario);

private slots:
    void guardar();

private:
    void cargarEmpleados(int idSucursal);

    int m_idVenta = -1;
    int m_idTicket = -1;
    int m_cantidadMaxima = 0;
    double m_precioUnitario = 0;
    Ui::DevolucionDialog *ui;
};
