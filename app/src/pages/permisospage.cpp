#include "permisospage.h"
#include "ui_permisospage.h"
#include "../database/sesionactual.h"
#include "../util/iconos.h"
#include "../util/modeloutc.h"
#include "../util/tablas.h"

#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QVBoxLayout>

PermisosPage::PermisosPage(QWidget *parent) : QWidget(parent), ui(new Ui::PermisosPage) {
    ui->setupUi(this);

    m_model = new ModeloUtc(this);
    m_model->setColumnasUtc({4, 5});   // Desde, Hasta

    ui->tableView->setModel(m_model);
    Tablas::aplicar(ui->tableView, "No hay permisos temporales.");
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView->setAlternatingRowColors(true);
    ui->tableView->verticalHeader()->setVisible(false);
    ui->tableView->horizontalHeader()->setStretchLastSection(true);

    connect(ui->btnNuevo, &QPushButton::clicked, this, &PermisosPage::nuevoPermiso);
    connect(ui->btnRevocar, &QPushButton::clicked, this, &PermisosPage::revocarPermiso);
    connect(ui->btnRefrescar, &QPushButton::clicked, this, &PermisosPage::refrescar);

    ui->btnNuevo->setIcon(Iconos::nuevo());
    ui->btnRevocar->setIcon(Iconos::eliminar());
    ui->btnRefrescar->setIcon(Iconos::refrescar());

    refrescar();
}

PermisosPage::~PermisosPage() {
    delete ui;
}

void PermisosPage::refrescar() {
    m_model->setQuery(
        "SELECT p.id_permiso AS 'ID', u.nombre_usuario AS 'Usuario', "
        "CONCAT(e.nombre, ' ', COALESCE(e.apellido_paterno, '')) AS 'Empleado', "
        "COALESCE(s.nombre, '—') AS 'Sucursal', p.fecha_inicio AS 'Desde', p.fecha_fin AS 'Hasta', "
        "CASE WHEN p.estado = 'revocado' THEN 'revocado' "
        "WHEN UTC_TIMESTAMP() > p.fecha_fin THEN 'vencido' ELSE 'vigente' END AS 'Estado', "
        "COALESCE(a.nombre_usuario, '—') AS 'Autorizó' "
        "FROM Permisos_Temporales p "
        "JOIN Usuarios u ON u.id_usuario = p.id_usuario "
        "LEFT JOIN Empleados e ON e.id_empleado = u.id_empleado "
        "LEFT JOIN Sucursales s ON s.id_sucursal = e.id_sucursal "
        "LEFT JOIN Usuarios a ON a.id_usuario = p.id_autoriza "
        "ORDER BY p.fecha_inicio DESC");
    ui->tableView->hideColumn(0);
    ui->tableView->resizeColumnsToContents();
}

void PermisosPage::nuevoPermiso() {
    QDialog dialogo(this);
    dialogo.setWindowTitle("Conceder permiso temporal");

    auto *usuarios = new QComboBox(&dialogo);
    QSqlQuery lista("SELECT u.id_usuario, u.nombre_usuario, e.nombre, e.apellido_paterno, s.nombre "
                    "FROM Usuarios u "
                    "JOIN Empleados e ON e.id_empleado = u.id_empleado "
                    "JOIN Sucursales s ON s.id_sucursal = e.id_sucursal "
                    "WHERE u.rol = 'empleado' AND u.estado = 'activo' AND e.estado = 'activo' "
                    "ORDER BY u.nombre_usuario");
    while (lista.next())
        usuarios->addItem(QString("%1 — %2 %3 (%4)").arg(lista.value(1).toString(), lista.value(2).toString(),
                                                        lista.value(3).toString(), lista.value(4).toString()),
                          lista.value(0));
    if (usuarios->count() == 0) {
        QMessageBox::information(this, "Conceder permiso", "No hay empleados activos con cuenta de usuario.");
        return;
    }

    auto *duracion = new QComboBox(&dialogo);
    duracion->addItem("1 día (24 horas)", 24);
    duracion->addItem("2 días (48 horas)", 48);
    duracion->addItem("3 días (72 horas)", 72);

    auto *formulario = new QFormLayout;
    formulario->addRow("Empleado:", usuarios);
    formulario->addRow("Duración:", duracion);
    auto *botones = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, &dialogo);
    connect(botones, &QDialogButtonBox::accepted, &dialogo, &QDialog::accept);
    connect(botones, &QDialogButtonBox::rejected, &dialogo, &QDialog::reject);
    auto *capa = new QVBoxLayout(&dialogo);
    capa->addWidget(new QLabel("Rol «encargado»: precios de su sucursal y alta/edición de lotes.", &dialogo));
    capa->addLayout(formulario);
    capa->addWidget(botones);
    if (dialogo.exec() != QDialog::Accepted)
        return;

    const int idUsuario = usuarios->currentData().toInt();
    QSqlQuery vigente;
    vigente.prepare("SELECT 1 FROM Permisos_Temporales WHERE id_usuario = :u AND estado = 'vigente' "
                    "AND UTC_TIMESTAMP() <= fecha_fin LIMIT 1");
    vigente.bindValue(":u", idUsuario);
    if (vigente.exec() && vigente.next()) {
        QMessageBox::warning(this, "Conceder permiso",
                             "Ese empleado ya tiene un permiso vigente. Revócalo primero si quieres cambiarlo.");
        return;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO Permisos_Temporales (id_usuario, fecha_inicio, fecha_fin, id_autoriza) "
                  "VALUES (:u, UTC_TIMESTAMP(), DATE_ADD(UTC_TIMESTAMP(), INTERVAL :horas HOUR), :autoriza)");
    query.bindValue(":u", idUsuario);
    query.bindValue(":horas", duracion->currentData().toInt());
    const int idAdmin = SesionActual::instance().idUsuario();
    query.bindValue(":autoriza", idAdmin >= 0 ? QVariant(idAdmin) : QVariant(QMetaType(QMetaType::Int)));
    if (!query.exec()) {
        QMessageBox::critical(this, "Error", "No se pudo conceder el permiso:\n" + query.lastError().text());
        return;
    }
    refrescar();
}

void PermisosPage::revocarPermiso() {
    auto seleccion = ui->tableView->selectionModel()->selectedRows();
    if (seleccion.isEmpty()) {
        QMessageBox::information(this, "Revocar", "Selecciona un permiso de la lista.");
        return;
    }
    const int id = m_model->data(m_model->index(seleccion.first().row(), 0)).toInt();
    if (QMessageBox::question(this, "Confirmar", "¿Revocar el permiso seleccionado?") != QMessageBox::Yes)
        return;

    QSqlQuery query;
    query.prepare("UPDATE Permisos_Temporales SET estado = 'revocado' WHERE id_permiso = :id");
    query.bindValue(":id", id);
    if (!query.exec()) {
        QMessageBox::critical(this, "Error", "No se pudo revocar:\n" + query.lastError().text());
        return;
    }
    refrescar();
}
