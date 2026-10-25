#include "usuariospage.h"
#include "ui_usuariospage.h"
#include "../util/iconos.h"
#include "../dialogs/usuariodialog.h"
#include "../database/sesionactual.h"

#include <QHeaderView>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlQueryModel>

UsuariosPage::UsuariosPage(QWidget *parent) : QWidget(parent), ui(new Ui::UsuariosPage) {
    ui->setupUi(this);

    m_model = new QSqlQueryModel(this);

    ui->tableView->setModel(m_model);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView->setAlternatingRowColors(true);
    ui->tableView->verticalHeader()->setVisible(false);
    ui->tableView->horizontalHeader()->setStretchLastSection(true);

    connect(ui->btnNuevo, &QPushButton::clicked, this, &UsuariosPage::nuevoUsuario);
    connect(ui->btnEditar, &QPushButton::clicked, this, &UsuariosPage::editarUsuario);
    connect(ui->btnEliminar, &QPushButton::clicked, this, &UsuariosPage::eliminarUsuario);
    connect(ui->btnRefrescar, &QPushButton::clicked, this, &UsuariosPage::refrescar);

    ui->btnNuevo->setIcon(Iconos::nuevo());
    ui->btnEditar->setIcon(Iconos::editar());
    ui->btnEliminar->setIcon(Iconos::eliminar());
    ui->btnRefrescar->setIcon(Iconos::refrescar());

    refrescar();
}

UsuariosPage::~UsuariosPage() {
    delete ui;
}

void UsuariosPage::refrescar() {
    m_model->setQuery(
        "SELECT u.id_usuario AS 'ID', "
        "COALESCE(CONCAT(e.nombre, ' ', e.apellido_paterno), '—') AS 'Empleado', "
        "u.nombre_usuario AS 'Usuario', u.rol AS 'Rol', u.estado AS 'Estado' "
        "FROM Usuarios u "
        "LEFT JOIN Empleados e ON e.id_empleado = u.id_empleado "
        "ORDER BY u.nombre_usuario");
    ui->tableView->hideColumn(0);  // ID: catálogo puro, no aporta al usuario
    ui->tableView->resizeColumnsToContents();
}

void UsuariosPage::nuevoUsuario() {
    UsuarioDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted)
        refrescar();
}

void UsuariosPage::editarUsuario() {
    auto seleccion = ui->tableView->selectionModel()->selectedRows();
    if (seleccion.isEmpty()) {
        QMessageBox::information(this, "Editar", "Selecciona un usuario de la lista.");
        return;
    }
    int id = m_model->data(m_model->index(seleccion.first().row(), 0)).toInt();

    UsuarioDialog dialog(this);
    dialog.setUsuarioId(id);
    if (dialog.exec() == QDialog::Accepted)
        refrescar();
}

void UsuariosPage::eliminarUsuario() {
    auto seleccion = ui->tableView->selectionModel()->selectedRows();
    if (seleccion.isEmpty()) {
        QMessageBox::information(this, "Eliminar", "Selecciona un usuario de la lista.");
        return;
    }
    int id = m_model->data(m_model->index(seleccion.first().row(), 0)).toInt();
    if (id == SesionActual::instance().idUsuario()) {
        QMessageBox::warning(this, "Eliminar", "No puedes eliminar tu propia cuenta mientras tienes la sesión abierta.");
        return;
    }

    if (QMessageBox::question(this, "Confirmar", "¿Eliminar el usuario seleccionado?") != QMessageBox::Yes)
        return;

    QSqlQuery query;
    query.prepare("DELETE FROM Usuarios WHERE id_usuario = :id");
    query.bindValue(":id", id);
    if (!query.exec()) {
        QMessageBox::critical(this, "Error", "No se pudo eliminar el usuario:\n" + query.lastError().text());
        return;
    }
    refrescar();
}
