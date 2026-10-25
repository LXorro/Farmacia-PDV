#include "iniciopage.h"
#include "ui_iniciopage.h"
#include "../database/sesionactual.h"
#include "../util/iconos.h"

#include <QIcon>
#include <QPushButton>
#include <QSqlQuery>

InicioPage::InicioPage(QWidget *parent) : QWidget(parent), ui(new Ui::InicioPage) {
    ui->setupUi(this);

    const SesionActual &sesion = SesionActual::instance();
    ui->labelBienvenida->setText("Bienvenido/a, " + sesion.nombreUsuario());
    ui->labelSubtitulo->setText(sesion.esAdmin()
                                     ? "Administrador — todas las sucursales"
                                     : "Empleado — " + sesion.nombreSucursal());

    // Logo (proporción 667x150 del SVG).
    const int anchoLogo = 420;
    ui->labelLogo->setPixmap(QIcon(":/logo.svg").pixmap(QSize(anchoLogo, anchoLogo * 150 / 667)));

    ui->btnPuntoVenta->setIcon(Iconos::puntoDeVenta());
    ui->btnProductos->setIcon(Iconos::productos());
    ui->btnClientes->setIcon(Iconos::clientes());
    ui->btnTickets->setIcon(Iconos::tickets());
    ui->btnInventario->setIcon(Iconos::inventario());
    ui->btnCategorias->setIcon(Iconos::categorias());
    ui->btnProveedores->setIcon(Iconos::proveedores());
    ui->btnEmpleados->setIcon(Iconos::empleados());
    ui->btnSucursales->setIcon(Iconos::sucursales());
    ui->btnUsuarios->setIcon(Iconos::usuarios());

    // Los accesos de administración solo existen para admin; el resto del
    // sistema sigue la misma regla (ver Plan_Proyecto.md, sección "Autenticación y roles").
    if (!sesion.esAdmin()) {
        ui->btnProveedores->setVisible(false);
        ui->btnEmpleados->setVisible(false);
        ui->btnSucursales->setVisible(false);
        ui->btnUsuarios->setVisible(false);
    }

    connect(ui->btnPuntoVenta, &QPushButton::clicked, this, [this]() { emit abrirPagina("Punto de Venta"); });
    connect(ui->btnProductos, &QPushButton::clicked, this, [this]() { emit abrirPagina("Productos"); });
    connect(ui->btnClientes, &QPushButton::clicked, this, [this]() { emit abrirPagina("Clientes"); });
    connect(ui->btnTickets, &QPushButton::clicked, this, [this]() { emit abrirPagina("Tickets"); });
    connect(ui->btnInventario, &QPushButton::clicked, this, [this]() { emit abrirPagina("Inventario"); });
    connect(ui->btnCategorias, &QPushButton::clicked, this, [this]() { emit abrirPagina("Categorías"); });
    connect(ui->btnProveedores, &QPushButton::clicked, this, [this]() { emit abrirPagina("Proveedores"); });
    connect(ui->btnEmpleados, &QPushButton::clicked, this, [this]() { emit abrirPagina("Empleados"); });
    connect(ui->btnSucursales, &QPushButton::clicked, this, [this]() { emit abrirPagina("Sucursales"); });
    connect(ui->btnUsuarios, &QPushButton::clicked, this, [this]() { emit abrirPagina("Usuarios"); });

    cargarEstadisticas();
}

InicioPage::~InicioPage() {
    delete ui;
}

void InicioPage::refrescar() {
    cargarEstadisticas();
}

void InicioPage::cargarEstadisticas() {
    const SesionActual &sesion = SesionActual::instance();
    QString filtroSucursal = sesion.esAdmin() ? "" : QString("AND id_sucursal = %1").arg(sesion.idSucursal());

    QSqlQuery ventasHoy;
    ventasHoy.exec(QString(
        "SELECT COALESCE(SUM(total), 0) FROM Tickets "
        "WHERE DATE(fecha_venta) = CURDATE() AND estado_ticket = 'completado' %1")
        .arg(filtroSucursal));
    if (ventasHoy.next())
        ui->labelVentasHoy->setText(QString("$%1").arg(ventasHoy.value(0).toDouble(), 0, 'f', 2));

    QSqlQuery pendientes;
    pendientes.exec(QString(
        "SELECT COUNT(*) FROM Tickets "
        "WHERE estado_pago IN ('pendiente', 'parcial') AND estado_ticket = 'completado' %1")
        .arg(filtroSucursal));
    if (pendientes.next())
        ui->labelPendientes->setText(pendientes.value(0).toString());

    QString filtroSucursalLotes = sesion.esAdmin() ? "" : QString("AND id_sucursal = %1").arg(sesion.idSucursal());
    QSqlQuery caducidad;
    caducidad.exec(QString(
        "SELECT COUNT(*) FROM Lotes "
        "WHERE cantidad_disponible > 0 AND fecha_caducidad <= DATE_ADD(CURDATE(), INTERVAL 30 DAY) %1")
        .arg(filtroSucursalLotes));
    if (caducidad.next())
        ui->labelCaducidad->setText(caducidad.value(0).toString());
}
