// Fuzz de formularios: cada diálogo se rellena con valores VÁLIDOS y se cambia UN campo cada vez por un valor
// hostil o extremo (como si lo pegara/tecleara un usuario malicioso o descuidado, o lo pusiera el código).
// Después de pulsar Aceptar se compara el estado de la BD: no debe quedar nada inválido guardado.
#include "testutil.h"

#include "dialogs/categoriadialog.h"
#include "dialogs/clientedialog.h"
#include "dialogs/empleadodialog.h"
#include "dialogs/lotedialog.h"
#include "dialogs/productodialog.h"
#include "dialogs/proveedordialog.h"
#include "dialogs/sucursaldialog.h"
#include "dialogs/usuariodialog.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDateEdit>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTextEdit>
#include <memory>

namespace {

QString letras(int n) {   // 0→"a", 1→"b", … 26→"ba": identificador único SOLO con letras (los nombres no admiten dígitos)
    QString s;
    do { s.prepend(QChar('a' + n % 26)); n /= 26; } while (n > 0);
    return s;
}
int g_contador = 0;

struct Hostil {
    QString nombre;
    QString valor;
    bool peligroso;   // true: NUNCA debe quedar guardado tal cual (HTML, control, NUL, tamaño)
};
QList<Hostil> hostiles() {
    return {
        {"sql-drop", "x'); DROP TABLE Productos; --", false},
        {"sql-or", "' OR '1'='1", false},
        {"html", "<img src=x onerror=alert(1)>", true},
        {"script", "<script>alert(1)</script>", true},
        {"control", QString("a\x01\x02\x03") + "b", true},
        {"nul", QString("a") + QChar(0) + "b", true},
        {"esc-ansi", QString("a\x1b[31mb"), true},
        {"saltos", "linea1\nlinea2\r\nlinea3", false},
        {"largo-5000", QString("a").repeated(5000), true},
        {"emoji", "💊💊💊", false},
        {"rtl", QString("‮abc"), true},
        {"solo-espacios", "     ", false},
        {"comodin", "%_\\", false},
        {"negativo", "-1", false},
        {"exponente", "1e999", false},
        {"vacio", "", false},
    };
}

QString valido(const QString &campo) {
    const QString u = letras(++g_contador);
    if (campo == "lineNombre") return "Prueba " + u;
    if (campo == "lineApellidoPaterno") return "Fuzz";
    if (campo == "lineApellidoMaterno") return "Test";
    if (campo == "lineTelefono") return "5551234567";
    if (campo == "lineCorreo") return "fuzz@test.com";
    if (campo == "lineDireccion") return "Calle 1 #2";
    if (campo == "linePuesto") return "Cajero";
    if (campo == "lineNombreEmpresa") return "Empresa " + u;
    if (campo == "lineMarca") return "Marca X";
    if (campo == "lineTipo") return "medicamento";
    if (campo == "lineCodigoBarras") return "FUZZ-" + QString::number(g_contador) + "1";
    if (campo == "lineNumeroLote") return "FZ-" + QString::number(g_contador);
    if (campo == "lineUsuario") return "fuzz" + u;
    if (campo == "lineContrasena") return "Fuzz1234";
    return "Valor " + u;
}

template <class W> QList<W *> hijos(QWidget *d) { return d->findChildren<W *>(); }

void rellenarValido(QDialog *d) {
    for (auto *l : hijos<QLineEdit>(d))
        if (!l->objectName().startsWith("qt_")) l->setText(valido(l->objectName()));
    for (auto *t : hijos<QTextEdit>(d)) t->setPlainText("Descripción válida");
    for (auto *t : hijos<QPlainTextEdit>(d)) t->setPlainText("Descripción válida");
    for (auto *s : hijos<QSpinBox>(d))
        if (!s->objectName().startsWith("qt_")) s->setValue(qBound(s->minimum(), 10, s->maximum()));
    for (auto *s : hijos<QDoubleSpinBox>(d))
        if (!s->objectName().startsWith("qt_")) s->setValue(qBound(s->minimum(), 12.5, s->maximum()));
    for (auto *c : hijos<QComboBox>(d))
        if (c->count() > 0 && c->currentIndex() < 0) c->setCurrentIndex(0);
    for (auto *e : hijos<QDateEdit>(d)) e->setDate(QDate::currentDate().addYears(1));
}

// Infracciones de integridad presentes en la BD (se comparan antes/después para atribuirlas al intento).
QStringList violaciones() {
    QStringList v;
    const auto contar = [&](const QString &etiqueta, const QString &sql) {
        const int n = T::escalar(sql).toInt();
        if (n > 0) v << QString("%1 (%2 filas)").arg(etiqueta).arg(n);
    };
    // Texto con HTML o caracteres de control guardado en columnas de captura.
    const QList<QPair<QString, QString>> textos = {
        {"Productos", "nombre"}, {"Productos", "marca"}, {"Productos", "descripcion"}, {"Productos", "codigo_barras"},
        {"Categorias", "nombre"}, {"Categorias", "tipo"}, {"Categorias", "descripcion"},
        {"Proveedores", "nombre_empresa"}, {"Proveedores", "telefono"}, {"Proveedores", "correo"}, {"Proveedores", "direccion"},
        {"Clientes", "nombre"}, {"Clientes", "apellido_paterno"}, {"Clientes", "apellido_materno"}, {"Clientes", "telefono"},
        {"Clientes", "correo"}, {"Clientes", "direccion"},
        {"Empleados", "nombre"}, {"Empleados", "apellido_paterno"}, {"Empleados", "apellido_materno"}, {"Empleados", "puesto"},
        {"Empleados", "telefono"}, {"Empleados", "correo"},
        {"Sucursales", "nombre"}, {"Sucursales", "direccion"}, {"Sucursales", "telefono"},
        {"Lotes", "numero_lote"}, {"Usuarios", "nombre_usuario"},
    };
    for (const auto &[tabla, col] : textos) {
        const bool multilinea = col == "descripcion";   // las descripciones admiten saltos de línea y tabuladores
        contar(QString("%1.%2 con <, > o caracteres de control").arg(tabla, col),
               multilinea ? QString("SELECT COUNT(*) FROM %1 WHERE %2 REGEXP '[<>]' OR REPLACE(REPLACE(REPLACE(%2, CHAR(10), ''), CHAR(13), ''), CHAR(9), '') REGEXP '[[:cntrl:]]'").arg(tabla, col)
                          : QString("SELECT COUNT(*) FROM %1 WHERE %2 REGEXP '[<>]|[[:cntrl:]]'").arg(tabla, col));
        if (!multilinea)
            contar(QString("%1.%2 con espacios repetidos").arg(tabla, col),
                   QString("SELECT COUNT(*) FROM %1 WHERE %2 REGEXP '[[:space:]]{2,}'").arg(tabla, col));
        contar(QString("%1.%2 con espacios sobrantes en los extremos").arg(tabla, col),
               QString("SELECT COUNT(*) FROM %1 WHERE %2 IS NOT NULL AND %2 <> TRIM(%2)").arg(tabla, col));
    }
    for (const auto &[tabla, col] : QList<QPair<QString, QString>>{{"Productos", "nombre"}, {"Clientes", "nombre"}, {"Empleados", "nombre"},
                                                                    {"Sucursales", "nombre"}, {"Categorias", "nombre"}, {"Proveedores", "nombre_empresa"},
                                                                    {"Usuarios", "nombre_usuario"}})
        contar(QString("%1.%2 vacío").arg(tabla, col), QString("SELECT COUNT(*) FROM %1 WHERE TRIM(%2) = ''").arg(tabla, col));
    contar("Productos.precio_sugerido <= 0", "SELECT COUNT(*) FROM Productos WHERE precio_sugerido <= 0");
    contar("Productos_Sucursal.precio_venta <= 0", "SELECT COUNT(*) FROM Productos_Sucursal WHERE precio_venta IS NOT NULL AND precio_venta <= 0");
    contar("Lotes.cantidad negativa", "SELECT COUNT(*) FROM Lotes WHERE cantidad_inicial < 0 OR cantidad_disponible < 0");
    contar("Lotes.costo_unitario <= 0", "SELECT COUNT(*) FROM Lotes WHERE costo_unitario <= 0");
    contar("Lotes con caducidad anterior al ingreso", "SELECT COUNT(*) FROM Lotes WHERE fecha_caducidad < fecha_ingreso");
    contar("Lotes con caducidad > 15 años", "SELECT COUNT(*) FROM Lotes WHERE fecha_caducidad > DATE_ADD(CURDATE(), INTERVAL 15 YEAR)");
    contar("Usuarios con hash/sal vacíos", "SELECT COUNT(*) FROM Usuarios WHERE contrasena_hash = '' OR sal = ''");
    const int tablas = T::escalar("SELECT COUNT(*) FROM information_schema.tables WHERE table_schema='farmacia'").toInt();
    if (tablas != 18) v << QString("Faltan tablas: hay %1 de 18").arg(tablas);
    return v;
}

}   // namespace

class TestFormularios : public QObject {
    Q_OBJECT
    T::Modales *m = nullptr;

    using Fabrica = std::function<QDialog *()>;
    struct Def {
        const char *nombre;
        Fabrica crear;
    };
    static QList<Def> defs() {
        return {
            {"ClienteDialog", [] { return new ClienteDialog; }},
            {"EmpleadoDialog", [] { return new EmpleadoDialog; }},
            {"ProveedorDialog", [] { return new ProveedorDialog; }},
            {"SucursalDialog", [] { return new SucursalDialog; }},
            {"CategoriaDialog", [] { return new CategoriaDialog; }},
            {"ProductoDialog", [] { return new ProductoDialog; }},
            {"LoteDialog", [] { return new LoteDialog; }},
            {"UsuarioDialog", [] { return new UsuarioDialog; }},
        };
    }

private slots:
    void initTestCase() {
        QString e;
        QVERIFY2(T::conectarBd(&e), qPrintable(e));
        T::sembrarUsuarios();
        m = new T::Modales;
        T::iniciarSesionAdmin();
    }
    void cleanupTestCase() { delete m; }

    // Línea base: con todo válido cada formulario guarda (si no, el resto del fuzz no significa nada).
    void base_todo_valido_data() {
        QTest::addColumn<int>("i");
        const auto ds = defs();
        for (int i = 0; i < ds.size(); ++i) QTest::addRow("%s", ds[i].nombre) << i;
    }
    void base_todo_valido() {
        QFETCH(int, i);
        m->limpiar();
        std::unique_ptr<QDialog> d(defs()[i].crear());
        d->show();
        rellenarValido(d.get());
        T::pulsarAceptar(d.get());
        QVERIFY2(d->result() == QDialog::Accepted,
                 qPrintable(QString("Con datos válidos %1 no guardó. Avisos: %2").arg(defs()[i].nombre, m->todosLosTextos())));
        QVERIFY2(violaciones().isEmpty(), qPrintable("Datos válidos dejaron la BD en mal estado: " + violaciones().join("; ")));
    }

    // Un campo de texto cada vez con cada valor hostil.
    void texto_hostil_data() {
        QTest::addColumn<int>("i");
        QTest::addColumn<bool>("directo");
        const auto ds = defs();
        for (int i = 0; i < ds.size(); ++i) {
            QTest::addRow("%s/pegado", ds[i].nombre) << i << false;     // pasa por el filtro del campo, como un usuario
            QTest::addRow("%s/directo(info)", ds[i].nombre) << i << true;   // setText: salta el filtro (solo informa)
        }
    }
    void texto_hostil() {
        QFETCH(int, i);
        QFETCH(bool, directo);
        QStringList problemas, informativos;
        std::unique_ptr<QDialog> probe(defs()[i].crear());
        QStringList campos;
        for (auto *l : probe->findChildren<QLineEdit *>())
            if (!l->objectName().startsWith("qt_") && !l->objectName().contains("Contrasena")) campos << l->objectName();
        for (auto *t : probe->findChildren<QTextEdit *>()) campos << t->objectName();
        probe.reset();

        for (const QString &campo : campos) {
            for (const Hostil &h : hostiles()) {
                m->limpiar();
                const QStringList antes = violaciones();
                std::unique_ptr<QDialog> d(defs()[i].crear());
                d->show();
                rellenarValido(d.get());
                if (auto *l = d->findChild<QLineEdit *>(campo)) { if (directo) l->setText(h.valor); else { l->clear(); l->insert(h.valor); } }
                else if (auto *t = d->findChild<QTextEdit *>(campo)) t->setPlainText(h.valor);
                T::pulsarAceptar(d.get());
                const bool acepto = d->result() == QDialog::Accepted;
                QStringList nuevas;
                for (const QString &x : violaciones())
                    if (!antes.contains(x)) nuevas << x;
                const QString etiqueta = QString("%1.%2 ← %3").arg(defs()[i].nombre, campo, h.nombre);
                if (!nuevas.isEmpty()) problemas << etiqueta + " → " + nuevas.join("; ");
                for (const auto &a : m->avisos)
                    if (a.clase == "QMessageBox" && a.titulo == "Error")
                        problemas << etiqueta + " → error técnico mostrado: " + a.texto.simplified().left(160);
                if (acepto && h.peligroso && nuevas.isEmpty() && h.nombre != "largo-5000")
                    informativos << etiqueta + " fue ACEPTADO (verifica que se haya limpiado)";
            }
        }
        for (const auto &s : informativos) qInfo().noquote() << "  INFO" << s;
        if (directo) {   // solo informativo: ningún usuario puede llegar aquí sin pasar por el filtro del campo
            for (const auto &p : problemas) qInfo().noquote() << "  INFO(directo)" << p;
            return;
        }
        QVERIFY2(problemas.isEmpty(), qPrintable("\n  - " + problemas.join("\n  - ")));
    }

    // Escritura REAL (tecla a tecla): lo que sí puede hacer cualquier usuario con el filtro del campo activo.
    void escritura_real_data() {
        QTest::addColumn<int>("i");
        QTest::addColumn<QString>("campo");
        QTest::addColumn<QString>("tabla");
        QTest::addColumn<QString>("columna");
        QTest::newRow("Sucursal.nombre") << 3 << "lineNombre" << "Sucursales" << "nombre";
        QTest::newRow("Sucursal.direccion") << 3 << "lineDireccion" << "Sucursales" << "direccion";
        QTest::newRow("Categoria.nombre") << 4 << "lineNombre" << "Categorias" << "nombre";
        QTest::newRow("Categoria.tipo") << 4 << "lineTipo" << "Categorias" << "tipo";
        QTest::newRow("Producto.nombre") << 5 << "lineNombre" << "Productos" << "nombre";
        QTest::newRow("Proveedor.nombre") << 2 << "lineNombreEmpresa" << "Proveedores" << "nombre_empresa";
        QTest::newRow("Cliente.nombre") << 0 << "lineNombre" << "Clientes" << "nombre";
        QTest::newRow("Empleado.puesto") << 1 << "linePuesto" << "Empleados" << "puesto";
    }
    void escritura_real() {
        QFETCH(int, i);
        QFETCH(QString, campo);
        QFETCH(QString, tabla);
        QFETCH(QString, columna);
        m->limpiar();
        std::unique_ptr<QDialog> d(defs()[i].crear());
        d->show();
        rellenarValido(d.get());
        auto *l = d->findChild<QLineEdit *>(campo);
        QVERIFY(l);
        l->clear();
        const QString base = "Zeta   Doble    Espacio  " + letras(++g_contador);
        QTest::keyClicks(l, "   " + base + "   ");
        T::pulsarAceptar(d.get());
        QVERIFY2(d->result() == QDialog::Accepted, qPrintable("No guardó: " + m->todosLosTextos()));
        const QString guardado = T::escalar(QString("SELECT %1 FROM %2 WHERE %1 LIKE 'Zeta%' ORDER BY 1 DESC LIMIT 1").arg(columna, tabla)).toString();
        QVERIFY2(!guardado.contains(QRegularExpression("\\s{2,}")) && guardado == guardado.trimmed(),
                 qPrintable(QString("Se guardó con espacios sin normalizar: «%1»").arg(guardado)));
    }

    // Un control numérico/fecha/combo cada vez en sus valores extremos.
    void numeros_y_fechas_extremos_data() {
        QTest::addColumn<int>("i");
        const auto ds = defs();
        for (int i = 0; i < ds.size(); ++i) QTest::addRow("%s", ds[i].nombre) << i;
    }
    void numeros_y_fechas_extremos() {
        QFETCH(int, i);
        QStringList problemas;
        std::unique_ptr<QDialog> probe(defs()[i].crear());
        QStringList spins, dobles, fechas, combos;
        for (auto *s : probe->findChildren<QSpinBox *>()) if (!s->objectName().startsWith("qt_")) spins << s->objectName();
        for (auto *s : probe->findChildren<QDoubleSpinBox *>()) if (!s->objectName().startsWith("qt_")) dobles << s->objectName();
        for (auto *e : probe->findChildren<QDateEdit *>()) fechas << e->objectName();
        for (auto *c : probe->findChildren<QComboBox *>()) combos << c->objectName();
        probe.reset();

        const auto intento = [&](const QString &etiqueta, const std::function<void(QDialog *)> &mutar) {
            m->limpiar();
            const QStringList antes = violaciones();
            std::unique_ptr<QDialog> d(defs()[i].crear());
            d->show();
            rellenarValido(d.get());
            mutar(d.get());
            T::pulsarAceptar(d.get());
            for (const QString &x : violaciones())
                if (!antes.contains(x)) problemas << QString("%1.%2 → %3").arg(defs()[i].nombre, etiqueta, x);
            for (const auto &a : m->avisos)
                if (a.clase == "QMessageBox" && a.titulo == "Error")
                    problemas << QString("%1.%2 → error técnico mostrado: %3").arg(defs()[i].nombre, etiqueta, a.texto.simplified().left(160));
        };
        for (const QString &n : spins) {
            for (const char *modo : {"mínimo", "máximo", "cero", "menos-uno"})
                intento(n + "=" + modo, [&](QDialog *d) {
                    auto *s = d->findChild<QSpinBox *>(n);
                    const QString k = modo;
                    s->setValue(k == "mínimo" ? s->minimum() : k == "máximo" ? s->maximum() : k == "cero" ? 0 : -1);
                });
        }
        for (const QString &n : dobles) {
            for (const char *modo : {"mínimo", "máximo", "cero", "0.001", "0.005", "99999999"})
                intento(n + "=" + modo, [&](QDialog *d) {
                    auto *s = d->findChild<QDoubleSpinBox *>(n);
                    const QString k = modo;
                    s->setValue(k == "mínimo" ? s->minimum() : k == "máximo" ? s->maximum() : k.toDouble());
                });
        }
        for (const QString &n : fechas) {
            for (const char *modo : {"1900-01-01", "ayer", "hoy", "+15 años", "+16 años", "9999-12-31"})
                intento(n + "=" + modo, [&](QDialog *d) {
                    auto *e = d->findChild<QDateEdit *>(n);
                    const QString k = modo;
                    e->setDate(k == "ayer" ? QDate::currentDate().addDays(-1) : k == "hoy" ? QDate::currentDate()
                               : k == "+15 años" ? QDate::currentDate().addYears(15) : k == "+16 años" ? QDate::currentDate().addYears(16)
                               : QDate::fromString(k, Qt::ISODate));
                });
        }
        for (const QString &n : combos) {
            if (n != "comboEstado" && n != "comboRol")   // los de opciones fijas no pueden quedar vacíos desde la interfaz
                intento(n + "=sin selección", [&](QDialog *d) { d->findChild<QComboBox *>(n)->setCurrentIndex(-1); });
            intento(n + "=último", [&](QDialog *d) { auto *c = d->findChild<QComboBox *>(n); c->setCurrentIndex(c->count() - 1); });
        }
        QVERIFY2(problemas.isEmpty(), qPrintable("\n  - " + problemas.join("\n  - ")));
    }
};

FARMACIA_TEST_MAIN(TestFormularios)
#include "test_formularios.moc"
