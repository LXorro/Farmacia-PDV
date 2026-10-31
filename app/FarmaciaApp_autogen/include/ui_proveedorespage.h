/********************************************************************************
** Form generated from reading UI file 'proveedorespage.ui'
**
** Created by: Qt User Interface Compiler version 6.12.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PROVEEDORESPAGE_H
#define UI_PROVEEDORESPAGE_H

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

class Ui_ProveedoresPage
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

    void setupUi(QWidget *ProveedoresPage)
    {
        if (ProveedoresPage->objectName().isEmpty())
            ProveedoresPage->setObjectName("ProveedoresPage");
        ProveedoresPage->resize(700, 420);
        verticalLayout = new QVBoxLayout(ProveedoresPage);
        verticalLayout->setObjectName("verticalLayout");
        botonesLayout = new QHBoxLayout();
        botonesLayout->setObjectName("botonesLayout");
        btnNuevo = new QPushButton(ProveedoresPage);
        btnNuevo->setObjectName("btnNuevo");

        botonesLayout->addWidget(btnNuevo);

        btnEditar = new QPushButton(ProveedoresPage);
        btnEditar->setObjectName("btnEditar");

        botonesLayout->addWidget(btnEditar);

        btnEliminar = new QPushButton(ProveedoresPage);
        btnEliminar->setObjectName("btnEliminar");

        botonesLayout->addWidget(btnEliminar);

        spacer = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        botonesLayout->addItem(spacer);

        btnRefrescar = new QPushButton(ProveedoresPage);
        btnRefrescar->setObjectName("btnRefrescar");

        botonesLayout->addWidget(btnRefrescar);


        verticalLayout->addLayout(botonesLayout);

        tableView = new QTableView(ProveedoresPage);
        tableView->setObjectName("tableView");

        verticalLayout->addWidget(tableView);


        retranslateUi(ProveedoresPage);

        QMetaObject::connectSlotsByName(ProveedoresPage);
    } // setupUi

    void retranslateUi(QWidget *ProveedoresPage)
    {
        btnNuevo->setText(QCoreApplication::translate("ProveedoresPage", "Nuevo", nullptr));
        btnEditar->setText(QCoreApplication::translate("ProveedoresPage", "Editar", nullptr));
        btnEliminar->setText(QCoreApplication::translate("ProveedoresPage", "Eliminar", nullptr));
        btnRefrescar->setText(QCoreApplication::translate("ProveedoresPage", "Refrescar", nullptr));
        (void)ProveedoresPage;
    } // retranslateUi

};

namespace Ui {
    class ProveedoresPage: public Ui_ProveedoresPage {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PROVEEDORESPAGE_H
