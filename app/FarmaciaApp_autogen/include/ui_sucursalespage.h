/********************************************************************************
** Form generated from reading UI file 'sucursalespage.ui'
**
** Created by: Qt User Interface Compiler version 6.12.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SUCURSALESPAGE_H
#define UI_SUCURSALESPAGE_H

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

class Ui_SucursalesPage
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

    void setupUi(QWidget *SucursalesPage)
    {
        if (SucursalesPage->objectName().isEmpty())
            SucursalesPage->setObjectName("SucursalesPage");
        SucursalesPage->resize(700, 420);
        verticalLayout = new QVBoxLayout(SucursalesPage);
        verticalLayout->setObjectName("verticalLayout");
        botonesLayout = new QHBoxLayout();
        botonesLayout->setObjectName("botonesLayout");
        btnNuevo = new QPushButton(SucursalesPage);
        btnNuevo->setObjectName("btnNuevo");

        botonesLayout->addWidget(btnNuevo);

        btnEditar = new QPushButton(SucursalesPage);
        btnEditar->setObjectName("btnEditar");

        botonesLayout->addWidget(btnEditar);

        btnEliminar = new QPushButton(SucursalesPage);
        btnEliminar->setObjectName("btnEliminar");

        botonesLayout->addWidget(btnEliminar);

        spacer = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        botonesLayout->addItem(spacer);

        btnRefrescar = new QPushButton(SucursalesPage);
        btnRefrescar->setObjectName("btnRefrescar");

        botonesLayout->addWidget(btnRefrescar);


        verticalLayout->addLayout(botonesLayout);

        tableView = new QTableView(SucursalesPage);
        tableView->setObjectName("tableView");

        verticalLayout->addWidget(tableView);


        retranslateUi(SucursalesPage);

        QMetaObject::connectSlotsByName(SucursalesPage);
    } // setupUi

    void retranslateUi(QWidget *SucursalesPage)
    {
        btnNuevo->setText(QCoreApplication::translate("SucursalesPage", "Nuevo", nullptr));
        btnEditar->setText(QCoreApplication::translate("SucursalesPage", "Editar", nullptr));
        btnEliminar->setText(QCoreApplication::translate("SucursalesPage", "Eliminar", nullptr));
        btnRefrescar->setText(QCoreApplication::translate("SucursalesPage", "Refrescar", nullptr));
        (void)SucursalesPage;
    } // retranslateUi

};

namespace Ui {
    class SucursalesPage: public Ui_SucursalesPage {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SUCURSALESPAGE_H
