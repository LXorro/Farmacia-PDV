#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QColor>
#include <QDateTime>
#include <QFont>
#include <QHash>
#include <QLabel>
#include <QListWidgetItem>
#include <QMetaObject>
#include <QSize>
#include <QStatusBar>
#include <QTimer>

#include "database/sesionactual.h"
#include "util/iconos.h"
#include "pages/categoriaspage.h"
#include "pages/clientespage.h"
#include "pages/empleadospage.h"
#include "pages/iniciopage.h"
#include "pages/lotespage.h"
#include "pages/productospage.h"
#include "pages/reportediariopage.h"
#include "pages/proveedorespage.h"
#include "pages/puntoventapage.h"
#include "pages/sucursalespage.h"
#include "pages/ticketspage.h"
#include "pages/usuariospage.h"
#include "pages/asignacionespage.h"
#include "pages/bitacorapage.h"
#include "pages/permisospage.h"
#include "database/databasemanager.h"
#include "database/estadosync.h"

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
    ui->listMenu->setUniformItemSizes(false);   // las secciones son más bajas que las pantallas
    ui->listMenu->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    // El menú ya no coincide 1 a 1 con el orden de las páginas (hay secciones): cada entrada guarda el
    // índice de su página en el QStackedWidget.
    auto agregarSeccion = [this](const QString &texto) {
        auto *item = new QListWidgetItem(texto.toUpper());
        item->setFlags(Qt::NoItemFlags);   // no seleccionable ni enfocable con el teclado
        QFont fuente = item->font();
        fuente.setPixelSize(11);
        fuente.setBold(true);
        fuente.setLetterSpacing(QFont::AbsoluteSpacing, 1.2);
        item->setFont(fuente);
        item->setSizeHint(QSize(0, 34));
        ui->listMenu->addItem(item);
    };
    auto agregarPagina = [this](const QIcon &icono, const QString &texto, QWidget *pagina) {
        auto *item = new QListWidgetItem(icono, texto);
        item->setData(Qt::UserRole, ui->stackedWidget->addWidget(pagina));
        item->setSizeHint(QSize(0, 40));
        ui->listMenu->addItem(item);
        return pagina;
    };

    auto *inicio = new InicioPage(this);
    connect(inicio, &InicioPage::abrirPagina, this, &MainWindow::irAPagina);
    agregarPagina(Iconos::inicio(), "Inicio", inicio);

    // Pantallas operativas: visibles para empleado (limitadas a su sucursal dentro de cada página) y para
    // admin (sin restricción). El central no vende: solo consulta lo que venden las sucursales.
    agregarSeccion("Ventas");
    if (!central)
        agregarPagina(Iconos::puntoDeVenta(), "Punto de Venta", new PuntoVentaPage(this));
    agregarPagina(Iconos::clientes(), "Clientes", central ? soloLectura(new ClientesPage(this)) : new ClientesPage(this));
    agregarPagina(Iconos::tickets(), "Tickets", new TicketsPage(this));
    agregarPagina(Iconos::tickets(), "Reporte diario", new ReporteDiarioPage(this));

    agregarSeccion("Inventario");
    agregarPagina(Iconos::productos(), "Productos", new ProductosPage(this));
    agregarPagina(Iconos::inventario(), "Inventario", central ? soloLectura(new LotesPage(this)) : new LotesPage(this));
    agregarPagina(Iconos::categorias(), "Categorías", new CategoriasPage(this));

    // Pantallas de administración: solo admin.
    if (sesion.esAdmin()) {
        agregarPagina(Iconos::proveedores(), "Proveedores",
                      sucursal ? soloLectura(new ProveedoresPage(this)) : new ProveedoresPage(this));

        agregarSeccion("Administración");
        agregarPagina(Iconos::empleados(), "Empleados", sucursal ? soloLectura(new EmpleadosPage(this)) : new EmpleadosPage(this));
        agregarPagina(Iconos::sucursales(), "Sucursales", sucursal ? soloLectura(new SucursalesPage(this)) : new SucursalesPage(this));
        agregarPagina(Iconos::usuarios(), "Usuarios", new UsuariosPage(this));
        // Permisos temporales (rol «encargado», 1-3 días): los concede el admin desde el central.
        agregarPagina(Iconos::usuarios(), "Permisos", sucursal ? soloLectura(new PermisosPage(this)) : new PermisosPage(this));
        // Asignaciones temporales entre sucursales: solo existen en una instalación con central.
        if (central || sucursal)
            agregarPagina(Iconos::sucursales(), "Asignaciones",
                          sucursal ? soloLectura(new AsignacionesPage(this)) : new AsignacionesPage(this));
        agregarPagina(Iconos::tickets(), "Bitácora", new BitacoraPage(this));
    }

    // Texto de ayuda bajo el título de cada pantalla. Las que ya traían su propia ayuda («labelAyuda») la
    // aportan aquí y la ocultan en la pantalla, para no repetirla.
    m_ayudaPagina = {
        {"Inicio", "Resumen del día y accesos directos."},
        {"Punto de Venta", "Arma el carrito, elige la forma de pago y cobra."},
        {"Clientes", "Clientes registrados en la sucursal."},
        {"Tickets", "Historial de ventas. Abre un ticket para ver sus líneas, abonos y devoluciones."},
        {"Reporte diario", "Movimientos del día por sucursal. Se puede exportar a PDF."},
        {"Productos", "Catálogo y existencias. El precio de cada sucursal lo ajusta quien tenga permiso."},
        {"Inventario", "Lotes con su caducidad, existencias y costo."},
        {"Categorías", "Clasificación de los productos del catálogo."},
        {"Proveedores", "Empresas que surten a la farmacia."},
        {"Empleados", "Personal de cada sucursal."},
        {"Sucursales", "Sucursales de la farmacia."},
        {"Usuarios", "Cuentas de acceso a la aplicación."},
    };
    for (int fila = 0; fila < ui->listMenu->count(); ++fila) {
        const QListWidgetItem *item = ui->listMenu->item(fila);
        if (!item->data(Qt::UserRole).isValid())
            continue;
        QWidget *pagina = ui->stackedWidget->widget(item->data(Qt::UserRole).toInt());
        if (auto *ayuda = pagina->findChild<QLabel *>("labelAyuda")) {
            m_ayudaPagina.insert(item->text(), ayuda->text());
            ayuda->hide();
        }
    }

    connect(ui->listMenu, &QListWidget::currentItemChanged, this, [this](QListWidgetItem *actual, QListWidgetItem *) {
        if (!actual || !actual->data(Qt::UserRole).isValid())
            return;   // una sección: no es una pantalla
        ui->stackedWidget->setCurrentIndex(actual->data(Qt::UserRole).toInt());
        // El Inicio ya trae su propio saludo; el encabezado se reserva para las demás pantallas.
        ui->encabezadoPagina->setVisible(actual->text() != "Inicio");
        ui->labelTituloPagina->setText(actual->text());
        ui->labelSubtituloPagina->setText(m_ayudaPagina.value(actual->text()));
        ui->labelSubtituloPagina->setVisible(!ui->labelSubtituloPagina->text().isEmpty());
        // Cada página expone su propio slot "refrescar()" (no comparten una
        // interfaz común) — se invoca por nombre para traer datos al día
        // cada vez que se entra a la pestaña, sin tener que darle "Refrescar"
        // a mano (p. ej. un ticket recién cobrado en Punto de Venta debe
        // verse de inmediato al entrar a Tickets).
        QMetaObject::invokeMethod(ui->stackedWidget->currentWidget(), "refrescar");
    });
    ui->listMenu->setCurrentRow(0);

    setWindowTitle(QString("Farmacia — %1 (%2) — %3")
                       .arg(sesion.nombreUsuario(), sesion.rol(),
                            sesion.veTodasLasSucursales() ? "todas las sucursales" : sesion.nombreSucursal()));  // barra de tareas / alt-tab
    ui->labelTituloApp->setText("Farmacia — " + sesion.nombreUsuario());

    // Insignia de sesión (rol, permiso temporal y estado de la réplica): se refresca cada 10 s, así un
    // permiso recién concedido o vencido, o una réplica que se cae, se nota sin reiniciar.
    if (funcion != DatabaseManager::Funcion::Unico) {
        m_labelSync = new QLabel(this);
        statusBar()->addPermanentWidget(m_labelSync);
    }
    auto *temporizador = new QTimer(this);
    connect(temporizador, &QTimer::timeout, this, &MainWindow::actualizarSync);
    temporizador->start(10000);
    actualizarSync();
}

void MainWindow::actualizarSync() {
    const SesionActual &sesion = SesionActual::instance();
    const auto funcion = DatabaseManager::funcion();

    QStringList partes;
    QString alcance = sesion.veTodasLasSucursales() ? "todas las sucursales" : sesion.nombreSucursal();
    if (sesion.esAdmin() && funcion == DatabaseManager::Funcion::Sucursal)
        alcance += " (modo local)";   // admin sin conexión con el central
    partes << QString("<b style='color:#ffffff'>%1</b> · %2").arg(sesion.esAdmin() ? "Administrador" : "Empleado", alcance.toHtmlEscaped());

    // Permiso temporal «encargado» vigente (un admin no lo necesita).
    if (!sesion.esAdmin()) {
        const QDateTime fin = sesion.vencePermisoTemporal();
        if (fin.isValid()) {
            const int dias = QDate::currentDate().daysTo(fin.date());
            const QString cuando = dias == 0 ? "hoy " + fin.toString("HH:mm")
                                   : dias == 1 ? "mañana " + fin.toString("HH:mm") : fin.toString("dd/MM HH:mm");
            partes << QString("<span style='color:#1abc9c'><b>Encargado</b> hasta %1</span>").arg(cuando);
        }
    }

    if (funcion != DatabaseManager::Funcion::Unico) {
        const EstadoSync::Estado estado = EstadoSync::consultar();
        QString color, corto;
        switch (estado.nivel) {
        case EstadoSync::Nivel::Bien: color = "#2e7d32"; corto = "Sincronizado"; break;
        case EstadoSync::Nivel::Retraso: color = "#ef6c00"; corto = "Sincronizando"; break;
        case EstadoSync::Nivel::SinConexion: color = "#e53935"; corto = "Sin conexión"; break;
        default: color = "gray"; corto = "Réplica sin datos"; break;
        }
        partes << QString("<span style='color:%1'>●</span> %2").arg(color, corto);
        ui->labelInsignia->setToolTip(estado.detalle);
        if (m_labelSync) {
            m_labelSync->setText(QString("<span style='color:%1'>●</span> %2").arg(color, estado.texto));
            m_labelSync->setToolTip(estado.detalle);
        }
    }
    ui->labelInsignia->setText(partes.join("&nbsp;&nbsp;&nbsp;│&nbsp;&nbsp;&nbsp;"));
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::irAPagina(const QString &nombrePagina) {
    // Las secciones del menú también son elementos de la lista: solo vale la que es una pantalla.
    const auto items = ui->listMenu->findItems(nombrePagina, Qt::MatchExactly);
    for (QListWidgetItem *item : items) {
        if (item->data(Qt::UserRole).isValid()) {
            ui->listMenu->setCurrentItem(item);
            return;
        }
    }
}
