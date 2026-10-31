/********************************************************************************
** Form generated from reading UI file 'proveedordialog.ui'
**
** Created by: Qt User Interface Compiler version 6.12.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PROVEEDORDIALOG_H
#define UI_PROVEEDORDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_ProveedorDialog
{
public:
    QVBoxLayout *verticalLayout;
    QFormLayout *formLayout;
    QLabel *labelNombreEmpresa;
    QLineEdit *lineNombreEmpresa;
    QLabel *labelTelefono;
    QLineEdit *lineTelefono;
    QLabel *labelCorreo;
    QLineEdit *lineCorreo;
    QLabel *labelDireccion;
    QLineEdit *lineDireccion;
    QLabel *labelEstado;
    QComboBox *comboEstado;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *ProveedorDialog)
    {
        if (ProveedorDialog->objectName().isEmpty())
            ProveedorDialog->setObjectName("ProveedorDialog");
        ProveedorDialog->resize(400, 280);
        verticalLayout = new QVBoxLayout(ProveedorDialog);
        verticalLayout->setObjectName("verticalLayout");
        formLayout = new QFormLayout();
        formLayout->setObjectName("formLayout");
        labelNombreEmpresa = new QLabel(ProveedorDialog);
        labelNombreEmpresa->setObjectName("labelNombreEmpresa");

        formLayout->setWidget(0, QFormLayout::ItemRole::LabelRole, labelNombreEmpresa);

        lineNombreEmpresa = new QLineEdit(ProveedorDialog);
        lineNombreEmpresa->setObjectName("lineNombreEmpresa");

        formLayout->setWidget(0, QFormLayout::ItemRole::FieldRole, lineNombreEmpresa);

        labelTelefono = new QLabel(ProveedorDialog);
        labelTelefono->setObjectName("labelTelefono");

        formLayout->setWidget(1, QFormLayout::ItemRole::LabelRole, labelTelefono);

        lineTelefono = new QLineEdit(ProveedorDialog);
        lineTelefono->setObjectName("lineTelefono");

        formLayout->setWidget(1, QFormLayout::ItemRole::FieldRole, lineTelefono);

        labelCorreo = new QLabel(ProveedorDialog);
        labelCorreo->setObjectName("labelCorreo");

        formLayout->setWidget(2, QFormLayout::ItemRole::LabelRole, labelCorreo);

        lineCorreo = new QLineEdit(ProveedorDialog);
        lineCorreo->setObjectName("lineCorreo");

        formLayout->setWidget(2, QFormLayout::ItemRole::FieldRole, lineCorreo);

        labelDireccion = new QLabel(ProveedorDialog);
        labelDireccion->setObjectName("labelDireccion");

        formLayout->setWidget(3, QFormLayout::ItemRole::LabelRole, labelDireccion);

        lineDireccion = new QLineEdit(ProveedorDialog);
        lineDireccion->setObjectName("lineDireccion");

        formLayout->setWidget(3, QFormLayout::ItemRole::FieldRole, lineDireccion);

        labelEstado = new QLabel(ProveedorDialog);
        labelEstado->setObjectName("labelEstado");

        formLayout->setWidget(4, QFormLayout::ItemRole::LabelRole, labelEstado);

        comboEstado = new QComboBox(ProveedorDialog);
        comboEstado->addItem(QString());
        comboEstado->addItem(QString());
        comboEstado->addItem(QString());
        comboEstado->setObjectName("comboEstado");

        formLayout->setWidget(4, QFormLayout::ItemRole::FieldRole, comboEstado);


        verticalLayout->addLayout(formLayout);

        buttonBox = new QDialogButtonBox(ProveedorDialog);
        buttonBox->setObjectName("buttonBox");
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Save);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(ProveedorDialog);
        QObject::connect(buttonBox, &QDialogButtonBox::rejected, ProveedorDialog, qOverload<>(&QDialog::reject));

        QMetaObject::connectSlotsByName(ProveedorDialog);
    } // setupUi

    void retranslateUi(QDialog *ProveedorDialog)
    {
        ProveedorDialog->setWindowTitle(QCoreApplication::translate("ProveedorDialog", "Proveedor", nullptr));
        labelNombreEmpresa->setText(QCoreApplication::translate("ProveedorDialog", "Nombre de la empresa:", nullptr));
        labelTelefono->setText(QCoreApplication::translate("ProveedorDialog", "Tel\303\251fono:", nullptr));
        labelCorreo->setText(QCoreApplication::translate("ProveedorDialog", "Correo:", nullptr));
        labelDireccion->setText(QCoreApplication::translate("ProveedorDialog", "Direcci\303\263n:", nullptr));
        labelEstado->setText(QCoreApplication::translate("ProveedorDialog", "Estado:", nullptr));
        comboEstado->setItemText(0, QCoreApplication::translate("ProveedorDialog", "activo", nullptr));
        comboEstado->setItemText(1, QCoreApplication::translate("ProveedorDialog", "inactivo", nullptr));
        comboEstado->setItemText(2, QCoreApplication::translate("ProveedorDialog", "suspendido", nullptr));

    } // retranslateUi

};

namespace Ui {
    class ProveedorDialog: public Ui_ProveedorDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PROVEEDORDIALOG_H
