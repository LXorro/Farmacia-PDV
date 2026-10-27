#include "productospage.h"
#include "ui_productospage.h"
#include "../util/iconos.h"
#include "../util/tablas.h"
#include "../dialogs/productodialog.h"
#include "../database/sesionactual.h"

#include <QHeaderView>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlQueryModel>

ProductosPage::ProductosPage(QWidget *parent) : QWidget(parent), ui(new Ui::ProductosPage) {
    ui->setupUi(this);

    m_model = new QSqlQueryModel(this);

    ui->tableView->setModel(m_model);
    Tablas::aplicar(ui->tableView, "No hay productos registrados.");
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView->setAlternatingRowColors(true);
    ui->tableView->verticalHeader()->setVisible(false);
    ui->tableView->horizontalHeader()->setStretchLastSection(true);

    connect(ui->btnNuevo, &QPushButton::clicked, this, &ProductosPage::nuevoProducto);
    connect(ui->btnEditar, &QPushButton::clicked, this, &ProductosPage::editarProducto);
    connect(ui->btnEliminar, &QPushButton::clicked, this, &ProductosPage::eliminarProducto);
    connect(ui->btnRefrescar, &QPushButton::clicked, this, &ProductosPage::refrescar);

    ui->btnNuevo->setIcon(Iconos::nuevo());
    ui->btnEditar->setIcon(Iconos::editar());
    ui->btnEliminar->setIcon(Iconos::eliminar());
    ui->btnRefrescar->setIcon(Iconos::refrescar());

    // Productos es catálogo maestro: solo el admin (central) lo edita.
    if (!SesionActual::instance().esAdmin()) {
        ui->btnNuevo->setEnabled(false);
        ui->btnEditar->setEnabled(false);
        ui->btnEliminar->setEnabled(false);
    }

    refrescar();
}

ProductosPage::~ProductosPage() {
    delete ui;
}

void ProductosPage::refrescar() {
    m_model->setQuery(
        "SELECT p.id_producto AS 'ID', p.nombre AS 'Nombre', p.marca AS 'Marca', "
        "p.codigo_barras AS 'Código de barras', c.nombre AS 'Categoría', "
        "pr.nombre_empresa AS 'Proveedor', p.precio_compra AS 'Precio compra', "
        "p.precio_venta AS 'Precio venta', p.requiere_receta AS 'Receta', p.estado AS 'Estado' "
        "FROM Productos p "
        "LEFT JOIN Categorias c ON c.id_categoria = p.id_categoria "
        "LEFT JOIN Proveedores pr ON pr.id_proveedor = p.id_proveedor "
        "ORDER BY p.nombre");
    ui->tableView->hideColumn(0);  // ID: catálogo puro, no aporta al usuario
    ui->tableView->resizeColumnsToContents();
}

void ProductosPage::nuevoProducto() {
    ProductoDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted)
        refrescar();
}

void ProductosPage::editarProducto() {
    auto seleccion = ui->tableView->selectionModel()->selectedRows();
    if (seleccion.isEmpty()) {
        QMessageBox::information(this, "Editar", "Selecciona un producto de la lista.");
        return;
    }
    int id = m_model->data(m_model->index(seleccion.first().row(), 0)).toInt();

    ProductoDialog dialog(this);
    dialog.setProductoId(id);
    if (dialog.exec() == QDialog::Accepted)
        refrescar();
}

void ProductosPage::eliminarProducto() {
    auto seleccion = ui->tableView->selectionModel()->selectedRows();
    if (seleccion.isEmpty()) {
        QMessageBox::information(this, "Eliminar", "Selecciona un producto de la lista.");
        return;
    }
    int id = m_model->data(m_model->index(seleccion.first().row(), 0)).toInt();

    if (QMessageBox::question(this, "Confirmar", "¿Eliminar el producto seleccionado?") != QMessageBox::Yes)
        return;

    QSqlQuery query;
    query.prepare("DELETE FROM Productos WHERE id_producto = :id");
    query.bindValue(":id", id);
    if (!query.exec()) {
        QMessageBox::critical(this, "Error", "No se pudo eliminar (verifica que no tenga lotes/ventas asociadas):\n" + query.lastError().text());
        return;
    }
    refrescar();
}
