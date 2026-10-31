/********************************************************************************
** Form generated from reading UI file 'ticketspage.ui'
**
** Created by: Qt User Interface Compiler version 6.12.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TICKETSPAGE_H
#define UI_TICKETSPAGE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDateEdit>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QTableView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_TicketsPage
{
public:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *botonesLayout;
    QLabel *labelFecha;
    QDateEdit *dateFiltro;
    QPushButton *btnHoy;
    QCheckBox *checkTodos;
    QSpacerItem *spacer;
    QPushButton *btnVerDetalle;
    QPushButton *btnRefrescar;
    QSplitter *splitter;
    QWidget *panelTickets;
    QVBoxLayout *panelTicketsLayout;
    QLabel *labelSeccionTickets;
    QTableView *tableView;
    QWidget *panelDevoluciones;
    QVBoxLayout *panelDevolucionesLayout;
    QLabel *labelSeccionDevoluciones;
    QTableView *tableDevoluciones;

    void setupUi(QWidget *TicketsPage)
    {
        if (TicketsPage->objectName().isEmpty())
            TicketsPage->setObjectName("TicketsPage");
        TicketsPage->resize(760, 520);
        verticalLayout = new QVBoxLayout(TicketsPage);
        verticalLayout->setObjectName("verticalLayout");
        botonesLayout = new QHBoxLayout();
        botonesLayout->setObjectName("botonesLayout");
        labelFecha = new QLabel(TicketsPage);
        labelFecha->setObjectName("labelFecha");

        botonesLayout->addWidget(labelFecha);

        dateFiltro = new QDateEdit(TicketsPage);
        dateFiltro->setObjectName("dateFiltro");
        dateFiltro->setCalendarPopup(true);

        botonesLayout->addWidget(dateFiltro);

        btnHoy = new QPushButton(TicketsPage);
        btnHoy->setObjectName("btnHoy");

        botonesLayout->addWidget(btnHoy);

        checkTodos = new QCheckBox(TicketsPage);
        checkTodos->setObjectName("checkTodos");

        botonesLayout->addWidget(checkTodos);

        spacer = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        botonesLayout->addItem(spacer);

        btnVerDetalle = new QPushButton(TicketsPage);
        btnVerDetalle->setObjectName("btnVerDetalle");

        botonesLayout->addWidget(btnVerDetalle);

        btnRefrescar = new QPushButton(TicketsPage);
        btnRefrescar->setObjectName("btnRefrescar");

        botonesLayout->addWidget(btnRefrescar);


        verticalLayout->addLayout(botonesLayout);

        splitter = new QSplitter(TicketsPage);
        splitter->setObjectName("splitter");
        splitter->setOrientation(Qt::Vertical);
        panelTickets = new QWidget(splitter);
        panelTickets->setObjectName("panelTickets");
        panelTicketsLayout = new QVBoxLayout(panelTickets);
        panelTicketsLayout->setObjectName("panelTicketsLayout");
        panelTicketsLayout->setContentsMargins(0, 0, 0, 0);
        labelSeccionTickets = new QLabel(panelTickets);
        labelSeccionTickets->setObjectName("labelSeccionTickets");

        panelTicketsLayout->addWidget(labelSeccionTickets);

        tableView = new QTableView(panelTickets);
        tableView->setObjectName("tableView");

        panelTicketsLayout->addWidget(tableView);

        splitter->addWidget(panelTickets);
        panelDevoluciones = new QWidget(splitter);
        panelDevoluciones->setObjectName("panelDevoluciones");
        panelDevolucionesLayout = new QVBoxLayout(panelDevoluciones);
        panelDevolucionesLayout->setObjectName("panelDevolucionesLayout");
        panelDevolucionesLayout->setContentsMargins(0, 0, 0, 0);
        labelSeccionDevoluciones = new QLabel(panelDevoluciones);
        labelSeccionDevoluciones->setObjectName("labelSeccionDevoluciones");

        panelDevolucionesLayout->addWidget(labelSeccionDevoluciones);

        tableDevoluciones = new QTableView(panelDevoluciones);
        tableDevoluciones->setObjectName("tableDevoluciones");

        panelDevolucionesLayout->addWidget(tableDevoluciones);

        splitter->addWidget(panelDevoluciones);

        verticalLayout->addWidget(splitter);


        retranslateUi(TicketsPage);

        QMetaObject::connectSlotsByName(TicketsPage);
    } // setupUi

    void retranslateUi(QWidget *TicketsPage)
    {
        labelFecha->setText(QCoreApplication::translate("TicketsPage", "Fecha:", nullptr));
        btnHoy->setText(QCoreApplication::translate("TicketsPage", "Hoy", nullptr));
        checkTodos->setText(QCoreApplication::translate("TicketsPage", "Mostrar todos los d\303\255as", nullptr));
        btnVerDetalle->setText(QCoreApplication::translate("TicketsPage", "Ver detalle / cobrar / devolver", nullptr));
        btnRefrescar->setText(QCoreApplication::translate("TicketsPage", "Refrescar", nullptr));
        labelSeccionTickets->setStyleSheet(QCoreApplication::translate("TicketsPage", "font-weight: bold;", nullptr));
        labelSeccionTickets->setText(QCoreApplication::translate("TicketsPage", "Tickets", nullptr));
        labelSeccionDevoluciones->setStyleSheet(QCoreApplication::translate("TicketsPage", "font-weight: bold;", nullptr));
        labelSeccionDevoluciones->setText(QCoreApplication::translate("TicketsPage", "Devoluciones", nullptr));
        (void)TicketsPage;
    } // retranslateUi

};

namespace Ui {
    class TicketsPage: public Ui_TicketsPage {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TICKETSPAGE_H
