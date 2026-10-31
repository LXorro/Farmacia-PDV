/********************************************************************************
** Form generated from reading UI file 'usuariodialog.ui'
**
** Created by: Qt User Interface Compiler version 6.12.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_USUARIODIALOG_H
#define UI_USUARIODIALOG_H

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

class Ui_UsuarioDialog
{
public:
    QVBoxLayout *verticalLayout;
    QFormLayout *formLayout;
    QLabel *labelEmpleado;
    QComboBox *comboEmpleado;
    QLabel *labelUsuario;
    QLineEdit *lineUsuario;
    QLabel *labelContrasena;
    QLineEdit *lineContrasena;
    QLabel *labelRol;
    QComboBox *comboRol;
    QLabel *labelEstado;
    QComboBox *comboEstado;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *UsuarioDialog)
    {
        if (UsuarioDialog->objectName().isEmpty())
            UsuarioDialog->setObjectName("UsuarioDialog");
        UsuarioDialog->resize(400, 300);
        verticalLayout = new QVBoxLayout(UsuarioDialog);
        verticalLayout->setObjectName("verticalLayout");
        formLayout = new QFormLayout();
        formLayout->setObjectName("formLayout");
        labelEmpleado = new QLabel(UsuarioDialog);
        labelEmpleado->setObjectName("labelEmpleado");

        formLayout->setWidget(0, QFormLayout::ItemRole::LabelRole, labelEmpleado);

        comboEmpleado = new QComboBox(UsuarioDialog);
        comboEmpleado->setObjectName("comboEmpleado");

        formLayout->setWidget(0, QFormLayout::ItemRole::FieldRole, comboEmpleado);

        labelUsuario = new QLabel(UsuarioDialog);
        labelUsuario->setObjectName("labelUsuario");

        formLayout->setWidget(1, QFormLayout::ItemRole::LabelRole, labelUsuario);

        lineUsuario = new QLineEdit(UsuarioDialog);
        lineUsuario->setObjectName("lineUsuario");

        formLayout->setWidget(1, QFormLayout::ItemRole::FieldRole, lineUsuario);

        labelContrasena = new QLabel(UsuarioDialog);
        labelContrasena->setObjectName("labelContrasena");

        formLayout->setWidget(2, QFormLayout::ItemRole::LabelRole, labelContrasena);

        lineContrasena = new QLineEdit(UsuarioDialog);
        lineContrasena->setObjectName("lineContrasena");
        lineContrasena->setEchoMode(QLineEdit::Password);

        formLayout->setWidget(2, QFormLayout::ItemRole::FieldRole, lineContrasena);

        labelRol = new QLabel(UsuarioDialog);
        labelRol->setObjectName("labelRol");

        formLayout->setWidget(3, QFormLayout::ItemRole::LabelRole, labelRol);

        comboRol = new QComboBox(UsuarioDialog);
        comboRol->addItem(QString());
        comboRol->addItem(QString());
        comboRol->setObjectName("comboRol");

        formLayout->setWidget(3, QFormLayout::ItemRole::FieldRole, comboRol);

        labelEstado = new QLabel(UsuarioDialog);
        labelEstado->setObjectName("labelEstado");

        formLayout->setWidget(4, QFormLayout::ItemRole::LabelRole, labelEstado);

        comboEstado = new QComboBox(UsuarioDialog);
        comboEstado->addItem(QString());
        comboEstado->addItem(QString());
        comboEstado->setObjectName("comboEstado");

        formLayout->setWidget(4, QFormLayout::ItemRole::FieldRole, comboEstado);


        verticalLayout->addLayout(formLayout);

        buttonBox = new QDialogButtonBox(UsuarioDialog);
        buttonBox->setObjectName("buttonBox");
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Save);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(UsuarioDialog);
        QObject::connect(buttonBox, &QDialogButtonBox::rejected, UsuarioDialog, qOverload<>(&QDialog::reject));

        QMetaObject::connectSlotsByName(UsuarioDialog);
    } // setupUi

    void retranslateUi(QDialog *UsuarioDialog)
    {
        UsuarioDialog->setWindowTitle(QCoreApplication::translate("UsuarioDialog", "Usuario", nullptr));
        labelEmpleado->setText(QCoreApplication::translate("UsuarioDialog", "Empleado:", nullptr));
        labelUsuario->setText(QCoreApplication::translate("UsuarioDialog", "Nombre de usuario:", nullptr));
        labelContrasena->setText(QCoreApplication::translate("UsuarioDialog", "Nueva contrase\303\261a:", nullptr));
        lineContrasena->setPlaceholderText(QCoreApplication::translate("UsuarioDialog", "dejar vac\303\255o para no cambiarla", nullptr));
        labelRol->setText(QCoreApplication::translate("UsuarioDialog", "Rol:", nullptr));
        comboRol->setItemText(0, QCoreApplication::translate("UsuarioDialog", "empleado", nullptr));
        comboRol->setItemText(1, QCoreApplication::translate("UsuarioDialog", "admin", nullptr));

        labelEstado->setText(QCoreApplication::translate("UsuarioDialog", "Estado:", nullptr));
        comboEstado->setItemText(0, QCoreApplication::translate("UsuarioDialog", "activo", nullptr));
        comboEstado->setItemText(1, QCoreApplication::translate("UsuarioDialog", "inactivo", nullptr));

    } // retranslateUi

};

namespace Ui {
    class UsuarioDialog: public Ui_UsuarioDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_USUARIODIALOG_H
