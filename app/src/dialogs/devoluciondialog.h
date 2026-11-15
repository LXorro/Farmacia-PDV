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
    void actualizarTipo();

private:
    void cargarEmpleados(int idSucursal);
    void cargarPagosDelTicket();

    int m_idVenta = -1;
    int m_idTicket = -1;
    int m_cantidadMaxima = 0;
    double m_precioUnitario = 0;
    // Cómo se pagó el ticket (para devolver el dinero por la misma vía): lo que aún puede reembolsarse.
    double m_tarjetaPagada = 0;
    double m_tarjetaDisponible = 0;
    double m_otrosDisponible = 0;   // efectivo/transferencia pagados y aún no devueltos
    bool m_pagoTarjetaHoy = false;
    Ui::DevolucionDialog *ui;
};
