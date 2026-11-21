#include "validacion.h"

#include <QLineEdit>
#include <QMessageBox>
#include <QRegularExpression>
#include <QRegularExpressionValidator>

namespace Validacion {

namespace {

// Letras de cualquier idioma (con sus acentos) = \p{L} y \p{M}; números = \p{N}.
const char *kLetras = "\\p{L}\\p{M}";
const char *kApostrofos = "'’";

QRegularExpression regex(const QString &patron) {
    return QRegularExpression(patron, QRegularExpression::UseUnicodePropertiesOption);
}

bool coincide(const QString &patron, const QString &texto) {
    return regex(patron).match(texto).hasMatch();
}

bool tieneLetra(const QString &texto) {
    for (const QChar c : texto)
        if (c.isLetter())
            return true;
    return false;
}

bool tieneControl(const QString &texto, bool permitirSaltos) {
    for (const QChar c : texto) {
        if (c.category() == QChar::Other_Control || c == QChar(0x7f) || c.unicode() < 0x20) {
            if (permitirSaltos && (c == '\n' || c == '\r' || c == '\t'))
                continue;
            return true;
        }
    }
    return false;
}

// Patrones (anclados) y su versión «mientras se teclea» (un carácter más se acepta si la regla lo permite).
// Palabras de letras unidas por espacio, apóstrofo, guion o «. » (inicial con punto); puede terminar en punto.
QString patNombre() {
    return QString("^[%1]+(?:(?:\\. ?|[ %2-])[%1]+)*\\.?$").arg(kLetras, kApostrofos);
}
QString filtroNombre() { return QString("[%1 %2.-]*").arg(kLetras, kApostrofos); }

QString filtroComercial() { return QString("[%1\\p{N} .,%2&()/%+#°ºª:-]*").arg(kLetras, kApostrofos); }
QString patComercial() { return QString("^[%1\\p{N}][%1\\p{N} .,%2&()/%+#°ºª:-]*$").arg(kLetras, kApostrofos); }

QString filtroDireccion() { return QString("[%1\\p{N} .,#°ºª/%2()&:-]*").arg(kLetras, kApostrofos); }

}   // namespace

QString normalizar(const QString &texto) {
    return texto.simplified();   // quita los extremos y junta cualquier racha de espacios o tabuladores
}

QString nombrePersona(const QString &texto, const QString &campo, bool obligatorio) {
    const QString t = normalizar(texto);
    if (t.isEmpty())
        return obligatorio ? campo + " es obligatorio." : QString();
    if (t.size() < 2)
        return campo + " debe tener al menos 2 letras.";
    if (t.size() > 50)
        return campo + " no puede pasar de 50 caracteres.";
    if (!coincide(patNombre(), t))
        return campo + " solo puede llevar letras, espacios, apóstrofo, guion y punto. "
                       "No se admiten números ni símbolos.";
    return QString();
}

QString textoComercial(const QString &texto, const QString &campo, bool obligatorio, int largoMaximo) {
    const QString t = normalizar(texto);
    if (t.isEmpty())
        return obligatorio ? campo + " es obligatorio." : QString();
    if (t.size() < 2)
        return campo + " debe tener al menos 2 caracteres.";
    if (t.size() > largoMaximo)
        return QString("%1 no puede pasar de %2 caracteres.").arg(campo).arg(largoMaximo);
    if (!coincide(patComercial(), t) || !tieneLetra(t))
        return campo + " solo puede llevar letras, números, espacios y los signos . , ' & ( ) / % + # : - °; "
                       "debe empezar con letra o número y llevar al menos una letra.";
    return QString();
}

QString direccion(const QString &texto) {
    const QString t = normalizar(texto);
    if (t.isEmpty())
        return QString();
    if (t.size() > 200)
        return "La dirección no puede pasar de 200 caracteres.";
    if (!coincide(QString("^[%1\\p{N} .,#°ºª/%2()&:-]+$").arg(kLetras, kApostrofos), t))
        return "La dirección solo puede llevar letras, números, espacios y los signos . , # / ( ) & : - °.";
    return QString();
}

QString telefono(const QString &texto) {
    const QString t = texto.trimmed();
    if (t.isEmpty())
        return QString();
    if (t.size() > 20)
        return "El teléfono no puede pasar de 20 caracteres.";
    // «+» solo al inicio; el resto, dígitos, espacios, guiones y paréntesis.
    if (!coincide("^\\+?[0-9 ()-]+$", t))
        return "El teléfono solo puede llevar números, espacios, guiones, paréntesis y un + al inicio.";
    int digitos = 0;
    for (const QChar c : t)
        digitos += c.isDigit();
    if (digitos < 7 || digitos > 15)
        return "El teléfono debe tener entre 7 y 15 dígitos.";
    return QString();
}

QString correo(const QString &texto) {
    const QString t = texto.trimmed();
    if (t.isEmpty())
        return QString();
    if (t.size() > 100)
        return "El correo no puede pasar de 100 caracteres.";
    if (!coincide("^[A-Za-z0-9._%+-]+@[A-Za-z0-9-]+(?:\\.[A-Za-z0-9-]+)*\\.[A-Za-z]{2,}$", t))
        return "El correo no es válido. Debe tener la forma nombre@dominio.com, sin espacios.";
    return QString();
}

QString nombreUsuario(const QString &texto) {
    const QString t = texto.trimmed();
    if (t.isEmpty())
        return "El nombre de usuario es obligatorio.";
    if (!coincide("^[A-Za-z0-9._-]{3,30}$", t))
        return "El nombre de usuario debe tener de 3 a 30 caracteres: letras sin acentos, números, punto, "
               "guion y guion bajo (sin espacios).";
    return QString();
}

QString contrasena(const QString &texto) {
    if (texto.isEmpty())
        return "La contraseña es obligatoria.";
    if (texto.size() < 6)
        return "La contraseña debe tener al menos 6 caracteres.";
    if (texto.size() > 64)
        return "La contraseña no puede pasar de 64 caracteres.";
    for (const QChar c : texto)
        if (c.isSpace() || c.unicode() < 0x20 || c == QChar(0x7f))
            return "La contraseña no puede llevar espacios ni caracteres de control.";
    bool letra = false, numero = false;
    for (const QChar c : texto) {
        letra = letra || c.isLetter();
        numero = numero || c.isDigit();
    }
    if (!letra || !numero)
        return "La contraseña debe llevar al menos una letra y un número.";
    return QString();
}

QString codigoBarras(const QString &texto) {
    const QString t = texto.trimmed();
    if (t.isEmpty())
        return QString();
    if (!coincide("^[A-Za-z0-9-]{4,30}$", t))
        return "El código de barras solo puede llevar letras, números y guion (de 4 a 30 caracteres, sin espacios).";
    return QString();
}

QString numeroLote(const QString &texto) {
    const QString t = texto.trimmed();
    if (t.isEmpty())
        return "El número de lote es obligatorio.";
    if (!coincide("^[A-Za-z0-9._/-]{1,50}$", t))
        return "El número de lote solo puede llevar letras, números y los signos . _ / - (hasta 50, sin espacios).";
    return QString();
}

QString textoLibre(const QString &texto, const QString &campo, int largoMaximo) {
    if (texto.size() > largoMaximo)
        return QString("%1 no puede pasar de %2 caracteres.").arg(campo).arg(largoMaximo);
    if (tieneControl(texto, true))
        return campo + " tiene caracteres no permitidos.";
    if (texto.contains('<') || texto.contains('>'))
        return campo + " no puede llevar los signos < ni >.";
    return QString();
}

QString direccionRed(const QString &texto) {
    const QString t = texto.trimmed();
    if (t.isEmpty())
        return "Indica la dirección del central.";
    if (t.size() > 253 || !coincide("^[A-Za-z0-9]([A-Za-z0-9.-]*[A-Za-z0-9])?$", t))
        return "La dirección del central solo puede llevar letras, números, puntos y guiones (sin espacios ni "
               "«http://»). Usa el nombre del equipo, por ejemplo ip-172-31-41-253.tail560a17.ts.net.";
    return QString();
}

// ---- Filtros ----------------------------------------------------------------------------------------

namespace {
void poner(QLineEdit *campo, const QString &patron, int largo) {
    if (!campo)
        return;
    campo->setMaxLength(largo);
    campo->setValidator(new QRegularExpressionValidator(regex(patron), campo));
}
}   // namespace

void filtrarNombrePersona(QLineEdit *campo) { poner(campo, filtroNombre(), 50); }
void filtrarTextoComercial(QLineEdit *campo, int largoMaximo) { poner(campo, filtroComercial(), largoMaximo); }
void filtrarDireccion(QLineEdit *campo) { poner(campo, filtroDireccion(), 200); }
void filtrarTelefono(QLineEdit *campo) { poner(campo, "\\+?[0-9 ()-]*", 20); }
void filtrarCorreo(QLineEdit *campo) { poner(campo, "[A-Za-z0-9._%+@-]*", 100); }
void filtrarNombreUsuario(QLineEdit *campo) { poner(campo, "[A-Za-z0-9._-]*", 30); }
void filtrarCodigoBarras(QLineEdit *campo) { poner(campo, "[A-Za-z0-9-]*", 30); }
void filtrarNumeroLote(QLineEdit *campo) { poner(campo, "[A-Za-z0-9._/-]*", 50); }
void limitarLargo(QLineEdit *campo, int largo) {
    if (campo)
        campo->setMaxLength(largo);
}

bool rechazar(QWidget *padre, QLineEdit *campo, const QString &error) {
    if (error.isEmpty())
        return false;
    QMessageBox::warning(padre, "Validación", error);
    if (campo) {
        campo->setFocus();
        campo->selectAll();
    }
    return true;
}

}   // namespace Validacion
