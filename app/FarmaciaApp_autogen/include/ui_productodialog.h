/********************************************************************************
** Form generated from reading UI file 'productodialog.ui'
**
** Created by: Qt User Interface Compiler version 6.12.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PRODUCTODIALOG_H
#define UI_PRODUCTODIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_ProductoDialog
{
public:
    QVBoxLayout *verticalLayout;
    QFormLayout *formLayout;
    QLabel *labelNombre;
    QLineEdit *lineNombre;
    QLabel *labelMarca;
    QLineEdit *lineMarca;
    QLabel *labelDescripcion;
    QTextEdit *textDescripcion;
    QLabel *labelCodigoBarras;
    QLineEdit *lineCodigoBarras;
    QLabel *labelCategoria;
    QComboBox *comboCategoria;
    QLabel *labelProveedor;
    QComboBox *comboProveedor;
    QLabel *labelPrecioCompra;
    QDoubleSpinBox *spinPrecioCompra;
    QLabel *labelPrecioVenta;
    QDoubleSpinBox *spinPrecioVenta;
    QCheckBox *checkRequiereReceta;
    QLabel *labelEstado;
    QComboBox *comboEstado;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *ProductoDialog)
    {
        if (ProductoDialog->objectName().isEmpty())
            ProductoDialog->setObjectName("ProductoDialog");
        ProductoDialog->resize(420, 420);
        verticalLayout = new QVBoxLayout(ProductoDialog);
        verticalLayout->setObjectName("verticalLayout");
        formLayout = new QFormLayout();
        formLayout->setObjectName("formLayout");
        labelNombre = new QLabel(ProductoDialog);
        labelNombre->setObjectName("labelNombre");

        formLayout->setWidget(0, QFormLayout::ItemRole::LabelRole, labelNombre);

        lineNombre = new QLineEdit(ProductoDialog);
        lineNombre->setObjectName("lineNombre");

        formLayout->setWidget(0, QFormLayout::ItemRole::FieldRole, lineNombre);

        labelMarca = new QLabel(ProductoDialog);
        labelMarca->setObjectName("labelMarca");

        formLayout->setWidget(1, QFormLayout::ItemRole::LabelRole, labelMarca);

        lineMarca = new QLineEdit(ProductoDialog);
        lineMarca->setObjectName("lineMarca");

        formLayout->setWidget(1, QFormLayout::ItemRole::FieldRole, lineMarca);

        labelDescripcion = new QLabel(ProductoDialog);
        labelDescripcion->setObjectName("labelDescripcion");

        formLayout->setWidget(2, QFormLayout::ItemRole::LabelRole, labelDescripcion);

        textDescripcion = new QTextEdit(ProductoDialog);
        textDescripcion->setObjectName("textDescripcion");
        textDescripcion->setMaximumHeight(70);

        formLayout->setWidget(2, QFormLayout::ItemRole::FieldRole, textDescripcion);

        labelCodigoBarras = new QLabel(ProductoDialog);
        labelCodigoBarras->setObjectName("labelCodigoBarras");

        formLayout->setWidget(3, QFormLayout::ItemRole::LabelRole, labelCodigoBarras);

        lineCodigoBarras = new QLineEdit(ProductoDialog);
        lineCodigoBarras->setObjectName("lineCodigoBarras");

        formLayout->setWidget(3, QFormLayout::ItemRole::FieldRole, lineCodigoBarras);

        labelCategoria = new QLabel(ProductoDialog);
        labelCategoria->setObjectName("labelCategoria");

        formLayout->setWidget(4, QFormLayout::ItemRole::LabelRole, labelCategoria);

        comboCategoria = new QComboBox(ProductoDialog);
        comboCategoria->setObjectName("comboCategoria");

        formLayout->setWidget(4, QFormLayout::ItemRole::FieldRole, comboCategoria);

        labelProveedor = new QLabel(ProductoDialog);
        labelProveedor->setObjectName("labelProveedor");

        formLayout->setWidget(5, QFormLayout::ItemRole::LabelRole, labelProveedor);

        comboProveedor = new QComboBox(ProductoDialog);
        comboProveedor->setObjectName("comboProveedor");

        formLayout->setWidget(5, QFormLayout::ItemRole::FieldRole, comboProveedor);

        labelPrecioCompra = new QLabel(ProductoDialog);
        labelPrecioCompra->setObjectName("labelPrecioCompra");

        formLayout->setWidget(6, QFormLayout::ItemRole::LabelRole, labelPrecioCompra);

        spinPrecioCompra = new QDoubleSpinBox(ProductoDialog);
        spinPrecioCompra->setObjectName("spinPrecioCompra");
        spinPrecioCompra->setMaximum(999999.000000000000000);
        spinPrecioCompra->setDecimals(2);

        formLayout->setWidget(6, QFormLayout::ItemRole::FieldRole, spinPrecioCompra);

        labelPrecioVenta = new QLabel(ProductoDialog);
        labelPrecioVenta->setObjectName("labelPrecioVenta");

        formLayout->setWidget(7, QFormLayout::ItemRole::LabelRole, labelPrecioVenta);

        spinPrecioVenta = new QDoubleSpinBox(ProductoDialog);
        spinPrecioVenta->setObjectName("spinPrecioVenta");
        spinPrecioVenta->setMaximum(999999.000000000000000);
        spinPrecioVenta->setDecimals(2);

        formLayout->setWidget(7, QFormLayout::ItemRole::FieldRole, spinPrecioVenta);

        checkRequiereReceta = new QCheckBox(ProductoDialog);
        checkRequiereReceta->setObjectName("checkRequiereReceta");

        formLayout->setWidget(8, QFormLayout::ItemRole::FieldRole, checkRequiereReceta);

        labelEstado = new QLabel(ProductoDialog);
        labelEstado->setObjectName("labelEstado");

        formLayout->setWidget(9, QFormLayout::ItemRole::LabelRole, labelEstado);

        comboEstado = new QComboBox(ProductoDialog);
        comboEstado->addItem(QString());
        comboEstado->addItem(QString());
        comboEstado->setObjectName("comboEstado");

        formLayout->setWidget(9, QFormLayout::ItemRole::FieldRole, comboEstado);


        verticalLayout->addLayout(formLayout);

        buttonBox = new QDialogButtonBox(ProductoDialog);
        buttonBox->setObjectName("buttonBox");
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Save);

        verticalLayout->addWidget(buttonBox);


        retranslateUi(ProductoDialog);
        QObject::connect(buttonBox, &QDialogButtonBox::rejected, ProductoDialog, qOverload<>(&QDialog::reject));

        QMetaObject::connectSlotsByName(ProductoDialog);
    } // setupUi

    void retranslateUi(QDialog *ProductoDialog)
    {
        ProductoDialog->setWindowTitle(QCoreApplication::translate("ProductoDialog", "Producto", nullptr));
        labelNombre->setText(QCoreApplication::translate("ProductoDialog", "Nombre:", nullptr));
        labelMarca->setText(QCoreApplication::translate("ProductoDialog", "Marca:", nullptr));
        labelDescripcion->setText(QCoreApplication::translate("ProductoDialog", "Descripci\303\263n:", nullptr));
        labelCodigoBarras->setText(QCoreApplication::translate("ProductoDialog", "C\303\263digo de barras:", nullptr));
        labelCategoria->setText(QCoreApplication::translate("ProductoDialog", "Categor\303\255a:", nullptr));
        labelProveedor->setText(QCoreApplication::translate("ProductoDialog", "Proveedor:", nullptr));
        labelPrecioCompra->setText(QCoreApplication::translate("ProductoDialog", "Precio de compra:", nullptr));
        labelPrecioVenta->setText(QCoreApplication::translate("ProductoDialog", "Precio de venta:", nullptr));
        checkRequiereReceta->setText(QCoreApplication::translate("ProductoDialog", "Requiere receta m\303\251dica", nullptr));
        labelEstado->setText(QCoreApplication::translate("ProductoDialog", "Estado:", nullptr));
        comboEstado->setItemText(0, QCoreApplication::translate("ProductoDialog", "activo", nullptr));
        comboEstado->setItemText(1, QCoreApplication::translate("ProductoDialog", "descontinuado", nullptr));

    } // retranslateUi

};

namespace Ui {
    class ProductoDialog: public Ui_ProductoDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PRODUCTODIALOG_H
