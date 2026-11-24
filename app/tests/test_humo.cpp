// Prueba de humo + "monkey": abre cada pantalla con cada rol y pulsa todos sus botones (con y sin fila
// seleccionada) con los campos vacíos. Detecta cierres inesperados, avisos con errores SQL crudos y
// advertencias de Qt.
#include "testutil.h"

#include "dialogs/asignaciondialog.h"
#include "dialogs/categoriadialog.h"
#include "dialogs/clientedialog.h"
#include "dialogs/configurarticketdialog.h"
#include "dialogs/devoluciondialog.h"
#include "dialogs/empleadodialog.h"
#include "dialogs/lotedialog.h"
#include "dialogs/pagomixtodialog.h"
#include "dialogs/pagotarjetadialog.h"
#include "dialogs/productodialog.h"
#include "dialogs/proveedordialog.h"
#include "dialogs/sucursaldialog.h"
#include "dialogs/ticketdetalledialog.h"
#include "dialogs/usuariodialog.h"
#include "dialogs/vistaticketdialog.h"
#include "mainwindow.h"
#include "pages/asignacionespage.h"
#include "pages/bitacorapage.h"
#include "pages/categoriaspage.h"
#include "pages/clientespage.h"
#include "pages/empleadospage.h"
#include "pages/iniciopage.h"
#include "pages/lotespage.h"
#include "pages/permisospage.h"
#include "pages/productospage.h"
#include "pages/proveedorespage.h"
#include "pages/puntoventapage.h"
#include "pages/reportediariopage.h"
#include "pages/sucursalespage.h"
#include "pages/ticketspage.h"
#include "pages/usuariospage.h"

#include <QAbstractItemView>
#include <QPointer>
#include <QPushButton>
#include <QRegularExpression>
#include <QTableView>
#include <QTableWidget>

static QStringList g_qtMensajes;
static QtMessageHandler g_previo = nullptr;
static void capturar(QtMsgType t, const QMessageLogContext &c, const QString &m) {
    // Ruido propio de la plataforma "offscreen", no de la app.
    if ((t == QtWarningMsg || t == QtCriticalMsg) && !m.contains("propagateSizeHints") && !m.contains("QFontDatabase"))
        g_qtMensajes << m;
    if (g_previo) g_previo(t, c, m);
}

static const QRegularExpression kErrorSqlCrudo(
    "SQLSTATE|You have an error in your SQL|Unknown column|doesn't exist|Data too long|Incorrect .* value|"
    "Duplicate entry|Cannot add or update|Cannot delete or update|QMYSQL|Out of range|Column '.*' cannot be null|"
    "Lock wait timeout|Deadlock|Access denied|command denied|trigger",
    QRegularExpression::CaseInsensitiveOption);

struct Pagina {
    const char *nombre;
    std::function<QWidget *()> crear;
};
static QList<Pagina> paginas() {
    return {
        {"Inicio", [] { return new InicioPage; }},
        {"PuntoVenta", [] { return new PuntoVentaPage; }},
        {"Clientes", [] { return new ClientesPage; }},
        {"Tickets", [] { return new TicketsPage; }},
        {"ReporteDiario", [] { return new ReporteDiarioPage; }},
        {"Productos", [] { return new ProductosPage; }},
        {"Lotes", [] { return new LotesPage; }},
        {"Categorias", [] { return new CategoriasPage; }},
        {"Proveedores", [] { return new ProveedoresPage; }},
        {"Empleados", [] { return new EmpleadosPage; }},
        {"Sucursales", [] { return new SucursalesPage; }},
        {"Usuarios", [] { return new UsuariosPage; }},
        {"Permisos", [] { return new PermisosPage; }},
        {"Asignaciones", [] { return new AsignacionesPage; }},
        {"Bitacora", [] { return new BitacoraPage; }},
    };
}

struct DialogoDef {
    const char *nombre;
    std::function<QDialog *()> crear;
};
static QList<DialogoDef> dialogos() {
    return {
        {"ClienteDialog", [] { return new ClienteDialog; }},
        {"ProductoDialog", [] { return new ProductoDialog; }},
        {"CategoriaDialog", [] { return new CategoriaDialog; }},
        {"ProveedorDialog", [] { return new ProveedorDialog; }},
        {"EmpleadoDialog", [] { return new EmpleadoDialog; }},
        {"SucursalDialog", [] { return new SucursalDialog; }},
        {"UsuarioDialog", [] { return new UsuarioDialog; }},
        {"LoteDialog", [] { return new LoteDialog; }},
        {"AsignacionDialog", [] { return new AsignacionDialog; }},
        {"DevolucionDialog", [] { return new DevolucionDialog; }},
        {"TicketDetalleDialog", [] { return new TicketDetalleDialog; }},
        {"ConfigurarTicketDialog", [] { return new ConfigurarTicketDialog; }},
        {"PagoTarjetaDialog(0)", [] { return new PagoTarjetaDialog(0); }},
        {"PagoTarjetaDialog(-5)", [] { return new PagoTarjetaDialog(-5); }},
        {"PagoMixtoDialog(0)", [] { return new PagoMixtoDialog(0); }},
        {"PagoMixtoDialog(-5)", [] { return new PagoMixtoDialog(-5); }},
        {"VistaTicketDialog(inexistente)", [] { return new VistaTicketDialog(999999); }},
        {"VistaTicketDialog(0)", [] { return new VistaTicketDialog(0); }},
        {"VistaTicketDialog(-1)", [] { return new VistaTicketDialog(-1); }},
    };
}

class TestHumo : public QObject {
    Q_OBJECT
    T::Modales *m = nullptr;

    void seleccionarPrimeraFila(QWidget *w) {
        for (auto *v : w->findChildren<QAbstractItemView *>())
            if (v->model() && v->model()->rowCount() > 0)
                v->setCurrentIndex(v->model()->index(0, 0));
    }

    // Pulsa todos los botones visibles y habilitados de `w`. Devuelve cuántos pulsó.
    int pulsarTodo(QWidget *w) {
        int n = 0;
        QList<QPointer<QPushButton>> botones;
        for (auto *b : w->findChildren<QPushButton *>()) botones << b;
        for (const auto &b : botones) {
            if (!b || !b->isVisibleTo(w) || !b->isEnabled()) continue;
            QTest::mouseClick(b, Qt::LeftButton);
            QCoreApplication::processEvents();
            ++n;
            ++m_clics;
        }
        return n;
    }

    int m_clics = 0, m_visibles = 0;
    void revisarAvisos(const QString &contexto) {
        qInfo().noquote() << QString("  [%1] botones pulsados=%2 avisos/diálogos=%3").arg(contexto).arg(m_clics).arg(m->avisos.size());
        m_clics = 0;
        for (const auto &a : m->avisos) {
            if (a.clase == "QMessageBox" && kErrorSqlCrudo.match(a.texto).hasMatch())
                QFAIL(qPrintable(QString("[%1] el usuario ve un error técnico: %2").arg(contexto, a.texto.simplified())));
        }
        m->limpiar();
    }

private slots:
    void initTestCase() {
        g_previo = qInstallMessageHandler(capturar);
        QString err;
        QVERIFY2(T::conectarBd(&err), qPrintable("Sin BD de pruebas: " + err));
        T::sembrarUsuarios();
        m = new T::Modales;
    }
    void cleanupTestCase() { delete m; }

    void paginas_data() {
        QTest::addColumn<int>("indice");
        QTest::addColumn<bool>("admin");
        const auto ps = ::paginas();
        for (int i = 0; i < ps.size(); ++i) {
            QTest::addRow("%s/admin", ps[i].nombre) << i << true;
            QTest::addRow("%s/empleado", ps[i].nombre) << i << false;
        }
    }
    void paginas() {
        QFETCH(int, indice);
        QFETCH(bool, admin);
        admin ? T::iniciarSesionAdmin() : T::iniciarSesionEmpleado();
        g_qtMensajes.clear();
        std::unique_ptr<QWidget> w(::paginas()[indice].crear());
        w->resize(1100, 700);
        w->show();
        QCoreApplication::processEvents();
        QMetaObject::invokeMethod(w.get(), "refrescar", Qt::DirectConnection);
        pulsarTodo(w.get());                 // sin selección
        seleccionarPrimeraFila(w.get());
        pulsarTodo(w.get());                 // con la primera fila seleccionada
        revisarAvisos(QTest::currentDataTag());
        QVERIFY2(g_qtMensajes.isEmpty(), qPrintable("Advertencias de Qt: " + g_qtMensajes.join(" | ")));
    }

    void ventanaPrincipal_data() {
        QTest::addColumn<bool>("admin");
        QTest::newRow("admin") << true;
        QTest::newRow("empleado") << false;
    }
    void ventanaPrincipal() {
        QFETCH(bool, admin);
        admin ? T::iniciarSesionAdmin() : T::iniciarSesionEmpleado();
        g_qtMensajes.clear();
        std::unique_ptr<MainWindow> w(new MainWindow);
        w->resize(1280, 800);
        w->show();
        QCoreApplication::processEvents();
        // Navega por todas las entradas del menú lateral.
        for (auto *lista : w->findChildren<QAbstractItemView *>()) {
            if (!lista->model() || lista->objectName().isEmpty()) continue;
            for (int i = 0; i < lista->model()->rowCount(); ++i) {
                lista->setCurrentIndex(lista->model()->index(i, 0));
                QCoreApplication::processEvents();
            }
        }
        revisarAvisos(QTest::currentDataTag());
        QVERIFY2(g_qtMensajes.isEmpty(), qPrintable("Advertencias de Qt: " + g_qtMensajes.join(" | ")));
    }

    // Cada diálogo, abierto "en blanco", con ambos roles: pulsar Aceptar/Cancelar no debe romper nada
    // ni dejar filas basura en la base.
    void dialogos_data() {
        QTest::addColumn<int>("indice");
        QTest::addColumn<bool>("admin");
        const auto ds = ::dialogos();
        for (int i = 0; i < ds.size(); ++i) {
            QTest::addRow("%s/admin", ds[i].nombre) << i << true;
            QTest::addRow("%s/empleado", ds[i].nombre) << i << false;
        }
    }
    void dialogos() {
        QFETCH(int, indice);
        QFETCH(bool, admin);
        admin ? T::iniciarSesionAdmin() : T::iniciarSesionEmpleado();
        g_qtMensajes.clear();
        const int antes = T::contar("Clientes") + T::contar("Productos") + T::contar("Categorias") +
                          T::contar("Proveedores") + T::contar("Empleados") + T::contar("Sucursales") +
                          T::contar("Lotes") + T::contar("Tickets") + T::contar("Usuarios");
        std::unique_ptr<QDialog> d(::dialogos()[indice].crear());
        d->show();
        QCoreApplication::processEvents();
        pulsarTodo(d.get());
        revisarAvisos(QTest::currentDataTag());
        const int despues = T::contar("Clientes") + T::contar("Productos") + T::contar("Categorias") +
                            T::contar("Proveedores") + T::contar("Empleados") + T::contar("Sucursales") +
                            T::contar("Lotes") + T::contar("Tickets") + T::contar("Usuarios");
        QVERIFY2(antes == despues, "Pulsar los botones de un formulario vacío creó filas en la base de datos");
        QVERIFY2(g_qtMensajes.isEmpty(), qPrintable("Advertencias de Qt: " + g_qtMensajes.join(" | ")));
    }
};

FARMACIA_TEST_MAIN(TestHumo)
#include "test_humo.moc"
