#include "proveedorespage.h"
#include "ui_proveedorespage.h"
#include "../util/iconos.h"
#include "../dialogs/proveedordialog.h"

#include <QHeaderView>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlTableModel>

ProveedoresPage::ProveedoresPage(QWidget *parent) : QWidget(parent), ui(new Ui::ProveedoresPage) {
    ui->setupUi(this);

    m_model = new QSqlTableModel(this);
    m_model->setTable("Proveedores");
    m_model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    m_model->select();

    m_model->setHeaderData(m_model->fieldIndex("nombre_empresa"), Qt::Horizontal, "Empresa");
    m_model->setHeaderData(m_model->fieldIndex("telefono"), Qt::Horizontal, "Teléfono");
    m_model->setHeaderData(m_model->fieldIndex("correo"), Qt::Horizontal, "Correo");
    m_model->setHeaderData(m_model->fieldIndex("estado"), Qt::Horizontal, "Estado");

    ui->tableView->setModel(m_model);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView->setAlternatingRowColors(true);
    ui->tableView->verticalHeader()->setVisible(false);
    ui->tableView->horizontalHeader()->setStretchLastSection(true);
    ui->tableView->hideColumn(m_model->fieldIndex("id_proveedor"));  // catálogo puro, no aporta al usuario
    ui->tableView->hideColumn(m_model->fieldIndex("direccion"));
    ui->tableView->hideColumn(m_model->fieldIndex("fecha_registro"));
    ui->tableView->hideColumn(m_model->fieldIndex("fecha_ultima_compra"));
    ui->tableView->resizeColumnsToContents();

    connect(ui->btnNuevo, &QPushButton::clicked, this, &ProveedoresPage::nuevoProveedor);
    connect(ui->btnEditar, &QPushButton::clicked, this, &ProveedoresPage::editarProveedor);
    connect(ui->btnEliminar, &QPushButton::clicked, this, &ProveedoresPage::eliminarProveedor);
    connect(ui->btnRefrescar, &QPushButton::clicked, this, &ProveedoresPage::refrescar);

    ui->btnNuevo->setIcon(Iconos::nuevo());
    ui->btnEditar->setIcon(Iconos::editar());
    ui->btnEliminar->setIcon(Iconos::eliminar());
    ui->btnRefrescar->setIcon(Iconos::refrescar());
}

ProveedoresPage::~ProveedoresPage() {
    delete ui;
}

void ProveedoresPage::refrescar() {
    m_model->select();
}

void ProveedoresPage::nuevoProveedor() {
    ProveedorDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted)
        refrescar();
}

void ProveedoresPage::editarProveedor() {
    auto seleccion = ui->tableView->selectionModel()->selectedRows();
    if (seleccion.isEmpty()) {
        QMessageBox::information(this, "Editar", "Selecciona un proveedor de la lista.");
        return;
    }
    int id = m_model->data(m_model->index(seleccion.first().row(), m_model->fieldIndex("id_proveedor"))).toInt();

    ProveedorDialog dialog(this);
    dialog.setProveedorId(id);
    if (dialog.exec() == QDialog::Accepted)
        refrescar();
}

void ProveedoresPage::eliminarProveedor() {
    auto seleccion = ui->tableView->selectionModel()->selectedRows();
    if (seleccion.isEmpty()) {
        QMessageBox::information(this, "Eliminar", "Selecciona un proveedor de la lista.");
        return;
    }

    if (QMessageBox::question(this, "Confirmar", "¿Eliminar el proveedor seleccionado?") != QMessageBox::Yes)
        return;

    m_model->removeRow(seleccion.first().row());
    if (!m_model->submitAll()) {
        QMessageBox::critical(this, "Error", "No se pudo eliminar (verifica que no tenga productos asociados):\n" + m_model->lastError().text());
        m_model->revertAll();
    }
    refrescar();
}
