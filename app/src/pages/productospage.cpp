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
    // Existencias y disponibilidad se derivan de Lotes (no vencidos); un empleado
    // solo ve las de su sucursal, el admin el total.
    QString filtroSucursal;
    if (!SesionActual::instance().esAdmin())
        filtroSucursal = QString("AND l.id_sucursal = %1 ").arg(SesionActual::instance().idSucursal());

    const QString existencias = QString(
        "(SELECT COALESCE(SUM(l.cantidad_disponible), 0) FROM Lotes l "
        "WHERE l.id_producto = p.id_producto AND l.cantidad_disponible > 0 "
        "AND (l.fecha_caducidad IS NULL OR l.fecha_caducidad >= CURDATE()) %1)").arg(filtroSucursal);
    const QString porCaducar = QString(
        "EXISTS (SELECT 1 FROM Lotes l WHERE l.id_producto = p.id_producto AND l.cantidad_disponible > 0 "
        "AND l.fecha_caducidad >= CURDATE() AND l.fecha_caducidad <= DATE_ADD(CURDATE(), INTERVAL 30 DAY) %1)")
        .arg(filtroSucursal);

    m_model->setQuery(QString(
        "SELECT p.id_producto AS 'ID', p.nombre AS 'Nombre', p.marca AS 'Marca', "
        "p.codigo_barras AS 'Código de barras', c.nombre AS 'Categoría', "
        "pr.nombre_empresa AS 'Proveedor', p.precio_compra AS 'Precio compra', "
        "p.precio_venta AS 'Precio venta', p.requiere_receta AS 'Receta', "
        "%1 AS 'Existencias', "
        "CASE WHEN %1 = 0 THEN 'agotado' WHEN %2 THEN 'por caducar' ELSE 'disponible' END AS 'Disponibilidad', "
        "p.estado AS 'Estado' "
        "FROM Productos p "
        "LEFT JOIN Categorias c ON c.id_categoria = p.id_categoria "
        "LEFT JOIN Proveedores pr ON pr.id_proveedor = p.id_proveedor "
        "ORDER BY p.nombre").arg(existencias, porCaducar));
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

    // Con lotes (y por tanto posibles ventas/movimientos) el producto es historial: no se borra.
    QSqlQuery historial;
    historial.prepare("SELECT estado, (SELECT COUNT(*) FROM Lotes WHERE id_producto = :id) "
                      "FROM Productos WHERE id_producto = :id");
    historial.bindValue(":id", id);
    if (!historial.exec() || !historial.next()) {
        QMessageBox::critical(this, "Error", "No se pudo consultar el producto:\n" + historial.lastError().text());
        return;
    }

    if (historial.value(1).toInt() > 0) {
        if (historial.value(0).toString() == "descontinuado") {
            QMessageBox::information(this, "Descontinuado",
                                     "Este producto ya está descontinuado y tiene historial de lotes/ventas, "
                                     "por lo que no puede eliminarse. Puedes reactivarlo desde Editar.");
            return;
        }
        if (QMessageBox::question(this, "Producto con historial",
                                  "Este producto tiene lotes o ventas registradas y no puede eliminarse.\n\n"
                                  "¿Descontinuarlo? Dejará de aparecer en el Punto de Venta pero se conserva el historial.") != QMessageBox::Yes)
            return;

        QSqlQuery desactivar;
        desactivar.prepare("UPDATE Productos SET estado = 'descontinuado' WHERE id_producto = :id");
        desactivar.bindValue(":id", id);
        if (!desactivar.exec()) {
            QMessageBox::critical(this, "Error", "No se pudo descontinuar:\n" + desactivar.lastError().text());
            return;
        }
        refrescar();
        return;
    }

    if (QMessageBox::question(this, "Confirmar", "¿Eliminar definitivamente el producto seleccionado?") != QMessageBox::Yes)
        return;

    QSqlQuery query;
    query.prepare("DELETE FROM Productos WHERE id_producto = :id");
    query.bindValue(":id", id);
    if (!query.exec()) {
        QMessageBox::critical(this, "Error", "No se pudo eliminar:\n" + query.lastError().text());
        return;
    }
    refrescar();
}
