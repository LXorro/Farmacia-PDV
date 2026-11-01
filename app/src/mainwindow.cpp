#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QListWidgetItem>
#include <QMetaObject>
#include <QSize>

#include "database/sesionactual.h"
#include "util/iconos.h"
#include "pages/categoriaspage.h"
#include "pages/clientespage.h"
#include "pages/empleadospage.h"
#include "pages/iniciopage.h"
#include "pages/lotespage.h"
#include "pages/productospage.h"
#include "pages/proveedorespage.h"
#include "pages/puntoventapage.h"
#include "pages/sucursalespage.h"
#include "pages/ticketspage.h"
#include "pages/usuariospage.h"
#include "pages/asignacionespage.h"
#include "database/databasemanager.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    // Terminal de punto de venta: sin barra de título del sistema, siempre a
    // pantalla completa (sin modo ventana) — barra propia con solo
    // minimizar y cerrar, ver showFullScreen() en main.cpp.
    setWindowFlag(Qt::FramelessWindowHint, true);
    ui->btnMinimizar->setIcon(Iconos::minimizar());
    ui->btnCerrarApp->setIcon(Iconos::cerrar());
    connect(ui->btnMinimizar, &QPushButton::clicked, this, &QWidget::showMinimized);
    connect(ui->btnCerrarApp, &QPushButton::clicked, this, &QWidget::close);

    const SesionActual &sesion = SesionActual::instance();
    const auto funcion = DatabaseManager::funcion();
    // Cada tabla tiene un único nodo escritor (el permiso también está en la base): en el central lo
    // operativo se consulta pero no se edita; en una sucursal, lo que baja del central.
    auto soloLectura = [](QWidget *pagina) {
        for (const char *nombre : {"btnNuevo", "btnEditar", "btnEliminar", "btnRevocar"})
            if (auto *boton = pagina->findChild<QPushButton *>(nombre))
                boton->setEnabled(false);
        return pagina;
    };
    const bool central = funcion == DatabaseManager::Funcion::Central;
    const bool sucursal = funcion == DatabaseManager::Funcion::Sucursal;

    ui->listMenu->setIconSize(QSize(22, 22));
    ui->listMenu->setSpacing(1);
    ui->listMenu->setUniformItemSizes(true);
    ui->listMenu->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    auto *inicio = new InicioPage(this);
    connect(inicio, &InicioPage::abrirPagina, this, &MainWindow::irAPagina);
    ui->listMenu->addItem(new QListWidgetItem(Iconos::inicio(), "Inicio"));
    ui->stackedWidget->addWidget(inicio);

    // Pantallas operativas: visibles para empleado (limitadas a su sucursal
    // dentro de cada página) y para admin (sin restricción).
    // El central no vende: solo consulta lo que venden las sucursales.
    if (!central) {
        ui->listMenu->addItem(new QListWidgetItem(Iconos::puntoDeVenta(), "Punto de Venta"));
        ui->stackedWidget->addWidget(new PuntoVentaPage(this));
    }

    ui->listMenu->addItem(new QListWidgetItem(Iconos::productos(), "Productos"));
    ui->stackedWidget->addWidget(new ProductosPage(this));

    ui->listMenu->addItem(new QListWidgetItem(Iconos::clientes(), "Clientes"));
    ui->stackedWidget->addWidget(central ? soloLectura(new ClientesPage(this)) : new ClientesPage(this));

    ui->listMenu->addItem(new QListWidgetItem(Iconos::tickets(), "Tickets"));
    ui->stackedWidget->addWidget(new TicketsPage(this));

    ui->listMenu->addItem(new QListWidgetItem(Iconos::inventario(), "Inventario"));
    ui->stackedWidget->addWidget(central ? soloLectura(new LotesPage(this)) : new LotesPage(this));

    ui->listMenu->addItem(new QListWidgetItem(Iconos::categorias(), "Categorías"));
    ui->stackedWidget->addWidget(new CategoriasPage(this));

    // Pantallas de administración: solo admin.
    if (sesion.esAdmin()) {
        ui->listMenu->addItem(new QListWidgetItem(Iconos::proveedores(), "Proveedores"));
        ui->stackedWidget->addWidget(sucursal ? soloLectura(new ProveedoresPage(this)) : new ProveedoresPage(this));

        ui->listMenu->addItem(new QListWidgetItem(Iconos::empleados(), "Empleados"));
        ui->stackedWidget->addWidget(sucursal ? soloLectura(new EmpleadosPage(this)) : new EmpleadosPage(this));

        ui->listMenu->addItem(new QListWidgetItem(Iconos::sucursales(), "Sucursales"));
        ui->stackedWidget->addWidget(sucursal ? soloLectura(new SucursalesPage(this)) : new SucursalesPage(this));

        ui->listMenu->addItem(new QListWidgetItem(Iconos::usuarios(), "Usuarios"));
        ui->stackedWidget->addWidget(new UsuariosPage(this));

        // Asignaciones temporales entre sucursales: solo existen en una instalación con central.
        if (central || sucursal) {
            ui->listMenu->addItem(new QListWidgetItem(Iconos::sucursales(), "Asignaciones"));
            auto *asignaciones = new AsignacionesPage(this);
            ui->stackedWidget->addWidget(sucursal ? soloLectura(asignaciones) : asignaciones);
        }
    }

    connect(ui->listMenu, &QListWidget::currentRowChanged, this, [this](int row) {
        ui->stackedWidget->setCurrentIndex(row);
        // Cada página expone su propio slot "refrescar()" (no comparten una
        // interfaz común) — se invoca por nombre para traer datos al día
        // cada vez que se entra a la pestaña, sin tener que darle "Refrescar"
        // a mano (p. ej. un ticket recién cobrado en Punto de Venta debe
        // verse de inmediato al entrar a Tickets).
        QMetaObject::invokeMethod(ui->stackedWidget->currentWidget(), "refrescar");
    });
    ui->listMenu->setCurrentRow(0);

    QString alcance = sesion.veTodasLasSucursales() ? "todas las sucursales" : sesion.nombreSucursal();
    QString titulo = QString("Farmacia — %1 (%2) — %3").arg(sesion.nombreUsuario(), sesion.rol(), alcance);
    setWindowTitle(titulo);       // para la barra de tareas / alt-tab
    ui->labelTituloApp->setText(titulo);  // visible en la barra propia
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::irAPagina(const QString &nombrePagina) {
    auto items = ui->listMenu->findItems(nombrePagina, Qt::MatchExactly);
    if (!items.isEmpty())
        ui->listMenu->setCurrentItem(items.first());
}
