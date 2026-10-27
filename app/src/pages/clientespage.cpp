#include "clientespage.h"
#include "ui_clientespage.h"
#include "../util/iconos.h"
#include "../util/tablas.h"
#include "../dialogs/clientedialog.h"
#include "../database/sesionactual.h"

#include <QHeaderView>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlQueryModel>

ClientesPage::ClientesPage(QWidget *parent) : QWidget(parent), ui(new Ui::ClientesPage) {
    ui->setupUi(this);

    m_model = new QSqlQueryModel(this);

    ui->tableView->setModel(m_model);
    Tablas::aplicar(ui->tableView, "No hay clientes registrados.");
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView->setAlternatingRowColors(true);
    ui->tableView->verticalHeader()->setVisible(false);
    ui->tableView->horizontalHeader()->setStretchLastSection(true);

    connect(ui->btnNuevo, &QPushButton::clicked, this, &ClientesPage::nuevoCliente);
    connect(ui->btnEditar, &QPushButton::clicked, this, &ClientesPage::editarCliente);
    connect(ui->btnEliminar, &QPushButton::clicked, this, &ClientesPage::eliminarCliente);
    connect(ui->btnRefrescar, &QPushButton::clicked, this, &ClientesPage::refrescar);

    ui->btnNuevo->setIcon(Iconos::nuevo());
    ui->btnEditar->setIcon(Iconos::editar());
    ui->btnEliminar->setIcon(Iconos::eliminar());
    ui->btnRefrescar->setIcon(Iconos::refrescar());

    refrescar();
}

ClientesPage::~ClientesPage() {
    delete ui;
}

void ClientesPage::refrescar() {
    QString sql =
        "SELECT c.id_cliente AS 'ID', s.nombre AS 'Sucursal', c.nombre AS 'Nombre', "
        "c.apellido_paterno AS 'Apellido paterno', c.apellido_materno AS 'Apellido materno', "
        "c.telefono AS 'Teléfono', c.correo AS 'Correo', c.estado AS 'Estado' "
        "FROM Clientes c "
        "JOIN Sucursales s ON s.id_sucursal = c.id_sucursal ";

    if (!SesionActual::instance().esAdmin())
        sql += QString("WHERE c.id_sucursal = %1 ").arg(SesionActual::instance().idSucursal());

    sql += "ORDER BY c.nombre";

    m_model->setQuery(sql);
    ui->tableView->hideColumn(0);  // ID: catálogo puro, no aporta al usuario
    ui->tableView->resizeColumnsToContents();
}

void ClientesPage::nuevoCliente() {
    ClienteDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted)
        refrescar();
}

void ClientesPage::editarCliente() {
    auto seleccion = ui->tableView->selectionModel()->selectedRows();
    if (seleccion.isEmpty()) {
        QMessageBox::information(this, "Editar", "Selecciona un cliente de la lista.");
        return;
    }
    int id = m_model->data(m_model->index(seleccion.first().row(), 0)).toInt();

    ClienteDialog dialog(this);
    dialog.setClienteId(id);
    if (dialog.exec() == QDialog::Accepted)
        refrescar();
}

void ClientesPage::eliminarCliente() {
    auto seleccion = ui->tableView->selectionModel()->selectedRows();
    if (seleccion.isEmpty()) {
        QMessageBox::information(this, "Eliminar", "Selecciona un cliente de la lista.");
        return;
    }
    int id = m_model->data(m_model->index(seleccion.first().row(), 0)).toInt();

    if (QMessageBox::question(this, "Confirmar", "¿Eliminar el cliente seleccionado?") != QMessageBox::Yes)
        return;

    QSqlQuery query;
    query.prepare("DELETE FROM Clientes WHERE id_cliente = :id");
    query.bindValue(":id", id);
    if (!query.exec()) {
        QMessageBox::critical(this, "Error", "No se pudo eliminar (verifica que no tenga tickets asociados):\n" + query.lastError().text());
        return;
    }
    refrescar();
}
