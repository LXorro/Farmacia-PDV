#include "sucursalespage.h"
#include "ui_sucursalespage.h"
#include "../util/iconos.h"
#include "../dialogs/sucursaldialog.h"

#include <QHeaderView>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlTableModel>

SucursalesPage::SucursalesPage(QWidget *parent) : QWidget(parent), ui(new Ui::SucursalesPage) {
    ui->setupUi(this);

    m_model = new QSqlTableModel(this);
    m_model->setTable("Sucursales");
    m_model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    m_model->select();

    m_model->setHeaderData(m_model->fieldIndex("nombre"), Qt::Horizontal, "Nombre");
    m_model->setHeaderData(m_model->fieldIndex("direccion"), Qt::Horizontal, "Dirección");
    m_model->setHeaderData(m_model->fieldIndex("telefono"), Qt::Horizontal, "Teléfono");
    m_model->setHeaderData(m_model->fieldIndex("rol"), Qt::Horizontal, "Rol");
    m_model->setHeaderData(m_model->fieldIndex("estado"), Qt::Horizontal, "Estado");

    ui->tableView->setModel(m_model);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView->setAlternatingRowColors(true);
    ui->tableView->verticalHeader()->setVisible(false);
    ui->tableView->horizontalHeader()->setStretchLastSection(true);
    ui->tableView->hideColumn(m_model->fieldIndex("id_sucursal"));  // catálogo puro, no aporta al usuario
    ui->tableView->resizeColumnsToContents();

    connect(ui->btnNuevo, &QPushButton::clicked, this, &SucursalesPage::nuevaSucursal);
    connect(ui->btnEditar, &QPushButton::clicked, this, &SucursalesPage::editarSucursal);
    connect(ui->btnEliminar, &QPushButton::clicked, this, &SucursalesPage::eliminarSucursal);
    connect(ui->btnRefrescar, &QPushButton::clicked, this, &SucursalesPage::refrescar);

    ui->btnNuevo->setIcon(Iconos::nuevo());
    ui->btnEditar->setIcon(Iconos::editar());
    ui->btnEliminar->setIcon(Iconos::eliminar());
    ui->btnRefrescar->setIcon(Iconos::refrescar());
}

SucursalesPage::~SucursalesPage() {
    delete ui;
}

void SucursalesPage::refrescar() {
    m_model->select();
}

void SucursalesPage::nuevaSucursal() {
    SucursalDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted)
        refrescar();
}

void SucursalesPage::editarSucursal() {
    auto seleccion = ui->tableView->selectionModel()->selectedRows();
    if (seleccion.isEmpty()) {
        QMessageBox::information(this, "Editar", "Selecciona una sucursal de la lista.");
        return;
    }
    int id = m_model->data(m_model->index(seleccion.first().row(), m_model->fieldIndex("id_sucursal"))).toInt();

    SucursalDialog dialog(this);
    dialog.setSucursalId(id);
    if (dialog.exec() == QDialog::Accepted)
        refrescar();
}

void SucursalesPage::eliminarSucursal() {
    auto seleccion = ui->tableView->selectionModel()->selectedRows();
    if (seleccion.isEmpty()) {
        QMessageBox::information(this, "Eliminar", "Selecciona una sucursal de la lista.");
        return;
    }

    if (QMessageBox::question(this, "Confirmar", "¿Eliminar la sucursal seleccionada?") != QMessageBox::Yes)
        return;

    m_model->removeRow(seleccion.first().row());
    if (!m_model->submitAll()) {
        QMessageBox::critical(this, "Error", "No se pudo eliminar (verifica que no tenga clientes/empleados/lotes/tickets asociados):\n" + m_model->lastError().text());
        m_model->revertAll();
    }
    refrescar();
}
