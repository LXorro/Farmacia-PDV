#include "asignacionespage.h"
#include "ui_asignacionespage.h"
#include "../util/iconos.h"
#include "../util/tablas.h"
#include "../dialogs/asignaciondialog.h"

#include <QHeaderView>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlQueryModel>

AsignacionesPage::AsignacionesPage(QWidget *parent) : QWidget(parent), ui(new Ui::AsignacionesPage) {
    ui->setupUi(this);

    m_model = new QSqlQueryModel(this);

    ui->tableView->setModel(m_model);
    Tablas::aplicar(ui->tableView, "No hay asignaciones temporales.");
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView->setAlternatingRowColors(true);
    ui->tableView->verticalHeader()->setVisible(false);
    ui->tableView->horizontalHeader()->setStretchLastSection(true);

    connect(ui->btnNuevo, &QPushButton::clicked, this, &AsignacionesPage::nuevaAsignacion);
    connect(ui->btnRevocar, &QPushButton::clicked, this, &AsignacionesPage::revocarAsignacion);
    connect(ui->btnRefrescar, &QPushButton::clicked, this, &AsignacionesPage::refrescar);

    ui->btnNuevo->setIcon(Iconos::nuevo());
    ui->btnRevocar->setIcon(Iconos::eliminar());
    ui->btnRefrescar->setIcon(Iconos::refrescar());

    refrescar();
}

AsignacionesPage::~AsignacionesPage() {
    delete ui;
}

void AsignacionesPage::refrescar() {
    m_model->setQuery(
        "SELECT a.id_asignacion AS 'ID', CONCAT(e.nombre, ' ', COALESCE(e.apellido_paterno, '')) AS 'Empleado', "
        "so.nombre AS 'Sucursal de origen', sd.nombre AS 'Puede operar en', "
        "a.fecha_inicio AS 'Desde', a.fecha_fin AS 'Hasta', "
        "CASE WHEN a.estado = 'revocada' THEN 'revocada' "
        "WHEN CURDATE() BETWEEN a.fecha_inicio AND a.fecha_fin THEN 'vigente' "
        "WHEN CURDATE() < a.fecha_inicio THEN 'programada' ELSE 'vencida' END AS 'Estado', "
        "COALESCE(u.nombre_usuario, '—') AS 'Autorizó' "
        "FROM Asignaciones_Temporales a "
        "JOIN Empleados e ON e.id_empleado = a.id_empleado "
        "JOIN Sucursales so ON so.id_sucursal = e.id_sucursal "
        "JOIN Sucursales sd ON sd.id_sucursal = a.id_sucursal "
        "LEFT JOIN Usuarios u ON u.id_usuario = a.id_autoriza "
        "ORDER BY a.fecha_inicio DESC");
    ui->tableView->hideColumn(0);
    ui->tableView->resizeColumnsToContents();
}

void AsignacionesPage::nuevaAsignacion() {
    AsignacionDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted)
        refrescar();
}

void AsignacionesPage::revocarAsignacion() {
    auto seleccion = ui->tableView->selectionModel()->selectedRows();
    if (seleccion.isEmpty()) {
        QMessageBox::information(this, "Revocar", "Selecciona una asignación de la lista.");
        return;
    }
    int id = m_model->data(m_model->index(seleccion.first().row(), 0)).toInt();
    if (QMessageBox::question(this, "Confirmar", "¿Revocar la asignación seleccionada?") != QMessageBox::Yes)
        return;

    QSqlQuery query;
    query.prepare("UPDATE Asignaciones_Temporales SET estado = 'revocada' WHERE id_asignacion = :id");
    query.bindValue(":id", id);
    if (!query.exec()) {
        QMessageBox::critical(this, "Error", "No se pudo revocar:\n" + query.lastError().text());
        return;
    }
    refrescar();
}
