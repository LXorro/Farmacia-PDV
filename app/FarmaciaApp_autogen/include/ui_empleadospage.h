/********************************************************************************
** Form generated from reading UI file 'empleadospage.ui'
**
** Created by: Qt User Interface Compiler version 6.12.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_EMPLEADOSPAGE_H
#define UI_EMPLEADOSPAGE_H

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

class Ui_EmpleadosPage
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

    void setupUi(QWidget *EmpleadosPage)
    {
        if (EmpleadosPage->objectName().isEmpty())
            EmpleadosPage->setObjectName("EmpleadosPage");
        EmpleadosPage->resize(700, 420);
        verticalLayout = new QVBoxLayout(EmpleadosPage);
        verticalLayout->setObjectName("verticalLayout");
        botonesLayout = new QHBoxLayout();
        botonesLayout->setObjectName("botonesLayout");
        btnNuevo = new QPushButton(EmpleadosPage);
        btnNuevo->setObjectName("btnNuevo");

        botonesLayout->addWidget(btnNuevo);

        btnEditar = new QPushButton(EmpleadosPage);
        btnEditar->setObjectName("btnEditar");

        botonesLayout->addWidget(btnEditar);

        btnEliminar = new QPushButton(EmpleadosPage);
        btnEliminar->setObjectName("btnEliminar");

        botonesLayout->addWidget(btnEliminar);

        spacer = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        botonesLayout->addItem(spacer);

        btnRefrescar = new QPushButton(EmpleadosPage);
        btnRefrescar->setObjectName("btnRefrescar");

        botonesLayout->addWidget(btnRefrescar);


        verticalLayout->addLayout(botonesLayout);

        tableView = new QTableView(EmpleadosPage);
        tableView->setObjectName("tableView");

        verticalLayout->addWidget(tableView);


        retranslateUi(EmpleadosPage);

        QMetaObject::connectSlotsByName(EmpleadosPage);
    } // setupUi

    void retranslateUi(QWidget *EmpleadosPage)
    {
        btnNuevo->setText(QCoreApplication::translate("EmpleadosPage", "Nuevo", nullptr));
        btnEditar->setText(QCoreApplication::translate("EmpleadosPage", "Editar", nullptr));
        btnEliminar->setText(QCoreApplication::translate("EmpleadosPage", "Eliminar", nullptr));
        btnRefrescar->setText(QCoreApplication::translate("EmpleadosPage", "Refrescar", nullptr));
        (void)EmpleadosPage;
    } // retranslateUi

};

namespace Ui {
    class EmpleadosPage: public Ui_EmpleadosPage {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_EMPLEADOSPAGE_H
