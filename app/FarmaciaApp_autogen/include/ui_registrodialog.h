/********************************************************************************
** Form generated from reading UI file 'registrodialog.ui'
**
** Created by: Qt User Interface Compiler version 6.12.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_REGISTRODIALOG_H
#define UI_REGISTRODIALOG_H

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

class Ui_RegistroDialog
{
public:
    QVBoxLayout *verticalLayout;
    QLabel *labelAyuda;
    QFormLayout *formLayout;
    QLabel *labelEmpleado;
    QComboBox *comboEmpleado;
    QLabel *labelUsuario;
    QLineEdit *lineUsuario;
    QLabel *labelContrasena;
    QLineEdit *lineContrasena;
    QLabel *labelConfirmar;
    QLineEdit *lineConfirmar;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *RegistroDialog)
    {
        if (RegistroDialog->objectName().isEmpty())
            RegistroDialog->setObjectName("RegistroDialog");
        RegistroDialog->resize(400, 300);
        verticalLayout = new QVBoxLayout(RegistroDialog);
        verticalLayout->setObjectName("verticalLayout");
        labelAyuda = new QLabel(RegistroDialog);
        labelAyuda->setObjectName("labelAyuda");
        labelAyuda->setWordWrap(true);

        verticalLayout->addWidget(labelAyuda);

        formLayout = new QFormLayout();
        formLayout->setObjectName("formLayout");
        labelEmpleado = new QLabel(RegistroDialog);
        labelEmpleado->setObjectName("labelEmpleado");

        formLayout->setWidget(0, QFormLayout::ItemRole::LabelRole, labelEmpleado);

        comboEmpleado = new QComboBox(RegistroDialog);
        comboEmpleado->setObjectName("comboEmpleado");

        formLayout->setWidget(0, QFormLayout::ItemRole::FieldRole, comboEmpleado);

        labelUsuario = new QLabel(RegistroDialog);
        labelUsuario->setObjectName("labelUsuario");

        formLayout->setWidget(1, QFormLayout::ItemRole::LabelRole, labelUsuario);

        lineUsuario = new QLineEdit(RegistroDialog);
        lineUsuario->setObjectName("lineUsuario");

        formLayout->setWidget(1, QFormLayout::ItemRole::FieldRole, lineUsuario);

        labelContrasena = new QLabel(RegistroDialog);
        labelContrasena->setObjectName("labelContrasena");

        formLayout->setWidget(2, QFormLayout::ItemRole::LabelRole, labelContrasena);

        lineContrasena = new QLineEdit(RegistroDialog);
        lineContrasena->setObjectName("lineContrasena");
        lineContrasena->setEchoMode(QLineEdit::Password);

        formLayout->setWidget(2, QFormLayout::ItemRole::FieldRole, lineContrasena);

        labelConfirmar = new QLabel(RegistroDialog);
        labelConfirmar->setObjectName("labelConfirmar");

        formLayout->setWidget(3, QFormLayout::ItemRole::LabelRole, labelConfirmar);

        lineConfirmar = new QLineEdit(RegistroDialog);
        lineConfirmar->setObjectName("lineConfirmar");
        lineConfirmar->setEchoMode(QLineEdit::Password);

        formLayout->setWidget(3, QFormLayout::ItemRole::FieldRole, lineConfirmar);


        verticalLayout->addLayout(formLayout);

        buttonBox = new QDialogButtonBox(RegistroDialog);
        buttonBox->setObjectName("buttonBox");
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Save);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(RegistroDialog);
        QObject::connect(buttonBox, &QDialogButtonBox::rejected, RegistroDialog, qOverload<>(&QDialog::reject));

        QMetaObject::connectSlotsByName(RegistroDialog);
    } // setupUi

    void retranslateUi(QDialog *RegistroDialog)
    {
        RegistroDialog->setWindowTitle(QCoreApplication::translate("RegistroDialog", "Registrar usuario", nullptr));
        labelAyuda->setText(QCoreApplication::translate("RegistroDialog", "Cada cuenta corresponde a un empleado existente. El primer usuario que se registre en el sistema queda como administrador; los siguientes, como empleados.", nullptr));
        labelEmpleado->setText(QCoreApplication::translate("RegistroDialog", "Empleado:", nullptr));
        labelUsuario->setText(QCoreApplication::translate("RegistroDialog", "Nombre de usuario:", nullptr));
        labelContrasena->setText(QCoreApplication::translate("RegistroDialog", "Contrase\303\261a:", nullptr));
        labelConfirmar->setText(QCoreApplication::translate("RegistroDialog", "Confirmar contrase\303\261a:", nullptr));
    } // retranslateUi

};

namespace Ui {
    class RegistroDialog: public Ui_RegistroDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_REGISTRODIALOG_H
