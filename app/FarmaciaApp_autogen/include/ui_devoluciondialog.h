/********************************************************************************
** Form generated from reading UI file 'devoluciondialog.ui'
**
** Created by: Qt User Interface Compiler version 6.12.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DEVOLUCIONDIALOG_H
#define UI_DEVOLUCIONDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_DevolucionDialog
{
public:
    QVBoxLayout *verticalLayout;
    QFormLayout *formLayout;
    QLabel *labelProductoTitulo;
    QLabel *labelProducto;
    QLabel *labelEmpleado;
    QComboBox *comboEmpleado;
    QLabel *labelCantidad;
    QSpinBox *spinCantidad;
    QLabel *labelEstadoProducto;
    QComboBox *comboEstadoProducto;
    QCheckBox *checkReintegrarStock;
    QLabel *labelTipoReembolso;
    QComboBox *comboTipoReembolso;
    QLabel *labelMotivo;
    QLineEdit *lineMotivo;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *DevolucionDialog)
    {
        if (DevolucionDialog->objectName().isEmpty())
            DevolucionDialog->setObjectName("DevolucionDialog");
        DevolucionDialog->resize(420, 340);
        verticalLayout = new QVBoxLayout(DevolucionDialog);
        verticalLayout->setObjectName("verticalLayout");
        formLayout = new QFormLayout();
        formLayout->setObjectName("formLayout");
        labelProductoTitulo = new QLabel(DevolucionDialog);
        labelProductoTitulo->setObjectName("labelProductoTitulo");

        formLayout->setWidget(0, QFormLayout::ItemRole::LabelRole, labelProductoTitulo);

        labelProducto = new QLabel(DevolucionDialog);
        labelProducto->setObjectName("labelProducto");

        formLayout->setWidget(0, QFormLayout::ItemRole::FieldRole, labelProducto);

        labelEmpleado = new QLabel(DevolucionDialog);
        labelEmpleado->setObjectName("labelEmpleado");

        formLayout->setWidget(1, QFormLayout::ItemRole::LabelRole, labelEmpleado);

        comboEmpleado = new QComboBox(DevolucionDialog);
        comboEmpleado->setObjectName("comboEmpleado");

        formLayout->setWidget(1, QFormLayout::ItemRole::FieldRole, comboEmpleado);

        labelCantidad = new QLabel(DevolucionDialog);
        labelCantidad->setObjectName("labelCantidad");

        formLayout->setWidget(2, QFormLayout::ItemRole::LabelRole, labelCantidad);

        spinCantidad = new QSpinBox(DevolucionDialog);
        spinCantidad->setObjectName("spinCantidad");
        spinCantidad->setMinimum(1);

        formLayout->setWidget(2, QFormLayout::ItemRole::FieldRole, spinCantidad);

        labelEstadoProducto = new QLabel(DevolucionDialog);
        labelEstadoProducto->setObjectName("labelEstadoProducto");

        formLayout->setWidget(3, QFormLayout::ItemRole::LabelRole, labelEstadoProducto);

        comboEstadoProducto = new QComboBox(DevolucionDialog);
        comboEstadoProducto->addItem(QString());
        comboEstadoProducto->addItem(QString());
        comboEstadoProducto->addItem(QString());
        comboEstadoProducto->addItem(QString());
        comboEstadoProducto->setObjectName("comboEstadoProducto");

        formLayout->setWidget(3, QFormLayout::ItemRole::FieldRole, comboEstadoProducto);

        checkReintegrarStock = new QCheckBox(DevolucionDialog);
        checkReintegrarStock->setObjectName("checkReintegrarStock");
        checkReintegrarStock->setChecked(true);

        formLayout->setWidget(4, QFormLayout::ItemRole::FieldRole, checkReintegrarStock);

        labelTipoReembolso = new QLabel(DevolucionDialog);
        labelTipoReembolso->setObjectName("labelTipoReembolso");

        formLayout->setWidget(5, QFormLayout::ItemRole::LabelRole, labelTipoReembolso);

        comboTipoReembolso = new QComboBox(DevolucionDialog);
        comboTipoReembolso->addItem(QString());
        comboTipoReembolso->addItem(QString());
        comboTipoReembolso->addItem(QString());
        comboTipoReembolso->addItem(QString());
        comboTipoReembolso->setObjectName("comboTipoReembolso");

        formLayout->setWidget(5, QFormLayout::ItemRole::FieldRole, comboTipoReembolso);

        labelMotivo = new QLabel(DevolucionDialog);
        labelMotivo->setObjectName("labelMotivo");

        formLayout->setWidget(6, QFormLayout::ItemRole::LabelRole, labelMotivo);

        lineMotivo = new QLineEdit(DevolucionDialog);
        lineMotivo->setObjectName("lineMotivo");

        formLayout->setWidget(6, QFormLayout::ItemRole::FieldRole, lineMotivo);


        verticalLayout->addLayout(formLayout);

        buttonBox = new QDialogButtonBox(DevolucionDialog);
        buttonBox->setObjectName("buttonBox");
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Save);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(DevolucionDialog);
        QObject::connect(buttonBox, &QDialogButtonBox::rejected, DevolucionDialog, qOverload<>(&QDialog::reject));

        QMetaObject::connectSlotsByName(DevolucionDialog);
    } // setupUi

    void retranslateUi(QDialog *DevolucionDialog)
    {
        DevolucionDialog->setWindowTitle(QCoreApplication::translate("DevolucionDialog", "Registrar devoluci\303\263n", nullptr));
        labelProductoTitulo->setText(QCoreApplication::translate("DevolucionDialog", "Producto:", nullptr));
        labelProducto->setText(QString());
        labelEmpleado->setText(QCoreApplication::translate("DevolucionDialog", "Empleado que atiende:", nullptr));
        labelCantidad->setText(QCoreApplication::translate("DevolucionDialog", "Cantidad a devolver:", nullptr));
        labelEstadoProducto->setText(QCoreApplication::translate("DevolucionDialog", "Condici\303\263n del producto:", nullptr));
        comboEstadoProducto->setItemText(0, QCoreApplication::translate("DevolucionDialog", "sellado", nullptr));
        comboEstadoProducto->setItemText(1, QCoreApplication::translate("DevolucionDialog", "abierto", nullptr));
        comboEstadoProducto->setItemText(2, QCoreApplication::translate("DevolucionDialog", "da\303\261ado", nullptr));
        comboEstadoProducto->setItemText(3, QCoreApplication::translate("DevolucionDialog", "caducado", nullptr));

        checkReintegrarStock->setText(QCoreApplication::translate("DevolucionDialog", "Reintegrar al inventario", nullptr));
        labelTipoReembolso->setText(QCoreApplication::translate("DevolucionDialog", "Tipo de reembolso:", nullptr));
        comboTipoReembolso->setItemText(0, QCoreApplication::translate("DevolucionDialog", "efectivo", nullptr));
        comboTipoReembolso->setItemText(1, QCoreApplication::translate("DevolucionDialog", "transferencia", nullptr));
        comboTipoReembolso->setItemText(2, QCoreApplication::translate("DevolucionDialog", "saldo_a_favor", nullptr));
        comboTipoReembolso->setItemText(3, QCoreApplication::translate("DevolucionDialog", "cambio", nullptr));

        labelMotivo->setText(QCoreApplication::translate("DevolucionDialog", "Motivo:", nullptr));
    } // retranslateUi

};

namespace Ui {
    class DevolucionDialog: public Ui_DevolucionDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DEVOLUCIONDIALOG_H
