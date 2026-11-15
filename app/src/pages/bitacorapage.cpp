#include "bitacorapage.h"
#include "ui_bitacorapage.h"
#include "../database/sesionactual.h"
#include "../util/iconos.h"
#include "../util/modeloutc.h"
#include "../util/tablas.h"

#include <QHeaderView>

BitacoraPage::BitacoraPage(QWidget *parent) : QWidget(parent), ui(new Ui::BitacoraPage) {
    ui->setupUi(this);

    m_model = new ModeloUtc(this);
    m_model->setColumnasUtc({0});   // Fecha

    ui->tableView->setModel(m_model);
    Tablas::aplicar(ui->tableView, "Aún no hay cambios registrados.");
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView->setAlternatingRowColors(true);
    ui->tableView->verticalHeader()->setVisible(false);
    ui->tableView->horizontalHeader()->setStretchLastSection(true);

    connect(ui->btnRefrescar, &QPushButton::clicked, this, &BitacoraPage::refrescar);
    ui->btnRefrescar->setIcon(Iconos::refrescar());

    refrescar();
}

BitacoraPage::~BitacoraPage() {
    delete ui;
}

void BitacoraPage::refrescar() {
    QString sql =
        "SELECT b.fecha AS 'Fecha', s.nombre AS 'Sucursal', b.nombre_usuario AS 'Usuario', "
        "CASE b.accion WHEN 'precio_sucursal' THEN 'Precio en sucursal' WHEN 'lote_alta' THEN 'Lote nuevo' "
        "WHEN 'lote_edicion' THEN 'Lote editado' ELSE b.accion END AS 'Acción', "
        "b.detalle AS 'Detalle', "
        "CASE WHEN b.con_permiso THEN 'permiso temporal' ELSE 'administrador' END AS 'Autorización' "
        "FROM Bitacora_Cambios b JOIN Sucursales s ON s.id_sucursal = b.id_sucursal ";
    // En el equipo de una sucursal (sin ver el resto) solo se muestran los cambios de esa sucursal.
    if (!SesionActual::instance().veTodasLasSucursales())
        sql += QString("WHERE b.id_sucursal = %1 ").arg(SesionActual::instance().idSucursal());
    sql += "ORDER BY b.fecha DESC, b.id_bitacora DESC LIMIT 500";
    m_model->setQuery(sql);
    ui->tableView->resizeColumnsToContents();
}
