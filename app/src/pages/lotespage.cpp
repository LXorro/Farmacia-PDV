#include "lotespage.h"
#include "ui_lotespage.h"
#include "../dialogs/lotedialog.h"
#include "../database/sesionactual.h"
#include "../util/iconos.h"

#include <QHeaderView>
#include <QMessageBox>
#include <QSqlQueryModel>

LotesPage::LotesPage(QWidget *parent) : QWidget(parent), ui(new Ui::LotesPage) {
    ui->setupUi(this);

    m_model = new QSqlQueryModel(this);

    ui->tableView->setModel(m_model);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView->setAlternatingRowColors(true);
    ui->tableView->verticalHeader()->setVisible(false);
    ui->tableView->horizontalHeader()->setStretchLastSection(true);

    connect(ui->btnNuevo, &QPushButton::clicked, this, &LotesPage::nuevoLote);
    connect(ui->btnEditar, &QPushButton::clicked, this, &LotesPage::editarLote);
    connect(ui->btnRefrescar, &QPushButton::clicked, this, &LotesPage::refrescar);

    ui->btnNuevo->setIcon(Iconos::nuevo());
    ui->btnEditar->setIcon(Iconos::editar());
    ui->btnRefrescar->setIcon(Iconos::refrescar());

    refrescar();
}

LotesPage::~LotesPage() {
    delete ui;
}

void LotesPage::refrescar() {
    QString sql =
        "SELECT l.id_lote AS 'ID', p.nombre AS 'Producto', s.nombre AS 'Sucursal', "
        "l.numero_lote AS 'N.º de lote', l.fecha_caducidad AS 'Caducidad', "
        "l.cantidad_disponible AS 'Disponible', l.estado AS 'Estado' "
        "FROM Lotes l "
        "JOIN Productos p ON p.id_producto = l.id_producto "
        "JOIN Sucursales s ON s.id_sucursal = l.id_sucursal ";

    if (!SesionActual::instance().esAdmin())
        sql += QString("WHERE l.id_sucursal = %1 ").arg(SesionActual::instance().idSucursal());

    sql += "ORDER BY l.fecha_caducidad ASC";

    m_model->setQuery(sql);
    ui->tableView->hideColumn(0);  // ID: catálogo puro, no aporta al usuario
    ui->tableView->resizeColumnsToContents();
}

void LotesPage::nuevoLote() {
    LoteDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted)
        refrescar();
}

void LotesPage::editarLote() {
    auto seleccion = ui->tableView->selectionModel()->selectedRows();
    if (seleccion.isEmpty()) {
        QMessageBox::information(this, "Editar", "Selecciona un lote de la lista.");
        return;
    }
    int id = m_model->data(m_model->index(seleccion.first().row(), 0)).toInt();

    LoteDialog dialog(this);
    dialog.setLoteId(id);
    if (dialog.exec() == QDialog::Accepted)
        refrescar();
}
