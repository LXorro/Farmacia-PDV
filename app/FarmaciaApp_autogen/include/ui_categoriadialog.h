/********************************************************************************
** Form generated from reading UI file 'categoriadialog.ui'
**
** Created by: Qt User Interface Compiler version 6.12.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CATEGORIADIALOG_H
#define UI_CATEGORIADIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_CategoriaDialog
{
public:
    QVBoxLayout *verticalLayout;
    QFormLayout *formLayout;
    QLabel *labelNombre;
    QLineEdit *lineNombre;
    QLabel *labelTipo;
    QLineEdit *lineTipo;
    QLabel *labelDescripcion;
    QTextEdit *textDescripcion;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *CategoriaDialog)
    {
        if (CategoriaDialog->objectName().isEmpty())
            CategoriaDialog->setObjectName("CategoriaDialog");
        CategoriaDialog->resize(380, 220);
        verticalLayout = new QVBoxLayout(CategoriaDialog);
        verticalLayout->setObjectName("verticalLayout");
        formLayout = new QFormLayout();
        formLayout->setObjectName("formLayout");
        labelNombre = new QLabel(CategoriaDialog);
        labelNombre->setObjectName("labelNombre");

        formLayout->setWidget(0, QFormLayout::ItemRole::LabelRole, labelNombre);

        lineNombre = new QLineEdit(CategoriaDialog);
        lineNombre->setObjectName("lineNombre");

        formLayout->setWidget(0, QFormLayout::ItemRole::FieldRole, lineNombre);

        labelTipo = new QLabel(CategoriaDialog);
        labelTipo->setObjectName("labelTipo");

        formLayout->setWidget(1, QFormLayout::ItemRole::LabelRole, labelTipo);

        lineTipo = new QLineEdit(CategoriaDialog);
        lineTipo->setObjectName("lineTipo");

        formLayout->setWidget(1, QFormLayout::ItemRole::FieldRole, lineTipo);

        labelDescripcion = new QLabel(CategoriaDialog);
        labelDescripcion->setObjectName("labelDescripcion");

        formLayout->setWidget(2, QFormLayout::ItemRole::LabelRole, labelDescripcion);

        textDescripcion = new QTextEdit(CategoriaDialog);
        textDescripcion->setObjectName("textDescripcion");
        textDescripcion->setMaximumHeight(70);

        formLayout->setWidget(2, QFormLayout::ItemRole::FieldRole, textDescripcion);


        verticalLayout->addLayout(formLayout);

        buttonBox = new QDialogButtonBox(CategoriaDialog);
        buttonBox->setObjectName("buttonBox");
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Save);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(CategoriaDialog);
        QObject::connect(buttonBox, &QDialogButtonBox::rejected, CategoriaDialog, qOverload<>(&QDialog::reject));

        QMetaObject::connectSlotsByName(CategoriaDialog);
    } // setupUi

    void retranslateUi(QDialog *CategoriaDialog)
    {
        CategoriaDialog->setWindowTitle(QCoreApplication::translate("CategoriaDialog", "Categor\303\255a", nullptr));
        labelNombre->setText(QCoreApplication::translate("CategoriaDialog", "Nombre:", nullptr));
        labelTipo->setText(QCoreApplication::translate("CategoriaDialog", "Tipo:", nullptr));
        lineTipo->setPlaceholderText(QCoreApplication::translate("CategoriaDialog", "medicamento, higiene, suplemento, equipo m\303\251dico...", nullptr));
        labelDescripcion->setText(QCoreApplication::translate("CategoriaDialog", "Descripci\303\263n:", nullptr));
    } // retranslateUi

};

namespace Ui {
    class CategoriaDialog: public Ui_CategoriaDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CATEGORIADIALOG_H
