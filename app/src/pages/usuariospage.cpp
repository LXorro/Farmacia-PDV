#include "usuariospage.h"
#include "ui_usuariospage.h"
#include "../util/iconos.h"
#include "../util/tablas.h"
#include "../dialogs/usuariodialog.h"
#include "../database/databasemanager.h"
#include "../database/sesionactual.h"

#include <QHeaderView>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlQueryModel>

UsuariosPage::UsuariosPage(QWidget *parent) : QWidget(parent), ui(new Ui::UsuariosPage) {
    ui->setupUi(this);

    m_model = new QSqlQueryModel(this);

    ui->tableView->setModel(m_model);
    Tablas::aplicar(ui->tableView, "No hay usuarios registrados.");
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

bool UsuariosPage::asegurarConexion() {
    auto &gestor = DatabaseManager::instance();
    if (gestor.centralConectado())
        return true;

    bool aceptado = false;
    const QString contrasena = QInputDialog::getText(
        this, "Conexión con el central",
        "Las cuentas se administran en el central. Escribe la contraseña de administración remota "
        "(cuenta «farmacia_remoto»):", QLineEdit::Password, QString(), &aceptado);
    if (!aceptado)
        return false;

    if (!gestor.conectarCentral(contrasena)) {
        QMessageBox::warning(this, "Sin conexión con el central",
                             "No se pudo conectar con el central; las cuentas no pueden crearse ni "
                             "modificarse sin conexión.\n\n" + gestor.lastError());
        return false;
    }
    return true;
}

void UsuariosPage::refrescar() {
    // Con el central a la vista se lista lo que ahí está; si no, la copia local replicada.
    auto &gestor = DatabaseManager::instance();
    const QSqlDatabase base = gestor.centralConectado() ? gestor.dbUsuarios() : gestor.db();
    m_model->setQuery(
        "SELECT u.id_usuario AS 'ID', "
        "COALESCE(CONCAT(e.nombre, ' ', e.apellido_paterno), '—') AS 'Empleado', "
        "u.nombre_usuario AS 'Usuario', u.rol AS 'Rol', u.estado AS 'Estado' "
        "FROM Usuarios u "
        "LEFT JOIN Empleados e ON e.id_empleado = u.id_empleado "
        "ORDER BY u.nombre_usuario", base);
    ui->tableView->hideColumn(0);  // ID: catálogo puro, no aporta al usuario
    ui->tableView->resizeColumnsToContents();
}

void UsuariosPage::nuevoUsuario() {
    if (!asegurarConexion())
        return;
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
    if (!asegurarConexion())
        return;

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

    if (!asegurarConexion())
        return;
    if (QMessageBox::question(this, "Confirmar", "¿Eliminar el usuario seleccionado?") != QMessageBox::Yes)
        return;

    QSqlQuery query(DatabaseManager::instance().dbUsuarios());
    query.prepare("DELETE FROM Usuarios WHERE id_usuario = :id");
    query.bindValue(":id", id);
    if (!query.exec()) {
        QMessageBox::critical(this, "Error", "No se pudo eliminar el usuario:\n" + query.lastError().text());
        return;
    }
    refrescar();
}
