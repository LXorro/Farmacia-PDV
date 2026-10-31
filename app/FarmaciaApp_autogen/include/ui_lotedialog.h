/********************************************************************************
** Form generated from reading UI file 'lotedialog.ui'
**
** Created by: Qt User Interface Compiler version 6.12.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LOTEDIALOG_H
#define UI_LOTEDIALOG_H

#include <QtCore/QDate>
#include <QtCore/QVariant>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDateEdit>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_LoteDialog
{
public:
    QVBoxLayout *verticalLayout;
    QFormLayout *formLayout;
    QLabel *labelProducto;
    QComboBox *comboProducto;
    QLabel *labelSucursal;
    QComboBox *comboSucursal;
    QLabel *labelNumeroLote;
    QLineEdit *lineNumeroLote;
    QLabel *labelFechaCaducidad;
    QDateEdit *dateFechaCaducidad;
    QLabel *labelCantidad;
    QSpinBox *spinCantidad;
    QLabel *labelEstado;
    QLabel *labelEstadoValor;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *LoteDialog)
    {
        if (LoteDialog->objectName().isEmpty())
            LoteDialog->setObjectName("LoteDialog");
        LoteDialog->resize(400, 320);
        verticalLayout = new QVBoxLayout(LoteDialog);
        verticalLayout->setObjectName("verticalLayout");
        formLayout = new QFormLayout();
        formLayout->setObjectName("formLayout");
        labelProducto = new QLabel(LoteDialog);
        labelProducto->setObjectName("labelProducto");

        formLayout->setWidget(0, QFormLayout::ItemRole::LabelRole, labelProducto);

        comboProducto = new QComboBox(LoteDialog);
        comboProducto->setObjectName("comboProducto");

        formLayout->setWidget(0, QFormLayout::ItemRole::FieldRole, comboProducto);

        labelSucursal = new QLabel(LoteDialog);
        labelSucursal->setObjectName("labelSucursal");

        formLayout->setWidget(1, QFormLayout::ItemRole::LabelRole, labelSucursal);

        comboSucursal = new QComboBox(LoteDialog);
        comboSucursal->setObjectName("comboSucursal");

        formLayout->setWidget(1, QFormLayout::ItemRole::FieldRole, comboSucursal);

        labelNumeroLote = new QLabel(LoteDialog);
        labelNumeroLote->setObjectName("labelNumeroLote");

        formLayout->setWidget(2, QFormLayout::ItemRole::LabelRole, labelNumeroLote);

        lineNumeroLote = new QLineEdit(LoteDialog);
        lineNumeroLote->setObjectName("lineNumeroLote");

        formLayout->setWidget(2, QFormLayout::ItemRole::FieldRole, lineNumeroLote);

        labelFechaCaducidad = new QLabel(LoteDialog);
        labelFechaCaducidad->setObjectName("labelFechaCaducidad");

        formLayout->setWidget(3, QFormLayout::ItemRole::LabelRole, labelFechaCaducidad);

        dateFechaCaducidad = new QDateEdit(LoteDialog);
        dateFechaCaducidad->setObjectName("dateFechaCaducidad");
        dateFechaCaducidad->setCalendarPopup(true);
        dateFechaCaducidad->setDate(QDate(2027, 1, 1));

        formLayout->setWidget(3, QFormLayout::ItemRole::FieldRole, dateFechaCaducidad);

        labelCantidad = new QLabel(LoteDialog);
        labelCantidad->setObjectName("labelCantidad");

        formLayout->setWidget(4, QFormLayout::ItemRole::LabelRole, labelCantidad);

        spinCantidad = new QSpinBox(LoteDialog);
        spinCantidad->setObjectName("spinCantidad");
        spinCantidad->setMinimum(1);
        spinCantidad->setMaximum(999999);
        spinCantidad->setValue(1);

        formLayout->setWidget(4, QFormLayout::ItemRole::FieldRole, spinCantidad);

        labelEstado = new QLabel(LoteDialog);
        labelEstado->setObjectName("labelEstado");

        formLayout->setWidget(5, QFormLayout::ItemRole::LabelRole, labelEstado);

        labelEstadoValor = new QLabel(LoteDialog);
        labelEstadoValor->setObjectName("labelEstadoValor");

        formLayout->setWidget(5, QFormLayout::ItemRole::FieldRole, labelEstadoValor);


        verticalLayout->addLayout(formLayout);

        buttonBox = new QDialogButtonBox(LoteDialog);
        buttonBox->setObjectName("buttonBox");
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Save);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(LoteDialog);
        QObject::connect(buttonBox, &QDialogButtonBox::rejected, LoteDialog, qOverload<>(&QDialog::reject));

        QMetaObject::connectSlotsByName(LoteDialog);
    } // setupUi

    void retranslateUi(QDialog *LoteDialog)
    {
        LoteDialog->setWindowTitle(QCoreApplication::translate("LoteDialog", "Lote", nullptr));
        labelProducto->setText(QCoreApplication::translate("LoteDialog", "Producto:", nullptr));
        labelSucursal->setText(QCoreApplication::translate("LoteDialog", "Sucursal:", nullptr));
        labelNumeroLote->setText(QCoreApplication::translate("LoteDialog", "N\303\272mero de lote:", nullptr));
        labelFechaCaducidad->setText(QCoreApplication::translate("LoteDialog", "Fecha de caducidad:", nullptr));
        labelCantidad->setText(QCoreApplication::translate("LoteDialog", "Cantidad recibida:", nullptr));
        labelEstado->setText(QCoreApplication::translate("LoteDialog", "Estado:", nullptr));
    } // retranslateUi

};

namespace Ui {
    class LoteDialog: public Ui_LoteDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LOTEDIALOG_H
