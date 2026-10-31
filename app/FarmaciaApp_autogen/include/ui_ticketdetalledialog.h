/********************************************************************************
** Form generated from reading UI file 'ticketdetalledialog.ui'
**
** Created by: Qt User Interface Compiler version 6.12.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TICKETDETALLEDIALOG_H
#define UI_TICKETDETALLEDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_TicketDetalleDialog
{
public:
    QVBoxLayout *verticalLayout;
    QLabel *labelEncabezado;
    QLabel *labelProductos;
    QTableWidget *tableVentas;
    QLabel *labelPagos;
    QTableWidget *tablePagos;
    QHBoxLayout *accionesLayout;
    QPushButton *btnRegistrarDevolucion;
    QSpacerItem *spacer;
    QPushButton *btnRegistrarAbono;
    QPushButton *btnCerrar;

    void setupUi(QDialog *TicketDetalleDialog)
    {
        if (TicketDetalleDialog->objectName().isEmpty())
            TicketDetalleDialog->setObjectName("TicketDetalleDialog");
        TicketDetalleDialog->resize(620, 540);
        verticalLayout = new QVBoxLayout(TicketDetalleDialog);
        verticalLayout->setObjectName("verticalLayout");
        labelEncabezado = new QLabel(TicketDetalleDialog);
        labelEncabezado->setObjectName("labelEncabezado");

        verticalLayout->addWidget(labelEncabezado);

        labelProductos = new QLabel(TicketDetalleDialog);
        labelProductos->setObjectName("labelProductos");

        verticalLayout->addWidget(labelProductos);

        tableVentas = new QTableWidget(TicketDetalleDialog);
        if (tableVentas->columnCount() < 5)
            tableVentas->setColumnCount(5);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        tableVentas->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        tableVentas->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        tableVentas->setHorizontalHeaderItem(2, __qtablewidgetitem2);
        QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
        tableVentas->setHorizontalHeaderItem(3, __qtablewidgetitem3);
        QTableWidgetItem *__qtablewidgetitem4 = new QTableWidgetItem();
        tableVentas->setHorizontalHeaderItem(4, __qtablewidgetitem4);
        tableVentas->setObjectName("tableVentas");

        verticalLayout->addWidget(tableVentas);

        labelPagos = new QLabel(TicketDetalleDialog);
        labelPagos->setObjectName("labelPagos");

        verticalLayout->addWidget(labelPagos);

        tablePagos = new QTableWidget(TicketDetalleDialog);
        if (tablePagos->columnCount() < 3)
            tablePagos->setColumnCount(3);
        QTableWidgetItem *__qtablewidgetitem5 = new QTableWidgetItem();
        tablePagos->setHorizontalHeaderItem(0, __qtablewidgetitem5);
        QTableWidgetItem *__qtablewidgetitem6 = new QTableWidgetItem();
        tablePagos->setHorizontalHeaderItem(1, __qtablewidgetitem6);
        QTableWidgetItem *__qtablewidgetitem7 = new QTableWidgetItem();
        tablePagos->setHorizontalHeaderItem(2, __qtablewidgetitem7);
        tablePagos->setObjectName("tablePagos");

        verticalLayout->addWidget(tablePagos);

        accionesLayout = new QHBoxLayout();
        accionesLayout->setObjectName("accionesLayout");
        btnRegistrarDevolucion = new QPushButton(TicketDetalleDialog);
        btnRegistrarDevolucion->setObjectName("btnRegistrarDevolucion");

        accionesLayout->addWidget(btnRegistrarDevolucion);

        spacer = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        accionesLayout->addItem(spacer);

        btnRegistrarAbono = new QPushButton(TicketDetalleDialog);
        btnRegistrarAbono->setObjectName("btnRegistrarAbono");

        accionesLayout->addWidget(btnRegistrarAbono);

        btnCerrar = new QPushButton(TicketDetalleDialog);
        btnCerrar->setObjectName("btnCerrar");

        accionesLayout->addWidget(btnCerrar);


        verticalLayout->addLayout(accionesLayout);


        retranslateUi(TicketDetalleDialog);
        QObject::connect(btnCerrar, &QPushButton::clicked, TicketDetalleDialog, qOverload<>(&QDialog::accept));

        QMetaObject::connectSlotsByName(TicketDetalleDialog);
    } // setupUi

    void retranslateUi(QDialog *TicketDetalleDialog)
    {
        TicketDetalleDialog->setWindowTitle(QCoreApplication::translate("TicketDetalleDialog", "Detalle del ticket", nullptr));
        labelEncabezado->setStyleSheet(QCoreApplication::translate("TicketDetalleDialog", "font-weight: bold;", nullptr));
        labelEncabezado->setText(QCoreApplication::translate("TicketDetalleDialog", "Ticket", nullptr));
        labelProductos->setText(QCoreApplication::translate("TicketDetalleDialog", "Productos:", nullptr));
        QTableWidgetItem *___qtablewidgetitem = tableVentas->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QCoreApplication::translate("TicketDetalleDialog", "Producto", nullptr));
        QTableWidgetItem *___qtablewidgetitem1 = tableVentas->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QCoreApplication::translate("TicketDetalleDialog", "Cantidad", nullptr));
        QTableWidgetItem *___qtablewidgetitem2 = tableVentas->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QCoreApplication::translate("TicketDetalleDialog", "Precio unitario", nullptr));
        QTableWidgetItem *___qtablewidgetitem3 = tableVentas->horizontalHeaderItem(3);
        ___qtablewidgetitem3->setText(QCoreApplication::translate("TicketDetalleDialog", "Subtotal", nullptr));
        QTableWidgetItem *___qtablewidgetitem4 = tableVentas->horizontalHeaderItem(4);
        ___qtablewidgetitem4->setText(QCoreApplication::translate("TicketDetalleDialog", "Devuelto", nullptr));
        labelPagos->setText(QCoreApplication::translate("TicketDetalleDialog", "Pagos:", nullptr));
        QTableWidgetItem *___qtablewidgetitem5 = tablePagos->horizontalHeaderItem(0);
        ___qtablewidgetitem5->setText(QCoreApplication::translate("TicketDetalleDialog", "Fecha", nullptr));
        QTableWidgetItem *___qtablewidgetitem6 = tablePagos->horizontalHeaderItem(1);
        ___qtablewidgetitem6->setText(QCoreApplication::translate("TicketDetalleDialog", "Monto", nullptr));
        QTableWidgetItem *___qtablewidgetitem7 = tablePagos->horizontalHeaderItem(2);
        ___qtablewidgetitem7->setText(QCoreApplication::translate("TicketDetalleDialog", "M\303\251todo", nullptr));
        btnRegistrarDevolucion->setText(QCoreApplication::translate("TicketDetalleDialog", "Registrar devoluci\303\263n de la l\303\255nea seleccionada", nullptr));
        btnRegistrarAbono->setText(QCoreApplication::translate("TicketDetalleDialog", "Registrar abono", nullptr));
        btnCerrar->setText(QCoreApplication::translate("TicketDetalleDialog", "Cerrar", nullptr));
    } // retranslateUi

};

namespace Ui {
    class TicketDetalleDialog: public Ui_TicketDetalleDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TICKETDETALLEDIALOG_H
