/********************************************************************************
** Form generated from reading UI file 'iniciopage.ui'
**
** Created by: Qt User Interface Compiler version 6.12.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_INICIOPAGE_H
#define UI_INICIOPAGE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_InicioPage
{
public:
    QVBoxLayout *verticalLayout;
    QLabel *labelLogo;
    QHBoxLayout *encabezadoLayout;
    QVBoxLayout *saludoLayout;
    QLabel *labelBienvenida;
    QLabel *labelSubtitulo;
    QSpacerItem *spacerSaludo;
    QHBoxLayout *statsLayout;
    QFrame *frameVentasHoy;
    QVBoxLayout *ventasHoyLayout;
    QLabel *labelVentasHoyTitulo;
    QLabel *labelVentasHoy;
    QFrame *framePendientes;
    QVBoxLayout *pendientesLayout;
    QLabel *labelPendientesTitulo;
    QLabel *labelPendientes;
    QFrame *frameCaducidad;
    QVBoxLayout *caducidadLayout;
    QLabel *labelCaducidadTitulo;
    QLabel *labelCaducidad;
    QLabel *labelAccesos;
    QGridLayout *gridAccesos;
    QPushButton *btnPuntoVenta;
    QPushButton *btnProductos;
    QPushButton *btnClientes;
    QPushButton *btnTickets;
    QPushButton *btnInventario;
    QPushButton *btnCategorias;
    QPushButton *btnProveedores;
    QPushButton *btnEmpleados;
    QPushButton *btnSucursales;
    QPushButton *btnUsuarios;
    QSpacerItem *spacerFinal;

    void setupUi(QWidget *InicioPage)
    {
        if (InicioPage->objectName().isEmpty())
            InicioPage->setObjectName("InicioPage");
        InicioPage->resize(760, 520);
        verticalLayout = new QVBoxLayout(InicioPage);
        verticalLayout->setObjectName("verticalLayout");
        labelLogo = new QLabel(InicioPage);
        labelLogo->setObjectName("labelLogo");
        labelLogo->setAlignment(Qt::AlignHCenter);

        verticalLayout->addWidget(labelLogo);

        encabezadoLayout = new QHBoxLayout();
        encabezadoLayout->setObjectName("encabezadoLayout");
        saludoLayout = new QVBoxLayout();
        saludoLayout->setObjectName("saludoLayout");
        labelBienvenida = new QLabel(InicioPage);
        labelBienvenida->setObjectName("labelBienvenida");

        saludoLayout->addWidget(labelBienvenida);

        labelSubtitulo = new QLabel(InicioPage);
        labelSubtitulo->setObjectName("labelSubtitulo");

        saludoLayout->addWidget(labelSubtitulo);

        spacerSaludo = new QSpacerItem(0, 0, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        saludoLayout->addItem(spacerSaludo);


        encabezadoLayout->addLayout(saludoLayout);


        verticalLayout->addLayout(encabezadoLayout);

        statsLayout = new QHBoxLayout();
        statsLayout->setObjectName("statsLayout");
        frameVentasHoy = new QFrame(InicioPage);
        frameVentasHoy->setObjectName("frameVentasHoy");
        frameVentasHoy->setFrameShape(QFrame::StyledPanel);
        ventasHoyLayout = new QVBoxLayout(frameVentasHoy);
        ventasHoyLayout->setObjectName("ventasHoyLayout");
        labelVentasHoyTitulo = new QLabel(frameVentasHoy);
        labelVentasHoyTitulo->setObjectName("labelVentasHoyTitulo");

        ventasHoyLayout->addWidget(labelVentasHoyTitulo);

        labelVentasHoy = new QLabel(frameVentasHoy);
        labelVentasHoy->setObjectName("labelVentasHoy");

        ventasHoyLayout->addWidget(labelVentasHoy);


        statsLayout->addWidget(frameVentasHoy);

        framePendientes = new QFrame(InicioPage);
        framePendientes->setObjectName("framePendientes");
        framePendientes->setFrameShape(QFrame::StyledPanel);
        pendientesLayout = new QVBoxLayout(framePendientes);
        pendientesLayout->setObjectName("pendientesLayout");
        labelPendientesTitulo = new QLabel(framePendientes);
        labelPendientesTitulo->setObjectName("labelPendientesTitulo");

        pendientesLayout->addWidget(labelPendientesTitulo);

        labelPendientes = new QLabel(framePendientes);
        labelPendientes->setObjectName("labelPendientes");

        pendientesLayout->addWidget(labelPendientes);


        statsLayout->addWidget(framePendientes);

        frameCaducidad = new QFrame(InicioPage);
        frameCaducidad->setObjectName("frameCaducidad");
        frameCaducidad->setFrameShape(QFrame::StyledPanel);
        caducidadLayout = new QVBoxLayout(frameCaducidad);
        caducidadLayout->setObjectName("caducidadLayout");
        labelCaducidadTitulo = new QLabel(frameCaducidad);
        labelCaducidadTitulo->setObjectName("labelCaducidadTitulo");

        caducidadLayout->addWidget(labelCaducidadTitulo);

        labelCaducidad = new QLabel(frameCaducidad);
        labelCaducidad->setObjectName("labelCaducidad");

        caducidadLayout->addWidget(labelCaducidad);


        statsLayout->addWidget(frameCaducidad);


        verticalLayout->addLayout(statsLayout);

        labelAccesos = new QLabel(InicioPage);
        labelAccesos->setObjectName("labelAccesos");

        verticalLayout->addWidget(labelAccesos);

        gridAccesos = new QGridLayout();
        gridAccesos->setObjectName("gridAccesos");
        btnPuntoVenta = new QPushButton(InicioPage);
        btnPuntoVenta->setObjectName("btnPuntoVenta");
        btnPuntoVenta->setMinimumHeight(64);

        gridAccesos->addWidget(btnPuntoVenta, 0, 0, 1, 1);

        btnProductos = new QPushButton(InicioPage);
        btnProductos->setObjectName("btnProductos");
        btnProductos->setMinimumHeight(64);

        gridAccesos->addWidget(btnProductos, 0, 1, 1, 1);

        btnClientes = new QPushButton(InicioPage);
        btnClientes->setObjectName("btnClientes");
        btnClientes->setMinimumHeight(64);

        gridAccesos->addWidget(btnClientes, 0, 2, 1, 1);

        btnTickets = new QPushButton(InicioPage);
        btnTickets->setObjectName("btnTickets");
        btnTickets->setMinimumHeight(64);

        gridAccesos->addWidget(btnTickets, 1, 0, 1, 1);

        btnInventario = new QPushButton(InicioPage);
        btnInventario->setObjectName("btnInventario");
        btnInventario->setMinimumHeight(64);

        gridAccesos->addWidget(btnInventario, 1, 1, 1, 1);

        btnCategorias = new QPushButton(InicioPage);
        btnCategorias->setObjectName("btnCategorias");
        btnCategorias->setMinimumHeight(64);

        gridAccesos->addWidget(btnCategorias, 1, 2, 1, 1);

        btnProveedores = new QPushButton(InicioPage);
        btnProveedores->setObjectName("btnProveedores");
        btnProveedores->setMinimumHeight(64);

        gridAccesos->addWidget(btnProveedores, 2, 0, 1, 1);

        btnEmpleados = new QPushButton(InicioPage);
        btnEmpleados->setObjectName("btnEmpleados");
        btnEmpleados->setMinimumHeight(64);

        gridAccesos->addWidget(btnEmpleados, 2, 1, 1, 1);

        btnSucursales = new QPushButton(InicioPage);
        btnSucursales->setObjectName("btnSucursales");
        btnSucursales->setMinimumHeight(64);

        gridAccesos->addWidget(btnSucursales, 2, 2, 1, 1);

        btnUsuarios = new QPushButton(InicioPage);
        btnUsuarios->setObjectName("btnUsuarios");
        btnUsuarios->setMinimumHeight(64);

        gridAccesos->addWidget(btnUsuarios, 3, 0, 1, 1);


        verticalLayout->addLayout(gridAccesos);

        spacerFinal = new QSpacerItem(0, 0, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout->addItem(spacerFinal);


        retranslateUi(InicioPage);

        QMetaObject::connectSlotsByName(InicioPage);
    } // setupUi

    void retranslateUi(QWidget *InicioPage)
    {
        labelLogo->setText(QString());
        labelBienvenida->setStyleSheet(QCoreApplication::translate("InicioPage", "font-size: 24px; font-weight: bold;", nullptr));
        labelBienvenida->setText(QCoreApplication::translate("InicioPage", "Bienvenido/a", nullptr));
        labelSubtitulo->setStyleSheet(QCoreApplication::translate("InicioPage", "color: #666;", nullptr));
        labelSubtitulo->setText(QString());
        labelVentasHoyTitulo->setText(QCoreApplication::translate("InicioPage", "Ventas de hoy", nullptr));
        labelVentasHoy->setStyleSheet(QCoreApplication::translate("InicioPage", "font-size: 24px; font-weight: bold;", nullptr));
        labelVentasHoy->setText(QCoreApplication::translate("InicioPage", "$0.00", nullptr));
        labelPendientesTitulo->setText(QCoreApplication::translate("InicioPage", "Tickets con saldo pendiente", nullptr));
        labelPendientes->setStyleSheet(QCoreApplication::translate("InicioPage", "font-size: 24px; font-weight: bold;", nullptr));
        labelPendientes->setText(QCoreApplication::translate("InicioPage", "0", nullptr));
        labelCaducidadTitulo->setText(QCoreApplication::translate("InicioPage", "Lotes por caducar (30 d\303\255as)", nullptr));
        labelCaducidad->setStyleSheet(QCoreApplication::translate("InicioPage", "font-size: 24px; font-weight: bold;", nullptr));
        labelCaducidad->setText(QCoreApplication::translate("InicioPage", "0", nullptr));
        labelAccesos->setStyleSheet(QCoreApplication::translate("InicioPage", "font-weight: bold; margin-top: 8px;", nullptr));
        labelAccesos->setText(QCoreApplication::translate("InicioPage", "Accesos r\303\241pidos", nullptr));
        btnPuntoVenta->setText(QCoreApplication::translate("InicioPage", "Punto de Venta", nullptr));
        btnProductos->setText(QCoreApplication::translate("InicioPage", "Productos", nullptr));
        btnClientes->setText(QCoreApplication::translate("InicioPage", "Clientes", nullptr));
        btnTickets->setText(QCoreApplication::translate("InicioPage", "Tickets", nullptr));
        btnInventario->setText(QCoreApplication::translate("InicioPage", "Inventario", nullptr));
        btnCategorias->setText(QCoreApplication::translate("InicioPage", "Categor\303\255as", nullptr));
        btnProveedores->setText(QCoreApplication::translate("InicioPage", "Proveedores", nullptr));
        btnEmpleados->setText(QCoreApplication::translate("InicioPage", "Empleados", nullptr));
        btnSucursales->setText(QCoreApplication::translate("InicioPage", "Sucursales", nullptr));
        btnUsuarios->setText(QCoreApplication::translate("InicioPage", "Usuarios", nullptr));
        (void)InicioPage;
    } // retranslateUi

};

namespace Ui {
    class InicioPage: public Ui_InicioPage {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_INICIOPAGE_H
