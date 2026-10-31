/********************************************************************************
** Form generated from reading UI file 'categoriaspage.ui'
**
** Created by: Qt User Interface Compiler version 6.12.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CATEGORIASPAGE_H
#define UI_CATEGORIASPAGE_H

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

class Ui_CategoriasPage
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

    void setupUi(QWidget *CategoriasPage)
    {
        if (CategoriasPage->objectName().isEmpty())
            CategoriasPage->setObjectName("CategoriasPage");
        CategoriasPage->resize(700, 420);
        verticalLayout = new QVBoxLayout(CategoriasPage);
        verticalLayout->setObjectName("verticalLayout");
        botonesLayout = new QHBoxLayout();
        botonesLayout->setObjectName("botonesLayout");
        btnNuevo = new QPushButton(CategoriasPage);
        btnNuevo->setObjectName("btnNuevo");

        botonesLayout->addWidget(btnNuevo);

        btnEditar = new QPushButton(CategoriasPage);
        btnEditar->setObjectName("btnEditar");

        botonesLayout->addWidget(btnEditar);

        btnEliminar = new QPushButton(CategoriasPage);
        btnEliminar->setObjectName("btnEliminar");

        botonesLayout->addWidget(btnEliminar);

        spacer = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        botonesLayout->addItem(spacer);

        btnRefrescar = new QPushButton(CategoriasPage);
        btnRefrescar->setObjectName("btnRefrescar");

        botonesLayout->addWidget(btnRefrescar);


        verticalLayout->addLayout(botonesLayout);

        tableView = new QTableView(CategoriasPage);
        tableView->setObjectName("tableView");

        verticalLayout->addWidget(tableView);


        retranslateUi(CategoriasPage);

        QMetaObject::connectSlotsByName(CategoriasPage);
    } // setupUi

    void retranslateUi(QWidget *CategoriasPage)
    {
        btnNuevo->setText(QCoreApplication::translate("CategoriasPage", "Nuevo", nullptr));
        btnEditar->setText(QCoreApplication::translate("CategoriasPage", "Editar", nullptr));
        btnEliminar->setText(QCoreApplication::translate("CategoriasPage", "Eliminar", nullptr));
        btnRefrescar->setText(QCoreApplication::translate("CategoriasPage", "Refrescar", nullptr));
        (void)CategoriasPage;
    } // retranslateUi

};

namespace Ui {
    class CategoriasPage: public Ui_CategoriasPage {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CATEGORIASPAGE_H
