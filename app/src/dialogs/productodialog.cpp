#include "productodialog.h"
#include "ui_productodialog.h"

#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

ProductoDialog::ProductoDialog(QWidget *parent) : QDialog(parent), ui(new Ui::ProductoDialog) {
    ui->setupUi(this);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &ProductoDialog::guardar);
    cargarCombos();
}

ProductoDialog::~ProductoDialog() {
    delete ui;
}

void ProductoDialog::cargarCombos() {
    QSqlQuery categorias("SELECT id_categoria, nombre FROM Categorias ORDER BY nombre");
    while (categorias.next())
        ui->comboCategoria->addItem(categorias.value(1).toString(), categorias.value(0));

    QSqlQuery proveedores("SELECT id_proveedor, nombre_empresa FROM Proveedores ORDER BY nombre_empresa");
    while (proveedores.next())
        ui->comboProveedor->addItem(proveedores.value(1).toString(), proveedores.value(0));
}

void ProductoDialog::setProductoId(int id) {
    m_idProducto = id;
    cargarProducto();
}

void ProductoDialog::cargarProducto() {
    if (m_idProducto < 0)
        return;

    QSqlQuery query;
    query.prepare("SELECT nombre, marca, descripcion, codigo_barras, id_categoria, id_proveedor, "
                  "precio_compra, precio_venta, requiere_receta, estado "
                  "FROM Productos WHERE id_producto = :id");
    query.bindValue(":id", m_idProducto);
    if (!query.exec() || !query.next())
        return;

    ui->lineNombre->setText(query.value(0).toString());
    ui->lineMarca->setText(query.value(1).toString());
    ui->textDescripcion->setPlainText(query.value(2).toString());
    ui->lineCodigoBarras->setText(query.value(3).toString());
    ui->comboCategoria->setCurrentIndex(ui->comboCategoria->findData(query.value(4)));
    ui->comboProveedor->setCurrentIndex(ui->comboProveedor->findData(query.value(5)));
    ui->spinPrecioCompra->setValue(query.value(6).toDouble());
    ui->spinPrecioVenta->setValue(query.value(7).toDouble());
    ui->checkRequiereReceta->setChecked(query.value(8).toBool());
    ui->comboEstado->setCurrentText(query.value(9).toString());
}

void ProductoDialog::guardar() {
    if (ui->lineNombre->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Validación", "El nombre es obligatorio.");
        return;
    }

    QSqlQuery query;
    if (m_idProducto < 0) {
        query.prepare(
            "INSERT INTO Productos (nombre, marca, descripcion, codigo_barras, id_categoria, "
            "id_proveedor, precio_compra, precio_venta, requiere_receta, estado) "
            "VALUES (:nombre, :marca, :descripcion, :codigo, :categoria, :proveedor, "
            ":pcompra, :pventa, :receta, :estado)");
    } else {
        query.prepare(
            "UPDATE Productos SET nombre = :nombre, marca = :marca, descripcion = :descripcion, "
            "codigo_barras = :codigo, id_categoria = :categoria, id_proveedor = :proveedor, "
            "precio_compra = :pcompra, precio_venta = :pventa, requiere_receta = :receta, "
            "estado = :estado WHERE id_producto = :id");
        query.bindValue(":id", m_idProducto);
    }

    query.bindValue(":nombre", ui->lineNombre->text().trimmed());
    query.bindValue(":marca", ui->lineMarca->text().trimmed());
    query.bindValue(":descripcion", ui->textDescripcion->toPlainText());
    query.bindValue(":codigo", ui->lineCodigoBarras->text().trimmed());
    query.bindValue(":categoria", ui->comboCategoria->currentData());
    query.bindValue(":proveedor", ui->comboProveedor->currentData());
    query.bindValue(":pcompra", ui->spinPrecioCompra->value());
    query.bindValue(":pventa", ui->spinPrecioVenta->value());
    query.bindValue(":receta", ui->checkRequiereReceta->isChecked());
    query.bindValue(":estado", ui->comboEstado->currentText());

    if (!query.exec()) {
        QMessageBox::critical(this, "Error", "No se pudo guardar el producto:\n" + query.lastError().text());
        return;
    }

    accept();
}
