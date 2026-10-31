/********************************************************************************
** Form generated from reading UI file 'usuariospage.ui'
**
** Created by: Qt User Interface Compiler version 6.12.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_USUARIOSPAGE_H
#define UI_USUARIOSPAGE_H

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

class Ui_UsuariosPage
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

    void setupUi(QWidget *UsuariosPage)
    {
        if (UsuariosPage->objectName().isEmpty())
            UsuariosPage->setObjectName("UsuariosPage");
        UsuariosPage->resize(700, 420);
        verticalLayout = new QVBoxLayout(UsuariosPage);
        verticalLayout->setObjectName("verticalLayout");
        botonesLayout = new QHBoxLayout();
        botonesLayout->setObjectName("botonesLayout");
        btnNuevo = new QPushButton(UsuariosPage);
        btnNuevo->setObjectName("btnNuevo");

        botonesLayout->addWidget(btnNuevo);

        btnEditar = new QPushButton(UsuariosPage);
        btnEditar->setObjectName("btnEditar");

        botonesLayout->addWidget(btnEditar);

        btnEliminar = new QPushButton(UsuariosPage);
        btnEliminar->setObjectName("btnEliminar");

        botonesLayout->addWidget(btnEliminar);

        spacer = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        botonesLayout->addItem(spacer);

        btnRefrescar = new QPushButton(UsuariosPage);
        btnRefrescar->setObjectName("btnRefrescar");

        botonesLayout->addWidget(btnRefrescar);


        verticalLayout->addLayout(botonesLayout);

        tableView = new QTableView(UsuariosPage);
        tableView->setObjectName("tableView");

        verticalLayout->addWidget(tableView);


        retranslateUi(UsuariosPage);

        QMetaObject::connectSlotsByName(UsuariosPage);
    } // setupUi

    void retranslateUi(QWidget *UsuariosPage)
    {
        btnNuevo->setText(QCoreApplication::translate("UsuariosPage", "Nuevo", nullptr));
        btnEditar->setText(QCoreApplication::translate("UsuariosPage", "Editar", nullptr));
        btnEliminar->setText(QCoreApplication::translate("UsuariosPage", "Eliminar", nullptr));
        btnRefrescar->setText(QCoreApplication::translate("UsuariosPage", "Refrescar", nullptr));
        (void)UsuariosPage;
    } // retranslateUi

};

namespace Ui {
    class UsuariosPage: public Ui_UsuariosPage {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_USUARIOSPAGE_H
