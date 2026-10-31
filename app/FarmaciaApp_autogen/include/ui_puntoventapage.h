/********************************************************************************
** Form generated from reading UI file 'puntoventapage.ui'
**
** Created by: Qt User Interface Compiler version 6.12.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PUNTOVENTAPAGE_H
#define UI_PUNTOVENTAPAGE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_PuntoVentaPage
{
public:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *cabeceraLayout;
    QLabel *labelSucursal;
    QComboBox *comboSucursal;
    QLabel *labelEmpleado;
    QComboBox *comboEmpleado;
    QLabel *labelCliente;
    QComboBox *comboCliente;
    QHBoxLayout *busquedaLayout;
    QLineEdit *lineBuscarProducto;
    QSpinBox *spinCantidad;
    QPushButton *btnAgregar;
    QTableWidget *tableCarrito;
    QHBoxLayout *piePagoLayout;
    QPushButton *btnQuitarLinea;
    QSpacerItem *spacer;
    QLabel *labelModalidad;
    QComboBox *comboModalidadPago;
    QLabel *labelMetodoPago;
    QComboBox *comboMetodoPago;
    QHBoxLayout *totalLayout;
    QSpacerItem *spacerTotal;
    QLabel *labelTotalTitulo;
    QLabel *labelTotal;
    QHBoxLayout *accionesLayout;
    QSpacerItem *spacerAcciones;
    QPushButton *btnCancelarVenta;
    QPushButton *btnCobrar;

    void setupUi(QWidget *PuntoVentaPage)
    {
        if (PuntoVentaPage->objectName().isEmpty())
            PuntoVentaPage->setObjectName("PuntoVentaPage");
        PuntoVentaPage->resize(760, 520);
        verticalLayout = new QVBoxLayout(PuntoVentaPage);
        verticalLayout->setObjectName("verticalLayout");
        cabeceraLayout = new QHBoxLayout();
        cabeceraLayout->setObjectName("cabeceraLayout");
        labelSucursal = new QLabel(PuntoVentaPage);
        labelSucursal->setObjectName("labelSucursal");

        cabeceraLayout->addWidget(labelSucursal);

        comboSucursal = new QComboBox(PuntoVentaPage);
        comboSucursal->setObjectName("comboSucursal");

        cabeceraLayout->addWidget(comboSucursal);

        labelEmpleado = new QLabel(PuntoVentaPage);
        labelEmpleado->setObjectName("labelEmpleado");

        cabeceraLayout->addWidget(labelEmpleado);

        comboEmpleado = new QComboBox(PuntoVentaPage);
        comboEmpleado->setObjectName("comboEmpleado");

        cabeceraLayout->addWidget(comboEmpleado);

        labelCliente = new QLabel(PuntoVentaPage);
        labelCliente->setObjectName("labelCliente");

        cabeceraLayout->addWidget(labelCliente);

        comboCliente = new QComboBox(PuntoVentaPage);
        comboCliente->setObjectName("comboCliente");

        cabeceraLayout->addWidget(comboCliente);


        verticalLayout->addLayout(cabeceraLayout);

        busquedaLayout = new QHBoxLayout();
        busquedaLayout->setObjectName("busquedaLayout");
        lineBuscarProducto = new QLineEdit(PuntoVentaPage);
        lineBuscarProducto->setObjectName("lineBuscarProducto");

        busquedaLayout->addWidget(lineBuscarProducto);

        spinCantidad = new QSpinBox(PuntoVentaPage);
        spinCantidad->setObjectName("spinCantidad");
        spinCantidad->setMinimum(1);
        spinCantidad->setMaximum(9999);
        spinCantidad->setValue(1);

        busquedaLayout->addWidget(spinCantidad);

        btnAgregar = new QPushButton(PuntoVentaPage);
        btnAgregar->setObjectName("btnAgregar");

        busquedaLayout->addWidget(btnAgregar);


        verticalLayout->addLayout(busquedaLayout);

        tableCarrito = new QTableWidget(PuntoVentaPage);
        if (tableCarrito->columnCount() < 5)
            tableCarrito->setColumnCount(5);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        tableCarrito->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        tableCarrito->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        tableCarrito->setHorizontalHeaderItem(2, __qtablewidgetitem2);
        QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
        tableCarrito->setHorizontalHeaderItem(3, __qtablewidgetitem3);
        QTableWidgetItem *__qtablewidgetitem4 = new QTableWidgetItem();
        tableCarrito->setHorizontalHeaderItem(4, __qtablewidgetitem4);
        tableCarrito->setObjectName("tableCarrito");

        verticalLayout->addWidget(tableCarrito);

        piePagoLayout = new QHBoxLayout();
        piePagoLayout->setObjectName("piePagoLayout");
        btnQuitarLinea = new QPushButton(PuntoVentaPage);
        btnQuitarLinea->setObjectName("btnQuitarLinea");

        piePagoLayout->addWidget(btnQuitarLinea);

        spacer = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        piePagoLayout->addItem(spacer);

        labelModalidad = new QLabel(PuntoVentaPage);
        labelModalidad->setObjectName("labelModalidad");

        piePagoLayout->addWidget(labelModalidad);

        comboModalidadPago = new QComboBox(PuntoVentaPage);
        comboModalidadPago->addItem(QString());
        comboModalidadPago->addItem(QString());
        comboModalidadPago->setObjectName("comboModalidadPago");

        piePagoLayout->addWidget(comboModalidadPago);

        labelMetodoPago = new QLabel(PuntoVentaPage);
        labelMetodoPago->setObjectName("labelMetodoPago");

        piePagoLayout->addWidget(labelMetodoPago);

        comboMetodoPago = new QComboBox(PuntoVentaPage);
        comboMetodoPago->addItem(QString());
        comboMetodoPago->addItem(QString());
        comboMetodoPago->addItem(QString());
        comboMetodoPago->setObjectName("comboMetodoPago");

        piePagoLayout->addWidget(comboMetodoPago);


        verticalLayout->addLayout(piePagoLayout);

        totalLayout = new QHBoxLayout();
        totalLayout->setObjectName("totalLayout");
        spacerTotal = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        totalLayout->addItem(spacerTotal);

        labelTotalTitulo = new QLabel(PuntoVentaPage);
        labelTotalTitulo->setObjectName("labelTotalTitulo");

        totalLayout->addWidget(labelTotalTitulo);

        labelTotal = new QLabel(PuntoVentaPage);
        labelTotal->setObjectName("labelTotal");

        totalLayout->addWidget(labelTotal);


        verticalLayout->addLayout(totalLayout);

        accionesLayout = new QHBoxLayout();
        accionesLayout->setObjectName("accionesLayout");
        spacerAcciones = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        accionesLayout->addItem(spacerAcciones);

        btnCancelarVenta = new QPushButton(PuntoVentaPage);
        btnCancelarVenta->setObjectName("btnCancelarVenta");

        accionesLayout->addWidget(btnCancelarVenta);

        btnCobrar = new QPushButton(PuntoVentaPage);
        btnCobrar->setObjectName("btnCobrar");

        accionesLayout->addWidget(btnCobrar);


        verticalLayout->addLayout(accionesLayout);


        retranslateUi(PuntoVentaPage);

        QMetaObject::connectSlotsByName(PuntoVentaPage);
    } // setupUi

    void retranslateUi(QWidget *PuntoVentaPage)
    {
        labelSucursal->setText(QCoreApplication::translate("PuntoVentaPage", "Sucursal:", nullptr));
        labelEmpleado->setText(QCoreApplication::translate("PuntoVentaPage", "Empleado:", nullptr));
        labelCliente->setText(QCoreApplication::translate("PuntoVentaPage", "Cliente:", nullptr));
        lineBuscarProducto->setPlaceholderText(QCoreApplication::translate("PuntoVentaPage", "C\303\263digo de barras o nombre del producto", nullptr));
        btnAgregar->setText(QCoreApplication::translate("PuntoVentaPage", "Agregar al carrito", nullptr));
        QTableWidgetItem *___qtablewidgetitem = tableCarrito->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QCoreApplication::translate("PuntoVentaPage", "Producto", nullptr));
        QTableWidgetItem *___qtablewidgetitem1 = tableCarrito->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QCoreApplication::translate("PuntoVentaPage", "Lote", nullptr));
        QTableWidgetItem *___qtablewidgetitem2 = tableCarrito->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QCoreApplication::translate("PuntoVentaPage", "Cantidad", nullptr));
        QTableWidgetItem *___qtablewidgetitem3 = tableCarrito->horizontalHeaderItem(3);
        ___qtablewidgetitem3->setText(QCoreApplication::translate("PuntoVentaPage", "Precio unitario", nullptr));
        QTableWidgetItem *___qtablewidgetitem4 = tableCarrito->horizontalHeaderItem(4);
        ___qtablewidgetitem4->setText(QCoreApplication::translate("PuntoVentaPage", "Subtotal", nullptr));
        btnQuitarLinea->setText(QCoreApplication::translate("PuntoVentaPage", "Quitar l\303\255nea", nullptr));
        labelModalidad->setText(QCoreApplication::translate("PuntoVentaPage", "Modalidad de pago:", nullptr));
        comboModalidadPago->setItemText(0, QCoreApplication::translate("PuntoVentaPage", "contado", nullptr));
        comboModalidadPago->setItemText(1, QCoreApplication::translate("PuntoVentaPage", "diferido", nullptr));

        labelMetodoPago->setText(QCoreApplication::translate("PuntoVentaPage", "M\303\251todo de pago:", nullptr));
        comboMetodoPago->setItemText(0, QCoreApplication::translate("PuntoVentaPage", "efectivo", nullptr));
        comboMetodoPago->setItemText(1, QCoreApplication::translate("PuntoVentaPage", "tarjeta", nullptr));
        comboMetodoPago->setItemText(2, QCoreApplication::translate("PuntoVentaPage", "transferencia", nullptr));

        labelTotalTitulo->setText(QCoreApplication::translate("PuntoVentaPage", "Total:", nullptr));
        labelTotal->setText(QCoreApplication::translate("PuntoVentaPage", "$0.00", nullptr));
        btnCancelarVenta->setText(QCoreApplication::translate("PuntoVentaPage", "Cancelar venta", nullptr));
        btnCobrar->setText(QCoreApplication::translate("PuntoVentaPage", "Cobrar", nullptr));
        (void)PuntoVentaPage;
    } // retranslateUi

};

namespace Ui {
    class PuntoVentaPage: public Ui_PuntoVentaPage {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PUNTOVENTAPAGE_H
