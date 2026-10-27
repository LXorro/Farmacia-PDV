#include "empleadospage.h"
#include "ui_empleadospage.h"
#include "../util/iconos.h"
#include "../util/tablas.h"
#include "../dialogs/empleadodialog.h"

#include <QHeaderView>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlQueryModel>

EmpleadosPage::EmpleadosPage(QWidget *parent) : QWidget(parent), ui(new Ui::EmpleadosPage) {
    ui->setupUi(this);

    m_model = new QSqlQueryModel(this);

    ui->tableView->setModel(m_model);
    Tablas::aplicar(ui->tableView, "No hay empleados registrados.");
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView->setAlternatingRowColors(true);
    ui->tableView->verticalHeader()->setVisible(false);
    ui->tableView->horizontalHeader()->setStretchLastSection(true);

    connect(ui->btnNuevo, &QPushButton::clicked, this, &EmpleadosPage::nuevoEmpleado);
    connect(ui->btnEditar, &QPushButton::clicked, this, &EmpleadosPage::editarEmpleado);
    connect(ui->btnEliminar, &QPushButton::clicked, this, &EmpleadosPage::eliminarEmpleado);
    connect(ui->btnRefrescar, &QPushButton::clicked, this, &EmpleadosPage::refrescar);

    ui->btnNuevo->setIcon(Iconos::nuevo());
    ui->btnEditar->setIcon(Iconos::editar());
    ui->btnEliminar->setIcon(Iconos::eliminar());
    ui->btnRefrescar->setIcon(Iconos::refrescar());

    refrescar();
}

EmpleadosPage::~EmpleadosPage() {
    delete ui;
}

void EmpleadosPage::refrescar() {
    m_model->setQuery(
        "SELECT e.id_empleado AS 'ID', s.nombre AS 'Sucursal', e.nombre AS 'Nombre', "
        "e.apellido_paterno AS 'Apellido paterno', e.apellido_materno AS 'Apellido materno', "
        "e.puesto AS 'Puesto', e.estado AS 'Estado' "
        "FROM Empleados e "
        "JOIN Sucursales s ON s.id_sucursal = e.id_sucursal "
        "ORDER BY e.nombre");
    ui->tableView->hideColumn(0);  // ID: catálogo puro, no aporta al usuario
    ui->tableView->resizeColumnsToContents();
}

void EmpleadosPage::nuevoEmpleado() {
    EmpleadoDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted)
        refrescar();
}

void EmpleadosPage::editarEmpleado() {
    auto seleccion = ui->tableView->selectionModel()->selectedRows();
    if (seleccion.isEmpty()) {
        QMessageBox::information(this, "Editar", "Selecciona un empleado de la lista.");
        return;
    }
    int id = m_model->data(m_model->index(seleccion.first().row(), 0)).toInt();

    EmpleadoDialog dialog(this);
    dialog.setEmpleadoId(id);
    if (dialog.exec() == QDialog::Accepted)
        refrescar();
}

void EmpleadosPage::eliminarEmpleado() {
    auto seleccion = ui->tableView->selectionModel()->selectedRows();
    if (seleccion.isEmpty()) {
        QMessageBox::information(this, "Eliminar", "Selecciona un empleado de la lista.");
        return;
    }
    int id = m_model->data(m_model->index(seleccion.first().row(), 0)).toInt();

    if (QMessageBox::question(this, "Confirmar", "¿Eliminar el empleado seleccionado?") != QMessageBox::Yes)
        return;

    QSqlQuery query;
    query.prepare("DELETE FROM Empleados WHERE id_empleado = :id");
    query.bindValue(":id", id);
    if (!query.exec()) {
        QMessageBox::critical(this, "Error", "No se pudo eliminar (verifica que no tenga tickets/devoluciones asociadas):\n" + query.lastError().text());
        return;
    }
    refrescar();
}
