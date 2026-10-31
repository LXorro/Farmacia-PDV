/********************************************************************************
** Form generated from reading UI file 'logindialog.ui'
**
** Created by: Qt User Interface Compiler version 6.12.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LOGINDIALOG_H
#define UI_LOGINDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_LoginDialog
{
public:
    QVBoxLayout *verticalLayout;
    QLabel *labelTitulo;
    QFormLayout *formLayout;
    QLabel *labelUsuario;
    QLineEdit *lineUsuario;
    QLabel *labelContrasena;
    QLineEdit *lineContrasena;
    QLabel *labelError;
    QHBoxLayout *botonesLayout;
    QPushButton *btnRegistrar;
    QSpacerItem *spacer;
    QPushButton *btnIngresar;

    void setupUi(QDialog *LoginDialog)
    {
        if (LoginDialog->objectName().isEmpty())
            LoginDialog->setObjectName("LoginDialog");
        LoginDialog->resize(480, 290);
        verticalLayout = new QVBoxLayout(LoginDialog);
        verticalLayout->setObjectName("verticalLayout");
        labelTitulo = new QLabel(LoginDialog);
        labelTitulo->setObjectName("labelTitulo");
        labelTitulo->setAlignment(Qt::AlignHCenter);

        verticalLayout->addWidget(labelTitulo);

        formLayout = new QFormLayout();
        formLayout->setObjectName("formLayout");
        labelUsuario = new QLabel(LoginDialog);
        labelUsuario->setObjectName("labelUsuario");

        formLayout->setWidget(0, QFormLayout::ItemRole::LabelRole, labelUsuario);

        lineUsuario = new QLineEdit(LoginDialog);
        lineUsuario->setObjectName("lineUsuario");

        formLayout->setWidget(0, QFormLayout::ItemRole::FieldRole, lineUsuario);

        labelContrasena = new QLabel(LoginDialog);
        labelContrasena->setObjectName("labelContrasena");

        formLayout->setWidget(1, QFormLayout::ItemRole::LabelRole, labelContrasena);

        lineContrasena = new QLineEdit(LoginDialog);
        lineContrasena->setObjectName("lineContrasena");
        lineContrasena->setEchoMode(QLineEdit::Password);

        formLayout->setWidget(1, QFormLayout::ItemRole::FieldRole, lineContrasena);


        verticalLayout->addLayout(formLayout);

        labelError = new QLabel(LoginDialog);
        labelError->setObjectName("labelError");
        labelError->setWordWrap(true);

        verticalLayout->addWidget(labelError);

        botonesLayout = new QHBoxLayout();
        botonesLayout->setObjectName("botonesLayout");
        btnRegistrar = new QPushButton(LoginDialog);
        btnRegistrar->setObjectName("btnRegistrar");

        botonesLayout->addWidget(btnRegistrar);

        spacer = new QSpacerItem(0, 0, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        botonesLayout->addItem(spacer);

        btnIngresar = new QPushButton(LoginDialog);
        btnIngresar->setObjectName("btnIngresar");

        botonesLayout->addWidget(btnIngresar);


        verticalLayout->addLayout(botonesLayout);


        retranslateUi(LoginDialog);

        btnIngresar->setDefault(true);


        QMetaObject::connectSlotsByName(LoginDialog);
    } // setupUi

    void retranslateUi(QDialog *LoginDialog)
    {
        LoginDialog->setWindowTitle(QCoreApplication::translate("LoginDialog", "Iniciar sesi\303\263n \342\200\224 Farmacia", nullptr));
        labelTitulo->setText(QString());
        labelUsuario->setText(QCoreApplication::translate("LoginDialog", "Usuario:", nullptr));
        labelContrasena->setText(QCoreApplication::translate("LoginDialog", "Contrase\303\261a:", nullptr));
        labelError->setStyleSheet(QCoreApplication::translate("LoginDialog", "color: #b00020;", nullptr));
        labelError->setText(QString());
        btnRegistrar->setText(QCoreApplication::translate("LoginDialog", "Registrar usuario nuevo", nullptr));
        btnIngresar->setText(QCoreApplication::translate("LoginDialog", "Ingresar", nullptr));
    } // retranslateUi

};

namespace Ui {
    class LoginDialog: public Ui_LoginDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LOGINDIALOG_H
