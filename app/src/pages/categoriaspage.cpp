#include "categoriaspage.h"
#include "ui_categoriaspage.h"
#include "../util/iconos.h"
#include "../util/tablas.h"
#include "../dialogs/categoriadialog.h"
#include "../database/sesionactual.h"

#include <QHeaderView>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlTableModel>

CategoriasPage::CategoriasPage(QWidget *parent) : QWidget(parent), ui(new Ui::CategoriasPage) {
    ui->setupUi(this);

    m_model = new QSqlTableModel(this);
    m_model->setTable("Categorias");
    m_model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    m_model->select();

    m_model->setHeaderData(m_model->fieldIndex("nombre"), Qt::Horizontal, "Nombre");
    m_model->setHeaderData(m_model->fieldIndex("tipo"), Qt::Horizontal, "Tipo");
    m_model->setHeaderData(m_model->fieldIndex("descripcion"), Qt::Horizontal, "Descripción");

    ui->tableView->setModel(m_model);
    Tablas::aplicar(ui->tableView, "No hay categorías registradas.");
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView->setAlternatingRowColors(true);
    ui->tableView->verticalHeader()->setVisible(false);
    ui->tableView->horizontalHeader()->setStretchLastSection(true);
    ui->tableView->hideColumn(m_model->fieldIndex("id_categoria"));  // catálogo puro, no aporta al usuario
    ui->tableView->resizeColumnsToContents();

    connect(ui->btnNuevo, &QPushButton::clicked, this, &CategoriasPage::nuevaCategoria);
    connect(ui->btnEditar, &QPushButton::clicked, this, &CategoriasPage::editarCategoria);
    connect(ui->btnEliminar, &QPushButton::clicked, this, &CategoriasPage::eliminarCategoria);
    connect(ui->btnRefrescar, &QPushButton::clicked, this, &CategoriasPage::refrescar);

    ui->btnNuevo->setIcon(Iconos::nuevo());
    ui->btnEditar->setIcon(Iconos::editar());
    ui->btnEliminar->setIcon(Iconos::eliminar());
    ui->btnRefrescar->setIcon(Iconos::refrescar());

    // Categorías es catálogo maestro: solo el admin (central) lo edita.
    if (!SesionActual::instance().esAdmin()) {
        ui->btnNuevo->setEnabled(false);
        ui->btnEditar->setEnabled(false);
        ui->btnEliminar->setEnabled(false);
    }
}

CategoriasPage::~CategoriasPage() {
    delete ui;
}

void CategoriasPage::refrescar() {
    m_model->select();
}

void CategoriasPage::nuevaCategoria() {
    CategoriaDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted)
        refrescar();
}

void CategoriasPage::editarCategoria() {
    auto seleccion = ui->tableView->selectionModel()->selectedRows();
    if (seleccion.isEmpty()) {
        QMessageBox::information(this, "Editar", "Selecciona una categoría de la lista.");
        return;
    }
    int id = m_model->data(m_model->index(seleccion.first().row(), m_model->fieldIndex("id_categoria"))).toInt();

    CategoriaDialog dialog(this);
    dialog.setCategoriaId(id);
    if (dialog.exec() == QDialog::Accepted)
        refrescar();
}

void CategoriasPage::eliminarCategoria() {
    auto seleccion = ui->tableView->selectionModel()->selectedRows();
    if (seleccion.isEmpty()) {
        QMessageBox::information(this, "Eliminar", "Selecciona una categoría de la lista.");
        return;
    }

    if (QMessageBox::question(this, "Confirmar", "¿Eliminar la categoría seleccionada?") != QMessageBox::Yes)
        return;

    m_model->removeRow(seleccion.first().row());
    if (!m_model->submitAll()) {
        QMessageBox::critical(this, "Error", "No se pudo eliminar (verifica que no tenga productos asociados):\n" + m_model->lastError().text());
        m_model->revertAll();
    }
    refrescar();
}
