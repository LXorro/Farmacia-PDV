#include "ticketspage.h"
#include "ui_ticketspage.h"
#include "../dialogs/ticketdetalledialog.h"
#include "../database/sesionactual.h"
#include "../util/iconos.h"
#include "../util/tablas.h"

#include <QDate>
#include <QHeaderView>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlQueryModel>

TicketsPage::TicketsPage(QWidget *parent) : QWidget(parent), ui(new Ui::TicketsPage) {
    ui->setupUi(this);

    m_model = new QSqlQueryModel(this);
    m_modelDevoluciones = new QSqlQueryModel(this);

    ui->tableView->setModel(m_model);
    Tablas::aplicar(ui->tableView, "No hay tickets para esta fecha.");
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView->setAlternatingRowColors(true);
    ui->tableView->verticalHeader()->setVisible(false);
    ui->tableView->horizontalHeader()->setStretchLastSection(true);

    ui->tableDevoluciones->setModel(m_modelDevoluciones);
    Tablas::aplicar(ui->tableDevoluciones, "No hay devoluciones para esta fecha.");
    ui->tableDevoluciones->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableDevoluciones->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableDevoluciones->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableDevoluciones->setAlternatingRowColors(true);
    ui->tableDevoluciones->verticalHeader()->setVisible(false);
    ui->tableDevoluciones->horizontalHeader()->setStretchLastSection(true);

    ui->dateFiltro->setDisplayFormat("dd/MM/yyyy");
    ui->dateFiltro->setDate(QDate::currentDate());

    connect(ui->btnVerDetalle, &QPushButton::clicked, this, &TicketsPage::verDetalle);
    connect(ui->tableView, &QTableView::doubleClicked, this, &TicketsPage::verDetalle);
    connect(ui->btnRefrescar, &QPushButton::clicked, this, &TicketsPage::refrescar);
    connect(ui->dateFiltro, &QDateEdit::dateChanged, this, &TicketsPage::refrescar);
    connect(ui->checkTodos, &QCheckBox::toggled, this, [this](bool todos) {
        ui->dateFiltro->setEnabled(!todos);
        refrescar();
    });
    connect(ui->btnHoy, &QPushButton::clicked, this, [this]() {
        ui->checkTodos->setChecked(false);
        ui->dateFiltro->setDate(QDate::currentDate());  // ya dispara refrescar() por dateChanged
    });

    ui->btnVerDetalle->setIcon(Iconos::verDetalle());
    ui->btnRefrescar->setIcon(Iconos::refrescar());

    refrescar();
}

TicketsPage::~TicketsPage() {
    delete ui;
}

void TicketsPage::refrescar() {
    QString sql =
        "SELECT t.id_ticket AS 'Ticket', s.nombre AS 'Sucursal', "
        "CONCAT(e.nombre, ' ', e.apellido_paterno) AS 'Empleado', "
        "COALESCE(CONCAT(c.nombre, ' ', c.apellido_paterno), 'Mostrador') AS 'Cliente', "
        "t.fecha_venta AS 'Fecha', t.total AS 'Total', t.modalidad_pago AS 'Modalidad', "
        "t.estado_pago AS 'Estado de pago', t.estado_ticket AS 'Estado' "
        "FROM Tickets t "
        "JOIN Sucursales s ON s.id_sucursal = t.id_sucursal "
        "JOIN Empleados e ON e.id_empleado = t.id_empleado "
        "LEFT JOIN Clientes c ON c.id_cliente = t.id_cliente ";

    QStringList condiciones;
    if (!SesionActual::instance().esAdmin())
        condiciones << QString("t.id_sucursal = %1").arg(SesionActual::instance().idSucursal());
    if (!ui->checkTodos->isChecked())
        condiciones << "DATE(t.fecha_venta) = :fecha";

    if (!condiciones.isEmpty())
        sql += "WHERE " + condiciones.join(" AND ") + " ";

    sql += "ORDER BY t.fecha_venta DESC";

    QSqlQuery query;
    query.prepare(sql);
    if (!ui->checkTodos->isChecked())
        query.bindValue(":fecha", ui->dateFiltro->date());
    query.exec();

    m_model->setQuery(std::move(query));
    ui->tableView->resizeColumnsToContents();

    QString sqlDev =
        "SELECT d.id_devolucion AS 'Devolución', d.id_ticket AS 'Ticket', s.nombre AS 'Sucursal', "
        "CONCAT(e.nombre, ' ', e.apellido_paterno) AS 'Empleado', d.fecha_devolucion AS 'Fecha', "
        "d.motivo AS 'Motivo', d.total_devuelto AS 'Total devuelto', "
        "d.tipo_reembolso AS 'Tipo de reembolso', d.estado AS 'Estado' "
        "FROM Devoluciones d "
        "JOIN Sucursales s ON s.id_sucursal = d.id_sucursal "
        "JOIN Empleados e ON e.id_empleado = d.id_empleado ";

    QStringList condicionesDev;
    if (!SesionActual::instance().esAdmin())
        condicionesDev << QString("d.id_sucursal = %1").arg(SesionActual::instance().idSucursal());
    if (!ui->checkTodos->isChecked())
        condicionesDev << "DATE(d.fecha_devolucion) = :fecha";

    if (!condicionesDev.isEmpty())
        sqlDev += "WHERE " + condicionesDev.join(" AND ") + " ";

    sqlDev += "ORDER BY d.fecha_devolucion DESC";

    QSqlQuery queryDev;
    queryDev.prepare(sqlDev);
    if (!ui->checkTodos->isChecked())
        queryDev.bindValue(":fecha", ui->dateFiltro->date());
    queryDev.exec();

    m_modelDevoluciones->setQuery(std::move(queryDev));
    ui->tableDevoluciones->resizeColumnsToContents();
}

void TicketsPage::verDetalle() {
    auto seleccion = ui->tableView->selectionModel()->selectedRows();
    if (seleccion.isEmpty()) {
        QMessageBox::information(this, "Detalle", "Selecciona un ticket de la lista.");
        return;
    }
    int idTicket = m_model->data(m_model->index(seleccion.first().row(), 0)).toInt();

    TicketDetalleDialog dialog(this);
    dialog.setTicketId(idTicket);
    dialog.exec();
    refrescar();
}
