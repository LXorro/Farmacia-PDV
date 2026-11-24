// Control de acceso (qué ve y qué puede guardar cada rol), permisos temporales y login.
#include "testutil.h"

#include "dialogs/lotedialog.h"
#include "dialogs/logindialog.h"
#include "dialogs/usuariodialog.h"
#include "mainwindow.h"

#include <QComboBox>
#include <QLabel>
#include <QListWidget>
#include <QDateEdit>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QRandomGenerator>
#include <QPushButton>
#include <QLineEdit>
#include <memory>

class TestAcceso : public QObject {
    Q_OBJECT
    T::Modales *m = nullptr;

    static void darPermiso(int horasInicio, int horasFin, const QString &estado = "vigente") {
        QSqlQuery q;
        q.exec("DELETE FROM Permisos_Temporales");
        q.prepare("INSERT INTO Permisos_Temporales (id_usuario, fecha_inicio, fecha_fin, estado) "
                  "VALUES ((SELECT id_usuario FROM Usuarios WHERE nombre_usuario='luis'), "
                  "DATE_ADD(UTC_TIMESTAMP(), INTERVAL :a HOUR), DATE_ADD(UTC_TIMESTAMP(), INTERVAL :b HOUR), :e)");
        q.bindValue(":a", horasInicio);
        q.bindValue(":b", horasFin);
        q.bindValue(":e", estado);
        q.exec();
    }
    int lotesNuevos() { return T::contar("Lotes", "numero_lote LIKE 'ACC-%'"); }
    // Intenta dar de alta un lote con datos válidos desde el diálogo.
    bool altaDeLote() {
        std::unique_ptr<LoteDialog> d(new LoteDialog);
        d->show();
        d->findChild<QLineEdit *>("lineNumeroLote")->setText("ACC-" + QString::number(QRandomGenerator::global()->bounded(1000000)));
        d->findChild<QDoubleSpinBox *>("spinCosto")->setValue(5.0);
        d->findChild<QSpinBox *>("spinCantidad")->setValue(5);
        d->findChild<QDateEdit *>("dateFechaCaducidad")->setDate(QDate::currentDate().addYears(1));
        T::pulsarAceptar(d.get());
        return d->result() == QDialog::Accepted;
    }
    // Inicia sesión por la pantalla de login real.
    bool login(const QString &usuario, const QString &clave, QString *mensaje = nullptr) {
        SesionActual::instance().cerrar();
        LoginDialog d;
        d.show();
        d.findChild<QLineEdit *>("lineUsuario")->setText(usuario);
        d.findChild<QLineEdit *>("lineContrasena")->setText(clave);
        d.findChild<QPushButton *>("btnIngresar")->click();
        if (mensaje) *mensaje = d.findChild<QLabel *>("labelError")->text();
        return SesionActual::instance().activa();
    }

private slots:
    void initTestCase() {
        QString e;
        QVERIFY2(T::conectarBd(&e), qPrintable(e));
        T::sembrarUsuarios();
        m = new T::Modales;
    }
    void cleanupTestCase() { delete m; }
    void init() { m->limpiar(); }

    // ---- menú por rol ---------------------------------------------------------------------------------
    void menu_por_rol() {
        const auto entradas = [&](bool admin) {
            admin ? T::iniciarSesionAdmin() : T::iniciarSesionEmpleado();
            MainWindow w;
            QStringList l;
            for (auto *lista : w.findChildren<QListWidget *>())
                if (lista->objectName() == "listMenu")
                    for (int i = 0; i < lista->count(); ++i) l << lista->item(i)->text();
            return l;
        };
        const QStringList admin = entradas(true), empleado = entradas(false);
        qInfo().noquote() << "  menú admin:   " << admin.join(" | ");
        qInfo().noquote() << "  menú empleado:" << empleado.join(" | ");
        for (const QString &prohibida : {"Usuarios", "Empleados", "Sucursales", "Permisos", "Asignaciones", "Bitácora", "Proveedores"})
            QVERIFY2(!empleado.contains(prohibida), qPrintable("El empleado ve la pantalla «" + prohibida + "»"));
        QVERIFY(admin.contains("Usuarios") && admin.contains("Bitácora"));
        QVERIFY(empleado.contains("Punto de Venta"));
    }

    // ---- permisos temporales -----------------------------------------------------------------------------
    void empleado_sin_permiso_no_da_de_alta_lotes() {
        T::iniciarSesionEmpleado();
        QSqlQuery("DELETE FROM Permisos_Temporales");
        QVERIFY2(!altaDeLote(), "El empleado SIN permiso pudo dar de alta un lote");
        QCOMPARE(lotesNuevos(), 0);
    }
    void permiso_vencido_no_sirve() {
        T::iniciarSesionEmpleado();
        darPermiso(-50, -2);
        QVERIFY2(!altaDeLote(), "Un permiso VENCIDO permitió dar de alta un lote");
    }
    void permiso_futuro_no_sirve() {
        T::iniciarSesionEmpleado();
        darPermiso(2, 20);
        QVERIFY2(!altaDeLote(), "Un permiso que aún NO empieza permitió dar de alta un lote");
    }
    void permiso_revocado_no_sirve() {
        T::iniciarSesionEmpleado();
        darPermiso(-1, 20, "revocado");
        QVERIFY2(!altaDeLote(), "Un permiso REVOCADO permitió dar de alta un lote");
    }
    void permiso_vigente_si_sirve() {
        T::iniciarSesionEmpleado();
        darPermiso(-1, 20);
        QVERIFY2(altaDeLote(), qPrintable("Con permiso vigente no se pudo: " + m->todosLosTextos()));
        QCOMPARE(lotesNuevos(), 1);
        QCOMPARE(T::contar("Bitacora_Cambios", "accion='lote_alta' AND con_permiso=TRUE"), 1);
    }
    void permiso_mas_de_72h_lo_rechaza_la_bd() {
        QSqlQuery q;
        q.prepare("INSERT INTO Permisos_Temporales (id_usuario, fecha_inicio, fecha_fin) VALUES (1, UTC_TIMESTAMP(), DATE_ADD(UTC_TIMESTAMP(), INTERVAL 100 HOUR))");
        QVERIFY2(!q.exec(), "La BD aceptó un permiso de 100 horas");
    }
    void empleado_con_permiso_solo_su_sucursal() {
        T::iniciarSesionEmpleado();
        darPermiso(-1, 20);
        LoteDialog d;
        auto *c = d.findChild<QComboBox *>("comboSucursal");
        QVERIFY2(!c->isEnabled() || c->count() == 1,
                 "Un empleado con permiso puede elegir OTRA sucursal al dar de alta un lote");
        QCOMPARE(c->currentData().toInt(), 2);
    }
    void permiso_que_vence_con_la_ventana_abierta() {
        T::iniciarSesionEmpleado();
        darPermiso(-1, 20);
        std::unique_ptr<LoteDialog> d(new LoteDialog);
        d->show();
        darPermiso(-50, -2);   // vence mientras el formulario está abierto
        d->findChild<QLineEdit *>("lineNumeroLote")->setText("ACC-VENCE");
        d->findChild<QDoubleSpinBox *>("spinCosto")->setValue(5.0);
        d->findChild<QDateEdit *>("dateFechaCaducidad")->setDate(QDate::currentDate().addYears(1));
        T::pulsarAceptar(d.get());
        QCOMPARE(T::contar("Lotes", "numero_lote='ACC-VENCE'"), 0);
    }

    // ---- login -------------------------------------------------------------------------------------------
    void login_correcto() {
        QVERIFY(login("admin", "Admin123"));
        QCOMPARE(SesionActual::instance().rol(), QString("admin"));
        QVERIFY(login("luis", "Luis12345"));
        QCOMPARE(SesionActual::instance().rol(), QString("empleado"));
    }
    void login_hostil_data() {
        QTest::addColumn<QString>("usuario");
        QTest::addColumn<QString>("clave");
        QTest::newRow("clave incorrecta") << "admin" << "admin123";
        QTest::newRow("clave vacia") << "admin" << "";
        QTest::newRow("usuario vacio") << "" << "Admin123";
        QTest::newRow("usuario inexistente") << "nadie" << "Admin123";
        QTest::newRow("sqli usuario") << "admin' --" << "x";
        QTest::newRow("sqli usuario 2") << "' OR '1'='1' -- " << "x";
        QTest::newRow("sqli clave") << "admin" << "' OR '1'='1";
        QTest::newRow("sqli union") << "x' UNION SELECT 1,'a','b','admin','activo',1,1,'S' -- " << "x";
        QTest::newRow("clave con espacio final") << "admin" << "Admin123 ";
        QTest::newRow("clave en minusculas") << "admin" << "admin123";
        QTest::newRow("solo espacios") << "   " << "   ";
        QTest::newRow("largo") << QString("a").repeated(100000) << QString("b").repeated(100000);
        QTest::newRow("NUL en usuario") << QString("admin") + QChar(0) + "x" << "Admin123";
        QTest::newRow("comodin") << "%" << "Admin123";
    }
    void login_hostil() {
        QFETCH(QString, usuario);
        QFETCH(QString, clave);
        QString msg;
        const bool entro = login(usuario, clave, &msg);
        QVERIFY2(!entro, qPrintable(QString("Entró con usuario «%1» y clave «%2»").arg(usuario.left(30), clave.left(30))));
        QVERIFY2(!msg.contains("SQL", Qt::CaseInsensitive) && !msg.contains("syntax", Qt::CaseInsensitive),
                 qPrintable("El login muestra un error técnico: " + msg));
    }
    // Informativo: la colación utf8mb4_unicode_ci de MariaDB trata 'ａｄｍｉｎ' (ancho completo) y 'admín' como 'admin'.
    // No hay escalada (la contraseña sigue siendo obligatoria), pero conviene saberlo.
    void login_colacion_unicode_informativo() {
        for (const QString &variante : {QString("ＡＤＭＩＮ"), QString("admín"), QString("ADMIN")})
            if (login(variante, "Admin123"))
                qInfo().noquote() << "  INFO el usuario «" + variante + "» entra como «admin» (colación unicode_ci)";
    }
    void login_no_distingue_usuario_inexistente_de_clave_mala() {
        QString a, b;
        login("admin", "mala", &a);
        login("noexiste", "mala", &b);
        QCOMPARE(a, b);   // si difieren, se puede averiguar qué usuarios existen
    }
    void login_cuenta_inactiva() {
        QSqlQuery("UPDATE Usuarios SET estado='inactivo' WHERE nombre_usuario='luis'");
        QString msg;
        const bool entro = login("luis", "Luis12345", &msg);
        QSqlQuery("UPDATE Usuarios SET estado='activo' WHERE nombre_usuario='luis'");
        QVERIFY2(!entro, "Una cuenta INACTIVA pudo iniciar sesión");
    }
    void login_sin_bloqueo_por_intentos() {
        int intentos = 0;
        for (; intentos < 25; ++intentos) {
            if (login("admin", "incorrecta" + QString::number(intentos))) break;
        }
        QString msg;
        const bool entra = login("admin", "Admin123", &msg);
        QVERIFY2(!entra,
                 qPrintable(QString("NO hay bloqueo tras %1 intentos fallidos seguidos: la clave correcta entró de inmediato (permite fuerza bruta)").arg(intentos)));
    }
    void login_espacios_en_el_usuario() {
        // El usuario se recorta (trimmed): " admin " entra como «admin». Se documenta el comportamiento.
        QVERIFY2(login(" admin ", "Admin123"), "Con espacios alrededor del usuario ya no entra");
    }

    // ---- bloqueo del último administrador ----------------------------------------------------------------
    void no_se_puede_desactivar_al_unico_admin() {
        T::iniciarSesionAdmin();
        QSqlQuery("UPDATE Usuarios SET estado='activo', rol='empleado' WHERE rol='admin' AND nombre_usuario<>'admin'");
        const int idAdmin = T::escalar("SELECT id_usuario FROM Usuarios WHERE nombre_usuario='admin'").toInt();
        std::unique_ptr<UsuarioDialog> d(new UsuarioDialog);
        d->setUsuarioId(idAdmin);
        d->show();
        d->findChild<QComboBox *>("comboEstado")->setCurrentText("inactivo");
        T::pulsarAceptar(d.get());
        const int activos = T::contar("Usuarios", "rol='admin' AND estado='activo'");
        QSqlQuery("UPDATE Usuarios SET estado='activo' WHERE nombre_usuario='admin'");
        QVERIFY2(activos > 0, "Se pudo DESACTIVAR al único administrador: el sistema queda sin nadie que lo administre");
    }
    void si_se_puede_desactivar_a_un_admin_cuando_hay_otro() {
        T::iniciarSesionAdmin();
        QSqlQuery("UPDATE Usuarios SET rol='admin', estado='activo' WHERE nombre_usuario='luis'");
        const int idLuis = T::escalar("SELECT id_usuario FROM Usuarios WHERE nombre_usuario='luis'").toInt();
        std::unique_ptr<UsuarioDialog> d(new UsuarioDialog);
        d->setUsuarioId(idLuis);
        d->show();
        d->findChild<QComboBox *>("comboEstado")->setCurrentText("inactivo");
        T::pulsarAceptar(d.get());
        const QString estado = T::escalar("SELECT estado FROM Usuarios WHERE nombre_usuario='luis'").toString();
        QSqlQuery("UPDATE Usuarios SET rol='empleado', estado='activo' WHERE nombre_usuario='luis'");
        QCOMPARE(estado, QString("inactivo"));   // hay otro admin activo: debe permitirse
    }
    void no_se_puede_desactivar_la_propia_cuenta() {
        T::iniciarSesionAdmin();
        QSqlQuery("UPDATE Usuarios SET rol='admin', estado='activo' WHERE nombre_usuario='luis'");   // hay otro admin
        const int idAdmin = T::escalar("SELECT id_usuario FROM Usuarios WHERE nombre_usuario='admin'").toInt();
        std::unique_ptr<UsuarioDialog> d(new UsuarioDialog);
        d->setUsuarioId(idAdmin);
        d->show();
        d->findChild<QComboBox *>("comboEstado")->setCurrentText("inactivo");
        T::pulsarAceptar(d.get());
        const QString estado = T::escalar("SELECT estado FROM Usuarios WHERE nombre_usuario='admin'").toString();
        QSqlQuery("UPDATE Usuarios SET rol='empleado' WHERE nombre_usuario='luis'");
        QSqlQuery("UPDATE Usuarios SET estado='activo' WHERE nombre_usuario='admin'");
        QCOMPARE(estado, QString("activo"));
    }
    void no_se_puede_degradar_al_unico_admin() {
        T::iniciarSesionAdmin();
        const int idAdmin = T::escalar("SELECT id_usuario FROM Usuarios WHERE nombre_usuario='admin'").toInt();
        std::unique_ptr<UsuarioDialog> d(new UsuarioDialog);
        d->setUsuarioId(idAdmin);
        d->show();
        d->findChild<QComboBox *>("comboRol")->setCurrentText("empleado");
        T::pulsarAceptar(d.get());
        const int admins = T::contar("Usuarios", "rol='admin' AND estado='activo'");
        QSqlQuery("UPDATE Usuarios SET rol='admin' WHERE nombre_usuario='admin'");
        QVERIFY2(admins > 0, "Se pudo DEGRADAR al único administrador a empleado: el sistema queda sin administrador");
    }
};

FARMACIA_TEST_MAIN(TestAcceso)
#include "test_acceso.moc"
