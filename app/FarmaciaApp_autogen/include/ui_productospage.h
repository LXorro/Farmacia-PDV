/********************************************************************************
** Form generated from reading UI file 'productospage.ui'
**
** Created by: Qt User Interface Compiler version 6.12.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PRODUCTOSPAGE_H
#define UI_PRODUCTOSPAGE_H

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

class Ui_ProductosPage
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

    void setupUi(QWidget *ProductosPage)
    {
        if (ProductosPage->objectName().isEmpty())
            ProductosPage->setObjectName("ProductosPage");
        ProductosPage->resize(700, 420);
        verticalLayout = new QVBoxLayout(ProductosPage);
        verticalLayout->setObjectName("verticalLayout");
        botonesLayout = new QHBoxLayout();
        botonesLayout->setObjectName("botonesLayout");
        btnNuevo = new QPushButton(ProductosPage);
        btnNuevo->setObjectName("btnNuevo");

        botonesLayout->addWidget(btnNuevo);

        btnEditar = new QPushButton(ProductosPage);
        btnEditar->setObjectName("btnEditar");

        botonesLayout->addWidget(btnEditar);

        btnEliminar = new QPushButton(ProductosPage);
        btnEliminar->setObjectName("btnEliminar");

        botonesLayout->addWidget(btnEliminar);

        spacer = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        botonesLayout->addItem(spacer);

        btnRefrescar = new QPushButton(ProductosPage);
        btnRefrescar->setObjectName("btnRefrescar");

        botonesLayout->addWidget(btnRefrescar);


        verticalLayout->addLayout(botonesLayout);

        tableView = new QTableView(ProductosPage);
        tableView->setObjectName("tableView");

        verticalLayout->addWidget(tableView);


        retranslateUi(ProductosPage);

        QMetaObject::connectSlotsByName(ProductosPage);
    } // setupUi

    void retranslateUi(QWidget *ProductosPage)
    {
        btnNuevo->setText(QCoreApplication::translate("ProductosPage", "Nuevo", nullptr));
        btnEditar->setText(QCoreApplication::translate("ProductosPage", "Editar", nullptr));
        btnEliminar->setText(QCoreApplication::translate("ProductosPage", "Eliminar", nullptr));
        btnRefrescar->setText(QCoreApplication::translate("ProductosPage", "Refrescar", nullptr));
        (void)ProductosPage;
    } // retranslateUi

};

namespace Ui {
    class ProductosPage: public Ui_ProductosPage {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PRODUCTOSPAGE_H
