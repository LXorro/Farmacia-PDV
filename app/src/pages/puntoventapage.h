#pragma once

#include <QString>
#include <QVector>
#include <QWidget>

namespace Ui {
class PuntoVentaPage;
}

struct LineaCarrito {
    int idProducto;
    int idLote;
    QString nombreProducto;
    QString numeroLote;
    int cantidad;
    double precioUnitario;
};

class PuntoVentaPage : public QWidget {
    Q_OBJECT
public:
    explicit PuntoVentaPage(QWidget *parent = nullptr);
    ~PuntoVentaPage();

public slots:
    void refrescar();

private slots:
    void agregarAlCarrito();
    void quitarLineaSeleccionada();
    void cancelarVenta();
    void cobrar();
    void alCambiarSucursal();

private:
    void cargarSucursales();
    void cargarEmpleados(int idSucursal);
    void cargarClientes(int idSucursal);
    void refrescarTabla();
    void actualizarTotal();
    void limpiarCarrito();

    QVector<LineaCarrito> m_carrito;
    Ui::PuntoVentaPage *ui;
};
