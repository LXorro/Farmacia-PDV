/********************************************************************************
** Form generated from reading UI file 'empleadodialog.ui'
**
** Created by: Qt User Interface Compiler version 6.12.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_EMPLEADODIALOG_H
#define UI_EMPLEADODIALOG_H

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

class Ui_EmpleadoDialog
{
public:
    QVBoxLayout *verticalLayout;
    QFormLayout *formLayout;
    QLabel *labelSucursal;
    QComboBox *comboSucursal;
    QLabel *labelNombre;
    QLineEdit *lineNombre;
    QLabel *labelApellidoPaterno;
    QLineEdit *lineApellidoPaterno;
    QLabel *labelApellidoMaterno;
    QLineEdit *lineApellidoMaterno;
    QLabel *labelPuesto;
    QLineEdit *linePuesto;
    QLabel *labelTelefono;
    QLineEdit *lineTelefono;
    QLabel *labelCorreo;
    QLineEdit *lineCorreo;
    QLabel *labelEstado;
    QComboBox *comboEstado;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *EmpleadoDialog)
    {
        if (EmpleadoDialog->objectName().isEmpty())
            EmpleadoDialog->setObjectName("EmpleadoDialog");
        EmpleadoDialog->resize(400, 360);
        verticalLayout = new QVBoxLayout(EmpleadoDialog);
        verticalLayout->setObjectName("verticalLayout");
        formLayout = new QFormLayout();
        formLayout->setObjectName("formLayout");
        labelSucursal = new QLabel(EmpleadoDialog);
        labelSucursal->setObjectName("labelSucursal");

        formLayout->setWidget(0, QFormLayout::ItemRole::LabelRole, labelSucursal);

        comboSucursal = new QComboBox(EmpleadoDialog);
        comboSucursal->setObjectName("comboSucursal");

        formLayout->setWidget(0, QFormLayout::ItemRole::FieldRole, comboSucursal);

        labelNombre = new QLabel(EmpleadoDialog);
        labelNombre->setObjectName("labelNombre");

        formLayout->setWidget(1, QFormLayout::ItemRole::LabelRole, labelNombre);

        lineNombre = new QLineEdit(EmpleadoDialog);
        lineNombre->setObjectName("lineNombre");

        formLayout->setWidget(1, QFormLayout::ItemRole::FieldRole, lineNombre);

        labelApellidoPaterno = new QLabel(EmpleadoDialog);
        labelApellidoPaterno->setObjectName("labelApellidoPaterno");

        formLayout->setWidget(2, QFormLayout::ItemRole::LabelRole, labelApellidoPaterno);

        lineApellidoPaterno = new QLineEdit(EmpleadoDialog);
        lineApellidoPaterno->setObjectName("lineApellidoPaterno");

        formLayout->setWidget(2, QFormLayout::ItemRole::FieldRole, lineApellidoPaterno);

        labelApellidoMaterno = new QLabel(EmpleadoDialog);
        labelApellidoMaterno->setObjectName("labelApellidoMaterno");

        formLayout->setWidget(3, QFormLayout::ItemRole::LabelRole, labelApellidoMaterno);

        lineApellidoMaterno = new QLineEdit(EmpleadoDialog);
        lineApellidoMaterno->setObjectName("lineApellidoMaterno");

        formLayout->setWidget(3, QFormLayout::ItemRole::FieldRole, lineApellidoMaterno);

        labelPuesto = new QLabel(EmpleadoDialog);
        labelPuesto->setObjectName("labelPuesto");

        formLayout->setWidget(4, QFormLayout::ItemRole::LabelRole, labelPuesto);

        linePuesto = new QLineEdit(EmpleadoDialog);
        linePuesto->setObjectName("linePuesto");

        formLayout->setWidget(4, QFormLayout::ItemRole::FieldRole, linePuesto);

        labelTelefono = new QLabel(EmpleadoDialog);
        labelTelefono->setObjectName("labelTelefono");

        formLayout->setWidget(5, QFormLayout::ItemRole::LabelRole, labelTelefono);

        lineTelefono = new QLineEdit(EmpleadoDialog);
        lineTelefono->setObjectName("lineTelefono");

        formLayout->setWidget(5, QFormLayout::ItemRole::FieldRole, lineTelefono);

        labelCorreo = new QLabel(EmpleadoDialog);
        labelCorreo->setObjectName("labelCorreo");

        formLayout->setWidget(6, QFormLayout::ItemRole::LabelRole, labelCorreo);

        lineCorreo = new QLineEdit(EmpleadoDialog);
        lineCorreo->setObjectName("lineCorreo");

        formLayout->setWidget(6, QFormLayout::ItemRole::FieldRole, lineCorreo);

        labelEstado = new QLabel(EmpleadoDialog);
        labelEstado->setObjectName("labelEstado");

        formLayout->setWidget(7, QFormLayout::ItemRole::LabelRole, labelEstado);

        comboEstado = new QComboBox(EmpleadoDialog);
        comboEstado->addItem(QString());
        comboEstado->addItem(QString());
        comboEstado->setObjectName("comboEstado");

        formLayout->setWidget(7, QFormLayout::ItemRole::FieldRole, comboEstado);


        verticalLayout->addLayout(formLayout);

        buttonBox = new QDialogButtonBox(EmpleadoDialog);
        buttonBox->setObjectName("buttonBox");
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Save);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(EmpleadoDialog);
        QObject::connect(buttonBox, &QDialogButtonBox::rejected, EmpleadoDialog, qOverload<>(&QDialog::reject));

        QMetaObject::connectSlotsByName(EmpleadoDialog);
    } // setupUi

    void retranslateUi(QDialog *EmpleadoDialog)
    {
        EmpleadoDialog->setWindowTitle(QCoreApplication::translate("EmpleadoDialog", "Empleado", nullptr));
        labelSucursal->setText(QCoreApplication::translate("EmpleadoDialog", "Sucursal:", nullptr));
        labelNombre->setText(QCoreApplication::translate("EmpleadoDialog", "Nombre:", nullptr));
        labelApellidoPaterno->setText(QCoreApplication::translate("EmpleadoDialog", "Apellido paterno:", nullptr));
        labelApellidoMaterno->setText(QCoreApplication::translate("EmpleadoDialog", "Apellido materno:", nullptr));
        labelPuesto->setText(QCoreApplication::translate("EmpleadoDialog", "Puesto:", nullptr));
        labelTelefono->setText(QCoreApplication::translate("EmpleadoDialog", "Tel\303\251fono:", nullptr));
        labelCorreo->setText(QCoreApplication::translate("EmpleadoDialog", "Correo:", nullptr));
        labelEstado->setText(QCoreApplication::translate("EmpleadoDialog", "Estado:", nullptr));
        comboEstado->setItemText(0, QCoreApplication::translate("EmpleadoDialog", "activo", nullptr));
        comboEstado->setItemText(1, QCoreApplication::translate("EmpleadoDialog", "inactivo", nullptr));

    } // retranslateUi

};

namespace Ui {
    class EmpleadoDialog: public Ui_EmpleadoDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_EMPLEADODIALOG_H
