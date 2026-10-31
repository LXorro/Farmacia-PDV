/********************************************************************************
** Form generated from reading UI file 'clientespage.ui'
**
** Created by: Qt User Interface Compiler version 6.12.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CLIENTESPAGE_H
#define UI_CLIENTESPAGE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTableView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ClientesPage
{
public:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *botonesLayout;
    QPushButton *btnNuevo;
    QPushButton *btnEditar;
    QPushButton *btnEliminar;
    QSpacerItem *spacer;
    QPushButton *btnRefrescar;
    QTableView *tableView;

    void setupUi(QWidget *ClientesPage)
    {
        if (ClientesPage->objectName().isEmpty())
            ClientesPage->setObjectName("ClientesPage");
        ClientesPage->resize(700, 420);
        verticalLayout = new QVBoxLayout(ClientesPage);
        verticalLayout->setObjectName("verticalLayout");
        botonesLayout = new QHBoxLayout();
        botonesLayout->setObjectName("botonesLayout");
        btnNuevo = new QPushButton(ClientesPage);
        btnNuevo->setObjectName("btnNuevo");

        botonesLayout->addWidget(btnNuevo);

        btnEditar = new QPushButton(ClientesPage);
        btnEditar->setObjectName("btnEditar");

        botonesLayout->addWidget(btnEditar);

        btnEliminar = new QPushButton(ClientesPage);
        btnEliminar->setObjectName("btnEliminar");

        botonesLayout->addWidget(btnEliminar);

        spacer = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        botonesLayout->addItem(spacer);

        btnRefrescar = new QPushButton(ClientesPage);
        btnRefrescar->setObjectName("btnRefrescar");

        botonesLayout->addWidget(btnRefrescar);


        verticalLayout->addLayout(botonesLayout);

        tableView = new QTableView(ClientesPage);
        tableView->setObjectName("tableView");

        verticalLayout->addWidget(tableView);


        retranslateUi(ClientesPage);

        QMetaObject::connectSlotsByName(ClientesPage);
    } // setupUi

    void retranslateUi(QWidget *ClientesPage)
    {
        btnNuevo->setText(QCoreApplication::translate("ClientesPage", "Nuevo", nullptr));
        btnEditar->setText(QCoreApplication::translate("ClientesPage", "Editar", nullptr));
        btnEliminar->setText(QCoreApplication::translate("ClientesPage", "Eliminar", nullptr));
        btnRefrescar->setText(QCoreApplication::translate("ClientesPage", "Refrescar", nullptr));
        (void)ClientesPage;
    } // retranslateUi

};

namespace Ui {
    class ClientesPage: public Ui_ClientesPage {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CLIENTESPAGE_H
