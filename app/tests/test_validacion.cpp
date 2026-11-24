// Validadores y filtros de captura frente a entradas hostiles (no necesita base de datos).
#include "testutil.h"
#include "util/passwordutil.h"
#include "util/validacion.h"

#include <QElapsedTimer>
#include <QLineEdit>

using namespace Validacion;

static QString rep(const QString &s, int n) { return s.repeated(n); }
static QString conNul(const QString &antes, const QString &despues) { return antes + QChar(0) + despues; }

class TestValidacion : public QObject {
    Q_OBJECT

    static void espera(bool valido, const QString &error, const QString &entrada, const char *quien) {
        const QString vista = entrada.left(40).toHtmlEscaped();
        if (valido)
            QVERIFY2(error.isEmpty(), qPrintable(QString("%1 rechazó una entrada válida «%2»: %3").arg(quien, vista, error)));
        else
            QVERIFY2(!error.isEmpty(), qPrintable(QString("%1 ACEPTÓ una entrada inválida «%2»").arg(quien, vista)));
    }

private slots:
    void nombrePersona_data() {
        QTest::addColumn<QString>("t");
        QTest::addColumn<bool>("valido");
        QTest::newRow("normal") << "Juan" << true;
        QTest::newRow("acentos") << "José Ángel" << true;
        QTest::newRow("eñe") << "Ñandú" << true;
        QTest::newRow("apostrofo") << "O'Brien" << true;
        QTest::newRow("guion") << "Ana-María" << true;
        QTest::newRow("inicial con punto") << "J. Pérez" << true;
        QTest::newRow("espacios extra se normalizan") << "  Juan    Pérez " << true;
        QTest::newRow("vacio") << "" << false;
        QTest::newRow("solo espacios") << "     " << false;
        QTest::newRow("una letra") << "J" << false;
        QTest::newRow("con numeros") << "Juan123" << false;
        QTest::newRow("sql injection") << "Robert'); DROP TABLE Usuarios;--" << false;
        QTest::newRow("sql OR") << "x' OR '1'='1" << false;
        QTest::newRow("html") << "<script>alert(1)</script>" << false;
        QTest::newRow("emoji") << "😀😀" << false;
        QTest::newRow("NUL embebido") << conNul("Jua", "n") << false;
        QTest::newRow("zero width space") << QString("Ju​an") << false;
        QTest::newRow("RTL override") << QString("‮anu") << false;
        QTest::newRow("51 letras") << rep("a", 51) << false;
        QTest::newRow("50 letras") << rep("a", 50) << true;
        QTest::newRow("solo guion") << "-" << false;
        QTest::newRow("solo puntos") << ".." << false;
        QTest::newRow("empieza con apostrofo") << "'Ana" << false;
        QTest::newRow("tab y salto se vuelven espacio") << "Ana\tLópez" << true;
        QTest::newRow("percent") << "100%" << false;
        QTest::newRow("backslash") << "Ana\\López" << false;
        QTest::newRow("punto y coma") << "Ana;López" << false;
    }
    void nombrePersona() {
        QFETCH(QString, t);
        QFETCH(bool, valido);
        espera(valido, Validacion::nombrePersona(t, "El nombre", true), t, "nombrePersona");
    }

    void textoComercial_data() {
        QTest::addColumn<QString>("t");
        QTest::addColumn<bool>("valido");
        QTest::newRow("producto") << "Paracetamol 500mg" << true;
        QTest::newRow("con simbolos") << "Vitamina C+Zinc (30 tabs) 5%" << true;
        QTest::newRow("solo numeros") << "500" << false;
        QTest::newRow("empieza con simbolo") << "-Producto" << false;
        QTest::newRow("html") << "<b>Producto</b>" << false;
        QTest::newRow("punto y coma") << "Prod; DROP TABLE x" << false;
        QTest::newRow("comillas dobles") << "Prod \"x\"" << false;
        QTest::newRow("igual") << "x' OR 1=1" << false;
        QTest::newRow("NUL") << conNul("Prod", "ucto") << false;
        QTest::newRow("101 chars") << rep("a", 101) << false;
        QTest::newRow("100 chars") << rep("a", 100) << true;
        QTest::newRow("vacio") << "" << false;
        QTest::newRow("emoji") << "Producto 💊" << false;
        QTest::newRow("backtick") << "Prod`uct" << false;
        QTest::newRow("salto de linea interno") << "Prod\nucto" << true;   // simplified → espacio
    }
    void textoComercial() {
        QFETCH(QString, t);
        QFETCH(bool, valido);
        espera(valido, Validacion::textoComercial(t, "El nombre", true), t, "textoComercial");
    }

    void telefono_data() {
        QTest::addColumn<QString>("t");
        QTest::addColumn<bool>("valido");
        QTest::newRow("vacio es opcional") << "" << true;
        QTest::newRow("10 digitos") << "5551234567" << true;
        QTest::newRow("internacional") << "+52 555 123 4567" << true;
        QTest::newRow("6 digitos") << "123456" << false;
        QTest::newRow("16 digitos") << "1234567890123456" << false;
        QTest::newRow("dos mas") << "++5551234567" << false;
        QTest::newRow("mas en medio") << "555+1234567" << false;
        QTest::newRow("letras") << "555-abc-1234" << false;
        QTest::newRow("solo parentesis") << "(((((((((((" << false;
        QTest::newRow("21 chars") << rep("1", 21) << false;
        QTest::newRow("sql") << "5551234'; --" << false;
        QTest::newRow("NUL") << conNul("55512", "34567") << false;
    }
    void telefono() {
        QFETCH(QString, t);
        QFETCH(bool, valido);
        espera(valido, Validacion::telefono(t), t, "telefono");
    }

    void correo_data() {
        QTest::addColumn<QString>("t");
        QTest::addColumn<bool>("valido");
        QTest::newRow("vacio opcional") << "" << true;
        QTest::newRow("simple") << "a@b.com" << true;
        QTest::newRow("subdominio+plus") << "a.b+c@sub.dominio.mx" << true;
        QTest::newRow("sin tld") << "a@b" << false;
        QTest::newRow("sin usuario") << "@b.com" << false;
        QTest::newRow("espacio") << "a b@c.com" << false;
        QTest::newRow("doble punto") << "a@b..com" << false;
        QTest::newRow("dos arrobas") << "a@@b.com" << false;
        QTest::newRow("html") << "<a@b.com>" << false;
        QTest::newRow("salto de linea final") << "a@b.com\n" << true;   // trimmed
        QTest::newRow("salto de linea medio") << "a@b.com\nbcc:x@y.com" << false;   // header injection
        QTest::newRow("101 chars") << (rep("a", 95) + "@b.com") << false;
        QTest::newRow("sql") << "a'--@b.com" << false;
        QTest::newRow("unicode local") << "ñ@b.com" << false;
    }
    void correo() {
        QFETCH(QString, t);
        QFETCH(bool, valido);
        espera(valido, Validacion::correo(t), t, "correo");
    }

    void nombreUsuario_data() {
        QTest::addColumn<QString>("t");
        QTest::addColumn<bool>("valido");
        QTest::newRow("normal") << "admin" << true;
        QTest::newRow("con simbolos permitidos") << "a.b_c-d" << true;
        QTest::newRow("vacio") << "" << false;
        QTest::newRow("2 chars") << "ab" << false;
        QTest::newRow("31 chars") << rep("a", 31) << false;
        QTest::newRow("30 chars") << rep("a", 30) << true;
        QTest::newRow("espacio") << "a b" << false;
        QTest::newRow("acento") << "ñandu" << false;
        QTest::newRow("sql") << "admin'--" << false;
        QTest::newRow("sql 2") << "' OR '1'='1" << false;
        QTest::newRow("NUL") << conNul("adm", "in") << false;
        QTest::newRow("salto interno") << "adm\nin" << false;
        QTest::newRow("mayusculas") << "ADMIN" << true;
    }
    void nombreUsuario() {
        QFETCH(QString, t);
        QFETCH(bool, valido);
        espera(valido, Validacion::nombreUsuario(t), t, "nombreUsuario");
    }

    void contrasena_data() {
        QTest::addColumn<QString>("t");
        QTest::addColumn<bool>("valido");
        QTest::newRow("valida") << "abc123" << true;
        QTest::newRow("con acento") << "Contraseña1" << true;
        QTest::newRow("vacia") << "" << false;
        QTest::newRow("5 chars") << "abc12" << false;
        QTest::newRow("sin numero") << "abcdefg" << false;
        QTest::newRow("sin letra") << "1234567" << false;
        QTest::newRow("espacio") << "abc 123" << false;
        QTest::newRow("salto final") << "abc123\n" << false;
        QTest::newRow("tab") << "abc\t123" << false;
        QTest::newRow("NUL") << conNul("abc", "123") << false;
        QTest::newRow("65 chars") << (rep("a", 64) + "1") << false;
        QTest::newRow("64 chars") << (rep("a", 63) + "1") << true;
        QTest::newRow("espacio unicode") << QString("abc 123") << false;
        QTest::newRow("espacio ideografico") << QString("abc　123") << false;
        QTest::newRow("zero width") << QString("abc​123") << true;   // documenta el comportamiento actual
    }
    void contrasena() {
        QFETCH(QString, t);
        QFETCH(bool, valido);
        espera(valido, Validacion::contrasena(t), t, "contrasena");
    }

    void codigoBarras_data() {
        QTest::addColumn<QString>("t");
        QTest::addColumn<bool>("valido");
        QTest::newRow("vacio opcional") << "" << true;
        QTest::newRow("ean13") << "7501000000011" << true;
        QTest::newRow("3 chars") << "123" << false;
        QTest::newRow("31 chars") << rep("1", 31) << false;
        QTest::newRow("espacio") << "abc def" << false;
        QTest::newRow("sql") << "1234'; --" << false;
        QTest::newRow("guion") << "7501-0000" << true;
    }
    void codigoBarras() {
        QFETCH(QString, t);
        QFETCH(bool, valido);
        espera(valido, Validacion::codigoBarras(t), t, "codigoBarras");
    }

    void numeroLote_data() {
        QTest::addColumn<QString>("t");
        QTest::addColumn<bool>("valido");
        QTest::newRow("normal") << "LOTE-A1" << true;
        QTest::newRow("simbolos") << "A/B.1_2" << true;
        QTest::newRow("vacio") << "" << false;
        QTest::newRow("espacio") << "A B" << false;
        QTest::newRow("51 chars") << rep("A", 51) << false;
        QTest::newRow("sql") << "A'; DROP" << false;
        QTest::newRow("salto interno") << "A\nB" << false;
    }
    void numeroLote() {
        QFETCH(QString, t);
        QFETCH(bool, valido);
        espera(valido, Validacion::numeroLote(t), t, "numeroLote");
    }

    void textoLibre_data() {
        QTest::addColumn<QString>("t");
        QTest::addColumn<bool>("valido");
        QTest::newRow("normal") << "Producto dañado en el traslado" << true;
        QTest::newRow("con salto") << "linea 1\nlinea 2" << true;
        QTest::newRow("script") << "<script>alert(1)</script>" << false;
        QTest::newRow("menor que") << "a < b" << false;
        QTest::newRow("NUL") << conNul("a", "b") << false;
        QTest::newRow("escape ANSI") << QString("a\x1b[31mb") << false;
        QTest::newRow("501 chars") << rep("a", 501) << false;
    }
    void textoLibre() {
        QFETCH(QString, t);
        QFETCH(bool, valido);
        espera(valido, Validacion::textoLibre(t, "El motivo", 500), t, "textoLibre");
    }

    void direccionRed_data() {
        QTest::addColumn<QString>("t");
        QTest::addColumn<bool>("valido");
        QTest::newRow("localhost") << "localhost" << true;
        QTest::newRow("ip") << "127.0.0.1" << true;
        QTest::newRow("tailscale") << "ip-172-31-41-253.tail560a17.ts.net" << true;
        QTest::newRow("con esquema") << "http://x.com" << false;
        QTest::newRow("con espacio") << "a b" << false;
        QTest::newRow("empieza con guion") << "-x" << false;
        QTest::newRow("termina con guion") << "x-" << false;
        QTest::newRow("shell") << "a;rm -rf /" << false;
        QTest::newRow("254 chars") << rep("a", 254) << false;
        QTest::newRow("vacio") << "" << false;
        QTest::newRow("con puerto") << "host:3306" << false;
    }
    void direccionRed() {
        QFETCH(QString, t);
        QFETCH(bool, valido);
        espera(valido, Validacion::direccionRed(t), t, "direccionRed");
    }

    // Ninguna validación debe tardar (ReDoS) ni caerse con textos gigantes.
    void textosGigantes() {
        const QString enorme = rep("a", 5'000'000);
        const QString patologico = rep("a ", 2'000'000) + "!";
        QElapsedTimer t;
        t.start();
        Validacion::nombrePersona(enorme, "x", true);
        Validacion::nombrePersona(patologico, "x", true);
        Validacion::textoComercial(patologico, "x", true);
        Validacion::direccion(patologico);
        Validacion::telefono(enorme);
        Validacion::correo(enorme);
        Validacion::nombreUsuario(enorme);
        Validacion::contrasena(enorme);
        Validacion::codigoBarras(enorme);
        Validacion::numeroLote(enorme);
        Validacion::textoLibre(enorme, "x", 500);
        Validacion::direccionRed(enorme);
        QVERIFY2(t.elapsed() < 3000, qPrintable(QString("Validar textos enormes tardó %1 ms").arg(t.elapsed())));
    }

    // ---- Filtros al teclear / pegar --------------------------------------------------------------------
    void filtro_teclear_nombre() {
        QLineEdit le;
        filtrarNombrePersona(&le);
        le.show();
        QTest::keyClicks(&le, "Juan123<>;'\"Perez");
        QCOMPARE(le.text(), QString("Juan'Perez"));   // los símbolos no entran; el apóstrofo sí
    }
    void filtro_pegar_rechaza_texto_invalido() {
        QLineEdit le;
        filtrarNombrePersona(&le);
        le.insert("Ana<script>");   // equivalente a pegar
        QVERIFY2(!le.text().contains('<'), qPrintable("Se coló un '<' al pegar: " + le.text()));
        le.clear();
        le.insert("Robert'); DROP TABLE x;--");
        QVERIFY2(!le.text().contains(';'), qPrintable("Se coló ';' al pegar: " + le.text()));
    }
    void filtro_largo_maximo() {
        QLineEdit le;
        filtrarNombrePersona(&le);
        QTest::keyClicks(&le, rep("a", 80));
        QCOMPARE(le.text().size(), 50);
        QLineEdit tel;
        filtrarTelefono(&tel);
        QTest::keyClicks(&tel, rep("1", 40));
        QCOMPARE(tel.text().size(), 20);
        QLineEdit usr;
        filtrarNombreUsuario(&usr);
        QTest::keyClicks(&usr, rep("a", 40));
        QCOMPARE(usr.text().size(), 30);
    }
    void filtro_telefono() {
        QLineEdit le;
        filtrarTelefono(&le);
        QTest::keyClicks(&le, "+52 (555) abc-1234;--");
        QCOMPARE(le.text(), QString("+52 (555) -1234--"));
    }
    void filtro_usuario_y_codigo() {
        QLineEdit u;
        filtrarNombreUsuario(&u);
        QTest::keyClicks(&u, "nandu 'x'; OR 1=1");
        u.insert("ñandú");
        QVERIFY2(!u.text().contains(QRegularExpression("[^A-Za-z0-9._-]")), qPrintable("Usuario con caracteres raros: " + u.text()));
        QLineEdit c;
        filtrarCodigoBarras(&c);
        QTest::keyClicks(&c, "75-01 00;00");
        c.insert("12\n34");   // pegar con salto de línea
        QVERIFY2(!c.text().contains(QRegularExpression("[^A-Za-z0-9-]")), qPrintable("Código con caracteres raros: " + c.text()));
    }
    // setText() NO pasa por el validador de Qt (documentado): por eso los formularios revalidan al guardar.
    void filtro_setText_no_filtra_pero_validador_si_atrapa() {
        QLineEdit le;
        filtrarNombrePersona(&le);
        le.setText("Juan123");
        QVERIFY(!Validacion::nombrePersona(le.text(), "El nombre", true).isEmpty());
    }

    // ---- Contraseñas -----------------------------------------------------------------------------------
    void hash_contrasena() {
        const QString sal = PasswordUtil::generarSal();
        QCOMPARE(sal.size(), 32);
        const QString h = PasswordUtil::hashear("Admin123", sal);
        QCOMPARE(h.size(), 64);
        QVERIFY(PasswordUtil::verificar("Admin123", sal, h));
        QVERIFY(!PasswordUtil::verificar("admin123", sal, h));
        QVERIFY(!PasswordUtil::verificar("Admin123 ", sal, h));
        QVERIFY(!PasswordUtil::verificar("", sal, h));
        QVERIFY(!PasswordUtil::verificar("Admin123", PasswordUtil::generarSal(), h));
        // Dos sales distintas no deben repetirse.
        QSet<QString> sales;
        for (int i = 0; i < 200; ++i) sales.insert(PasswordUtil::generarSal());
        QVERIFY2(sales.size() == 200, "Se repitieron sales");
        // Mismo hash para misma clave y misma sal; distinto con otra sal.
        QCOMPARE(PasswordUtil::hashear("x1x1x1", "sal"), PasswordUtil::hashear("x1x1x1", "sal"));
        QVERIFY(PasswordUtil::hashear("x1x1x1", "sal") != PasswordUtil::hashear("x1x1x1", "sal2"));
    }
};

FARMACIA_TEST_MAIN(TestValidacion)
#include "test_validacion.moc"
