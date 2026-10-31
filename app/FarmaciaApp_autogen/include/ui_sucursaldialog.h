/********************************************************************************
** Form generated from reading UI file 'sucursaldialog.ui'
**
** Created by: Qt User Interface Compiler version 6.12.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SUCURSALDIALOG_H
#define UI_SUCURSALDIALOG_H

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

class Ui_SucursalDialog
{
public:
    QVBoxLayout *verticalLayout;
    QFormLayout *formLayout;
    QLabel *labelNombre;
    QLineEdit *lineNombre;
    QLabel *labelDireccion;
    QLineEdit *lineDireccion;
    QLabel *labelTelefono;
    QLineEdit *lineTelefono;
    QLabel *labelRol;
    QComboBox *comboRol;
    QLabel *labelEstado;
    QComboBox *comboEstado;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *SucursalDialog)
    {
        if (SucursalDialog->objectName().isEmpty())
            SucursalDialog->setObjectName("SucursalDialog");
        SucursalDialog->resize(380, 260);
        verticalLayout = new QVBoxLayout(SucursalDialog);
        verticalLayout->setObjectName("verticalLayout");
        formLayout = new QFormLayout();
        formLayout->setObjectName("formLayout");
        labelNombre = new QLabel(SucursalDialog);
        labelNombre->setObjectName("labelNombre");

        formLayout->setWidget(0, QFormLayout::ItemRole::LabelRole, labelNombre);

        lineNombre = new QLineEdit(SucursalDialog);
        lineNombre->setObjectName("lineNombre");

        formLayout->setWidget(0, QFormLayout::ItemRole::FieldRole, lineNombre);

        labelDireccion = new QLabel(SucursalDialog);
        labelDireccion->setObjectName("labelDireccion");

        formLayout->setWidget(1, QFormLayout::ItemRole::LabelRole, labelDireccion);

        lineDireccion = new QLineEdit(SucursalDialog);
        lineDireccion->setObjectName("lineDireccion");

        formLayout->setWidget(1, QFormLayout::ItemRole::FieldRole, lineDireccion);

        labelTelefono = new QLabel(SucursalDialog);
        labelTelefono->setObjectName("labelTelefono");

        formLayout->setWidget(2, QFormLayout::ItemRole::LabelRole, labelTelefono);

        lineTelefono = new QLineEdit(SucursalDialog);
        lineTelefono->setObjectName("lineTelefono");

        formLayout->setWidget(2, QFormLayout::ItemRole::FieldRole, lineTelefono);

        labelRol = new QLabel(SucursalDialog);
        labelRol->setObjectName("labelRol");

        formLayout->setWidget(3, QFormLayout::ItemRole::LabelRole, labelRol);

        comboRol = new QComboBox(SucursalDialog);
        comboRol->addItem(QString());
        comboRol->addItem(QString());
        comboRol->setObjectName("comboRol");

        formLayout->setWidget(3, QFormLayout::ItemRole::FieldRole, comboRol);

        labelEstado = new QLabel(SucursalDialog);
        labelEstado->setObjectName("labelEstado");

        formLayout->setWidget(4, QFormLayout::ItemRole::LabelRole, labelEstado);

        comboEstado = new QComboBox(SucursalDialog);
        comboEstado->addItem(QString());
        comboEstado->addItem(QString());
        comboEstado->setObjectName("comboEstado");

        formLayout->setWidget(4, QFormLayout::ItemRole::FieldRole, comboEstado);


        verticalLayout->addLayout(formLayout);

        buttonBox = new QDialogButtonBox(SucursalDialog);
        buttonBox->setObjectName("buttonBox");
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Save);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(SucursalDialog);
        QObject::connect(buttonBox, &QDialogButtonBox::rejected, SucursalDialog, qOverload<>(&QDialog::reject));

        QMetaObject::connectSlotsByName(SucursalDialog);
    } // setupUi

    void retranslateUi(QDialog *SucursalDialog)
    {
        SucursalDialog->setWindowTitle(QCoreApplication::translate("SucursalDialog", "Sucursal", nullptr));
        labelNombre->setText(QCoreApplication::translate("SucursalDialog", "Nombre:", nullptr));
        labelDireccion->setText(QCoreApplication::translate("SucursalDialog", "Direcci\303\263n:", nullptr));
        labelTelefono->setText(QCoreApplication::translate("SucursalDialog", "Tel\303\251fono:", nullptr));
        labelRol->setText(QCoreApplication::translate("SucursalDialog", "Rol:", nullptr));
        comboRol->setItemText(0, QCoreApplication::translate("SucursalDialog", "sucursal", nullptr));
        comboRol->setItemText(1, QCoreApplication::translate("SucursalDialog", "central", nullptr));

        labelEstado->setText(QCoreApplication::translate("SucursalDialog", "Estado:", nullptr));
        comboEstado->setItemText(0, QCoreApplication::translate("SucursalDialog", "activa", nullptr));
        comboEstado->setItemText(1, QCoreApplication::translate("SucursalDialog", "inactiva", nullptr));

    } // retranslateUi

};

namespace Ui {
    class SucursalDialog: public Ui_SucursalDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SUCURSALDIALOG_H
