/********************************************************************************
** Form generated from reading UI file 'lotespage.ui'
**
** Created by: Qt User Interface Compiler version 6.12.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LOTESPAGE_H
#define UI_LOTESPAGE_H

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

class Ui_LotesPage
{
public:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *botonesLayout;
    QPushButton *btnNuevo;
    QPushButton *btnEditar;
    QSpacerItem *spacer;
    QPushButton *btnRefrescar;
    QTableView *tableView;

    void setupUi(QWidget *LotesPage)
    {
        if (LotesPage->objectName().isEmpty())
            LotesPage->setObjectName("LotesPage");
        LotesPage->resize(760, 420);
        verticalLayout = new QVBoxLayout(LotesPage);
        verticalLayout->setObjectName("verticalLayout");
        botonesLayout = new QHBoxLayout();
        botonesLayout->setObjectName("botonesLayout");
        btnNuevo = new QPushButton(LotesPage);
        btnNuevo->setObjectName("btnNuevo");

        botonesLayout->addWidget(btnNuevo);

        btnEditar = new QPushButton(LotesPage);
        btnEditar->setObjectName("btnEditar");

        botonesLayout->addWidget(btnEditar);

        spacer = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        botonesLayout->addItem(spacer);

        btnRefrescar = new QPushButton(LotesPage);
        btnRefrescar->setObjectName("btnRefrescar");

        botonesLayout->addWidget(btnRefrescar);


        verticalLayout->addLayout(botonesLayout);

        tableView = new QTableView(LotesPage);
        tableView->setObjectName("tableView");

        verticalLayout->addWidget(tableView);


        retranslateUi(LotesPage);

        QMetaObject::connectSlotsByName(LotesPage);
    } // setupUi

    void retranslateUi(QWidget *LotesPage)
    {
        btnNuevo->setText(QCoreApplication::translate("LotesPage", "Recibir lote", nullptr));
        btnEditar->setText(QCoreApplication::translate("LotesPage", "Editar", nullptr));
        btnRefrescar->setText(QCoreApplication::translate("LotesPage", "Refrescar", nullptr));
        (void)LotesPage;
    } // retranslateUi

};

namespace Ui {
    class LotesPage: public Ui_LotesPage {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LOTESPAGE_H
