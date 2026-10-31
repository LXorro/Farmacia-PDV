/********************************************************************************
** Form generated from reading UI file 'clientedialog.ui'
**
** Created by: Qt User Interface Compiler version 6.12.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CLIENTEDIALOG_H
#define UI_CLIENTEDIALOG_H

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

class Ui_ClienteDialog
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
    QLabel *labelTelefono;
    QLineEdit *lineTelefono;
    QLabel *labelCorreo;
    QLineEdit *lineCorreo;
    QLabel *labelDireccion;
    QLineEdit *lineDireccion;
    QLabel *labelEstado;
    QComboBox *comboEstado;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *ClienteDialog)
    {
        if (ClienteDialog->objectName().isEmpty())
            ClienteDialog->setObjectName("ClienteDialog");
        ClienteDialog->resize(400, 360);
        verticalLayout = new QVBoxLayout(ClienteDialog);
        verticalLayout->setObjectName("verticalLayout");
        formLayout = new QFormLayout();
        formLayout->setObjectName("formLayout");
        labelSucursal = new QLabel(ClienteDialog);
        labelSucursal->setObjectName("labelSucursal");

        formLayout->setWidget(0, QFormLayout::ItemRole::LabelRole, labelSucursal);

        comboSucursal = new QComboBox(ClienteDialog);
        comboSucursal->setObjectName("comboSucursal");

        formLayout->setWidget(0, QFormLayout::ItemRole::FieldRole, comboSucursal);

        labelNombre = new QLabel(ClienteDialog);
        labelNombre->setObjectName("labelNombre");

        formLayout->setWidget(1, QFormLayout::ItemRole::LabelRole, labelNombre);

        lineNombre = new QLineEdit(ClienteDialog);
        lineNombre->setObjectName("lineNombre");

        formLayout->setWidget(1, QFormLayout::ItemRole::FieldRole, lineNombre);

        labelApellidoPaterno = new QLabel(ClienteDialog);
        labelApellidoPaterno->setObjectName("labelApellidoPaterno");

        formLayout->setWidget(2, QFormLayout::ItemRole::LabelRole, labelApellidoPaterno);

        lineApellidoPaterno = new QLineEdit(ClienteDialog);
        lineApellidoPaterno->setObjectName("lineApellidoPaterno");

        formLayout->setWidget(2, QFormLayout::ItemRole::FieldRole, lineApellidoPaterno);

        labelApellidoMaterno = new QLabel(ClienteDialog);
        labelApellidoMaterno->setObjectName("labelApellidoMaterno");

        formLayout->setWidget(3, QFormLayout::ItemRole::LabelRole, labelApellidoMaterno);

        lineApellidoMaterno = new QLineEdit(ClienteDialog);
        lineApellidoMaterno->setObjectName("lineApellidoMaterno");

        formLayout->setWidget(3, QFormLayout::ItemRole::FieldRole, lineApellidoMaterno);

        labelTelefono = new QLabel(ClienteDialog);
        labelTelefono->setObjectName("labelTelefono");

        formLayout->setWidget(4, QFormLayout::ItemRole::LabelRole, labelTelefono);

        lineTelefono = new QLineEdit(ClienteDialog);
        lineTelefono->setObjectName("lineTelefono");

        formLayout->setWidget(4, QFormLayout::ItemRole::FieldRole, lineTelefono);

        labelCorreo = new QLabel(ClienteDialog);
        labelCorreo->setObjectName("labelCorreo");

        formLayout->setWidget(5, QFormLayout::ItemRole::LabelRole, labelCorreo);

        lineCorreo = new QLineEdit(ClienteDialog);
        lineCorreo->setObjectName("lineCorreo");

        formLayout->setWidget(5, QFormLayout::ItemRole::FieldRole, lineCorreo);

        labelDireccion = new QLabel(ClienteDialog);
        labelDireccion->setObjectName("labelDireccion");

        formLayout->setWidget(6, QFormLayout::ItemRole::LabelRole, labelDireccion);

        lineDireccion = new QLineEdit(ClienteDialog);
        lineDireccion->setObjectName("lineDireccion");

        formLayout->setWidget(6, QFormLayout::ItemRole::FieldRole, lineDireccion);

        labelEstado = new QLabel(ClienteDialog);
        labelEstado->setObjectName("labelEstado");

        formLayout->setWidget(7, QFormLayout::ItemRole::LabelRole, labelEstado);

        comboEstado = new QComboBox(ClienteDialog);
        comboEstado->addItem(QString());
        comboEstado->addItem(QString());
        comboEstado->setObjectName("comboEstado");

        formLayout->setWidget(7, QFormLayout::ItemRole::FieldRole, comboEstado);


        verticalLayout->addLayout(formLayout);

        buttonBox = new QDialogButtonBox(ClienteDialog);
        buttonBox->setObjectName("buttonBox");
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Save);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(ClienteDialog);
        QObject::connect(buttonBox, &QDialogButtonBox::rejected, ClienteDialog, qOverload<>(&QDialog::reject));

        QMetaObject::connectSlotsByName(ClienteDialog);
    } // setupUi

    void retranslateUi(QDialog *ClienteDialog)
    {
        ClienteDialog->setWindowTitle(QCoreApplication::translate("ClienteDialog", "Cliente", nullptr));
        labelSucursal->setText(QCoreApplication::translate("ClienteDialog", "Sucursal:", nullptr));
        labelNombre->setText(QCoreApplication::translate("ClienteDialog", "Nombre:", nullptr));
        labelApellidoPaterno->setText(QCoreApplication::translate("ClienteDialog", "Apellido paterno:", nullptr));
        labelApellidoMaterno->setText(QCoreApplication::translate("ClienteDialog", "Apellido materno:", nullptr));
        labelTelefono->setText(QCoreApplication::translate("ClienteDialog", "Tel\303\251fono:", nullptr));
        labelCorreo->setText(QCoreApplication::translate("ClienteDialog", "Correo:", nullptr));
        labelDireccion->setText(QCoreApplication::translate("ClienteDialog", "Direcci\303\263n:", nullptr));
        labelEstado->setText(QCoreApplication::translate("ClienteDialog", "Estado:", nullptr));
        comboEstado->setItemText(0, QCoreApplication::translate("ClienteDialog", "activo", nullptr));
        comboEstado->setItemText(1, QCoreApplication::translate("ClienteDialog", "inactivo", nullptr));

    } // retranslateUi

};

namespace Ui {
    class ClienteDialog: public Ui_ClienteDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CLIENTEDIALOG_H
