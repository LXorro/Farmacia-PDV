#include "ticketimpreso.h"

#include <QFont>
#include <QFontDatabase>
#include <QFontMetricsF>
#include <QImage>
#include <QMap>
#include <QPageSize>
#include <QPainter>
#include <QPdfWriter>
#include <QPrinter>
#include <QRegularExpression>
#include <QSettings>
#include <QSqlQuery>
#include <QtMath>

namespace TicketImpreso {

// ---- Configuración -----------------------------------------------------------------------------

QString Configuracion::pieInicial() {
    return "¡Gracias por su compra!\n"
           "Conserve este ticket para cambios y devoluciones.\n"
           "Este ticket no es una factura.";
}

bool Configuracion::rfcValido(const QString &rfc) {
    if (rfc.trimmed().isEmpty())
        return true;
    // Persona moral: 3 letras + 6 dígitos de fecha + 3 de homoclave; persona física: 4 letras.
    static const QRegularExpression patron("^[A-ZÑ&]{3,4}\\d{6}[A-Z0-9]{3}$");
    return patron.match(rfc.trimmed().toUpper()).hasMatch();
}

Configuracion Configuracion::cargar() {
    QSettings s;
    Configuracion c;
    c.columnas = qBound(32, s.value("ticket/columnas", c.columnas).toInt(), 64);
    c.anchoMm = qBound(40.0, s.value("ticket/ancho_mm", c.anchoMm).toDouble(), 120.0);
    c.impresora = s.value("ticket/impresora").toString();
    c.nombreComercial = s.value("ticket/nombre").toString();
    c.razonSocial = s.value("ticket/razon_social").toString();
    c.rfc = s.value("ticket/rfc").toString();
    c.domicilio = s.value("ticket/domicilio").toString();
    c.telefono = s.value("ticket/telefono").toString();
    c.pie = s.value("ticket/pie", pieInicial()).toString();
    c.medicamentoIvaCero = s.value("ticket/medicamento_iva_cero", true).toBool();
    return c;
}

void Configuracion::guardar() const {
    QSettings s;
    s.setValue("ticket/columnas", columnas);
    s.setValue("ticket/ancho_mm", anchoMm);
    s.setValue("ticket/impresora", impresora);
    s.setValue("ticket/nombre", nombreComercial);
    s.setValue("ticket/razon_social", razonSocial);
    s.setValue("ticket/rfc", rfc.trimmed().toUpper());
    s.setValue("ticket/domicilio", domicilio);
    s.setValue("ticket/telefono", telefono);
    s.setValue("ticket/pie", pie);
    s.setValue("ticket/medicamento_iva_cero", medicamentoIvaCero);
}

// ---- Texto por columnas ------------------------------------------------------------------------

namespace {

double redondear(double x) { return qRound64(x * 100.0) / 100.0; }

QString dinero(double x) { return QString::number(x, 'f', 2); }

QString tasaTexto(double tasa) {
    const bool entera = qAbs(tasa - qRound(tasa)) < 1e-9;
    return (entera ? QString::number(qRound(tasa)) : QString::number(tasa, 'f', 2)) + "%";
}

// Parte `texto` por palabras en líneas de a lo sumo `ancho`; una palabra más larga se corta.
QStringList envolver(const QString &texto, int ancho) {
    QStringList lineas;
    for (const QString &parrafo : texto.split('\n')) {
        QString actual;
        for (QString palabra : parrafo.split(' ', Qt::SkipEmptyParts)) {
            while (palabra.size() > ancho) {
                if (!actual.isEmpty()) {
                    lineas << actual;
                    actual.clear();
                }
                lineas << palabra.left(ancho);
                palabra = palabra.mid(ancho);
            }
            if (actual.isEmpty())
                actual = palabra;
            else if (actual.size() + 1 + palabra.size() <= ancho)
                actual += ' ' + palabra;
            else {
                lineas << actual;
                actual = palabra;
            }
        }
        if (!actual.isEmpty() || parrafo.isEmpty())
            lineas << actual;
    }
    return lineas;
}

QString derecha(const QString &t, int ancho) { return t.rightJustified(ancho, ' ', true); }

QString sinFinal(QString t) {
    while (t.endsWith(' '))
        t.chop(1);
    return t;
}

class Constructor {
public:
    explicit Constructor(int columnas) : m_cols(columnas) {}

    void linea(const QString &t = QString()) { m_lineas << sinFinal(t.left(m_cols)); }
    void separador(QChar c = '-') { m_lineas << QString(m_cols, c); }

    void centrado(const QString &texto) {
        for (const QString &l : envolver(texto, m_cols))
            m_lineas << QString((m_cols - l.size()) / 2, ' ') + l;
    }

    // «Etiqueta: valor» que, si no cabe, sigue en las líneas siguientes con 2 espacios más de sangría.
    void campo(const QString &texto, int sangria = 0) {
        const QString margen(sangria, ' ');
        QStringList partes = envolver(texto.trimmed(), m_cols - sangria);
        if (partes.isEmpty())
            return;
        m_lineas << sinFinal(margen + partes.takeFirst());
        if (partes.isEmpty())
            return;
        for (const QString &l : envolver(partes.join(' '), m_cols - sangria - 2))
            m_lineas << sinFinal(margen + "  " + l);
    }

    // Texto a la izquierda y cantidad a la derecha en la misma línea.
    void fila(const QString &izq, const QString &der) {
        const int hueco = m_cols - der.size() - 1;
        const QString i = izq.size() > hueco ? izq.left(qMax(0, hueco)) : izq;
        m_lineas << i + QString(qMax(1, m_cols - i.size() - der.size()), ' ') + der;
    }

    QStringList resultado() const { return m_lineas; }
    int columnas() const { return m_cols; }

private:
    int m_cols;
    QStringList m_lineas;
};

QString etiquetaMetodo(const PagoLinea &p) {
    if (p.metodo == "tarjeta")
        return p.ultimos.isEmpty() ? QString("Tarjeta") : QString("Tarjeta ****%1").arg(p.ultimos);
    if (p.metodo == "transferencia")
        return "Transferencia";
    return "Efectivo";
}

QString etiquetaReembolso(const QString &tipo) {
    QString t = tipo;
    t.replace('_', ' ');
    return t;
}

}   // namespace

QStringList renderizar(const Datos &d, int columnas) {
    const int cols = qMax(32, columnas);
    Constructor c(cols);

    // ---- Encabezado del establecimiento
    c.centrado(d.nombreComercial.trimmed().isEmpty() ? QString("FARMACIA") : d.nombreComercial.trimmed().toUpper());
    if (!d.razonSocial.trimmed().isEmpty())
        c.centrado(d.razonSocial.trimmed());
    if (!d.rfc.trimmed().isEmpty())
        c.centrado("RFC: " + d.rfc.trimmed().toUpper());
    if (!d.domicilio.trimmed().isEmpty())
        c.centrado(d.domicilio.trimmed());
    if (!d.telefono.trimmed().isEmpty())
        c.centrado("Tel. " + d.telefono.trimmed());
    if (!d.sucursal.trimmed().isEmpty())
        c.centrado(d.sucursal.trimmed());
    c.separador('=');

    // ---- Datos de la venta
    c.fila(d.modalidad == "diferido" ? "TICKET - VENTA A CRÉDITO" : "TICKET DE VENTA",
           QString("Folio %1").arg(d.folio, 6, 10, QChar('0')));
    c.linea("Fecha: " + (d.fecha.isValid() ? d.fecha.toString("dd/MM/yyyy HH:mm") : QString("-")));
    c.campo("Atendió: " + (d.cajero.isEmpty() ? QString("-") : d.cajero));
    c.campo("Cliente: " + (d.cliente.isEmpty() ? QString("Venta de mostrador") : d.cliente));
    c.separador();

    // ---- Líneas: CANT  DESCRIPCION  P.UNIT  IMPORTE
    int anchoImporte = 9, anchoPrecio = 8;
    const int anchoCant = 4;
    for (const Linea &l : d.lineas) {
        anchoImporte = qMax(anchoImporte, dinero(l.importe).size());
        anchoPrecio = qMax(anchoPrecio, dinero(l.precio).size());
    }
    int anchoDesc = cols - anchoCant - anchoPrecio - anchoImporte - 3;
    while (anchoDesc < 12 && (anchoPrecio > 7 || anchoImporte > 8)) {
        if (anchoPrecio > 7) --anchoPrecio; else --anchoImporte;
        ++anchoDesc;
    }
    anchoDesc = qMax(anchoDesc, 8);

    c.linea(derecha("CANT", anchoCant) + ' ' + QString("DESCRIPCION").left(anchoDesc).leftJustified(anchoDesc) + ' '
            + derecha("P.UNIT", anchoPrecio) + ' ' + derecha("IMPORTE", anchoImporte));
    for (const Linea &l : d.lineas) {
        const QStringList desc = envolver(l.descripcion.isEmpty() ? QString("-") : l.descripcion, anchoDesc);
        for (int i = 0; i < desc.size(); ++i) {
            const QString izq = (i == 0 ? derecha(QString::number(l.cantidad), anchoCant) : QString(anchoCant, ' '))
                                + ' ' + desc.at(i).leftJustified(anchoDesc);
            if (i == 0)
                c.linea(izq + ' ' + derecha(dinero(l.precio), anchoPrecio) + ' ' + derecha(dinero(l.importe), anchoImporte));
            else
                c.linea(izq);
        }
    }
    c.separador();

    // ---- Totales con el IVA desglosado (los precios ya lo incluyen)
    double bruto = 0;
    QMap<double, double> brutoPorTasa;
    for (const Linea &l : d.lineas) {
        bruto += l.importe;
        brutoPorTasa[l.tasaIva] += l.importe;
    }
    const double total = d.totalGuardado >= 0 ? d.totalGuardado : redondear(bruto - d.descuento);
    const double factor = bruto > 0 ? total / bruto : 1.0;   // reparte un descuento entre las tasas
    double subtotal = 0;
    QVector<QPair<QString, double>> ivas;
    for (auto it = brutoPorTasa.constBegin(); it != brutoPorTasa.constEnd(); ++it) {
        const double conIva = it.value() * factor;
        const double base = redondear(conIva / (1.0 + it.key() / 100.0));
        subtotal += base;
        ivas.append({QString("IVA %1 (base $%2)").arg(tasaTexto(it.key()), dinero(base)), redondear(conIva - base)});
    }
    c.fila("SUBTOTAL (sin IVA)", "$" + dinero(subtotal));
    for (const auto &iva : ivas)
        c.fila(iva.first, "$" + dinero(iva.second));
    if (d.descuento > 0)
        c.fila("DESCUENTO", "-$" + dinero(d.descuento));
    c.fila("TOTAL", "$" + dinero(total));
    c.linea("(Precios con IVA incluido)");
    c.separador();

    // ---- Pagos
    double pagado = 0;
    for (const PagoLinea &p : d.pagos)
        pagado += p.monto;
    const bool credito = d.modalidad == "diferido";
    if (credito)
        c.linea(d.pagos.isEmpty() ? "ABONOS: ninguno" : "ABONOS");
    else
        c.linea(d.pagos.size() > 1 ? "FORMA DE PAGO (MIXTO)" : "FORMA DE PAGO");
    for (const PagoLinea &p : d.pagos) {
        c.fila(etiquetaMetodo(p), "$" + dinero(p.monto));
        if (!p.referencia.isEmpty())
            c.campo(QString("%1: %2").arg(p.metodo == "tarjeta" ? "Autorización" : "Referencia", p.referencia), 2);
    }
    if (d.efectivoRecibido > 0) {
        c.fila("Efectivo recibido", "$" + dinero(d.efectivoRecibido));
        c.fila("Cambio", "$" + dinero(d.cambio));
    }
    if (credito) {
        c.fila("Pagado", "$" + dinero(pagado));
        c.fila("SALDO PENDIENTE", "$" + dinero(d.saldo));
        if (d.fechaLimite.isValid())
            c.linea("Fecha límite: " + d.fechaLimite.toString("dd/MM/yyyy"));
    }

    // ---- Devoluciones registradas
    if (!d.devoluciones.isEmpty()) {
        c.separador();
        c.linea("DEVOLUCIONES REGISTRADAS");
        for (const DevolucionLinea &dv : d.devoluciones) {
            c.fila(dv.fecha.toString("dd/MM HH:mm") + " " + etiquetaReembolso(dv.tipo), "$" + dinero(dv.monto));
            QString estado = dv.estado == "solicitada" && dv.tipo == "tarjeta" ? QString("pendiente en terminal")
                                                                              : dv.estado;
            if (!dv.folio.isEmpty())
                estado += " - folio " + dv.folio;
            c.campo("Estado: " + estado, 2);
        }
    }

    // ---- Pie
    c.separador('=');
    for (const QString &l : d.pie.split('\n'))
        if (!l.trimmed().isEmpty())
            c.centrado(l.trimmed());
    c.linea();
    return c.resultado();
}

// ---- Lectura de la base --------------------------------------------------------------------------

bool cargar(int idTicket, const Configuracion &config, Datos &d, QString &error) {
    QSqlQuery q;
    q.prepare(
        "SELECT s.nombre, s.direccion, s.telefono, t.fecha_venta, t.descuento, t.total, t.modalidad_pago, "
        "t.saldo_pendiente, t.fecha_limite_pago, CONCAT_WS(' ', e.nombre, e.apellido_paterno), "
        "CONCAT_WS(' ', c.nombre, c.apellido_paterno) "
        "FROM Tickets t JOIN Sucursales s ON s.id_sucursal = t.id_sucursal "
        "LEFT JOIN Empleados e ON e.id_empleado = t.id_empleado "
        "LEFT JOIN Clientes c ON c.id_cliente = t.id_cliente WHERE t.id_ticket = :id");
    q.bindValue(":id", idTicket);
    if (!q.exec() || !q.next()) {
        error = "No se encontró el ticket #" + QString::number(idTicket) + ".";
        return false;
    }
    d = Datos();
    d.folio = idTicket;
    d.nombreComercial = config.nombreComercial;
    d.razonSocial = config.razonSocial;
    d.rfc = config.rfc;
    d.domicilio = config.domicilio.trimmed().isEmpty() ? q.value(1).toString() : config.domicilio;
    d.telefono = config.telefono.trimmed().isEmpty() ? q.value(2).toString() : config.telefono;
    d.sucursal = q.value(0).toString();
    d.pie = config.pie;
    d.fecha = q.value(3).toDateTime();
    d.descuento = q.value(4).toDouble();
    d.totalGuardado = q.value(5).toDouble();
    d.modalidad = q.value(6).toString();
    d.saldo = q.value(7).toDouble();
    d.fechaLimite = q.value(8).toDate();
    d.cajero = q.value(9).toString().trimmed();
    d.cliente = q.value(10).toString().trimmed();

    QSqlQuery lineas;
    lineas.prepare(
        "SELECT p.nombre, v.cantidad, v.precio_unitario, v.subtotal, cat.tipo "
        "FROM Ventas v JOIN Lotes l ON l.id_lote = v.id_lote JOIN Productos p ON p.id_producto = l.id_producto "
        "LEFT JOIN Categorias cat ON cat.id_categoria = p.id_categoria WHERE v.id_ticket = :id ORDER BY v.id_venta");
    lineas.bindValue(":id", idTicket);
    lineas.exec();
    while (lineas.next()) {
        Linea l;
        l.descripcion = lineas.value(0).toString();
        l.cantidad = lineas.value(1).toInt();
        l.precio = lineas.value(2).toDouble();
        l.importe = lineas.value(3).toDouble();
        l.tasaIva = (config.medicamentoIvaCero && lineas.value(4).toString() == "medicamento") ? 0.0 : 16.0;
        d.lineas.append(l);
    }

    QSqlQuery pagos;
    pagos.prepare("SELECT metodo_pago, monto, numero_referencia, ultimos_digitos FROM Pagos "
                  "WHERE id_ticket = :id AND estado = 'aplicado' ORDER BY fecha_pago, id_pago");
    pagos.bindValue(":id", idTicket);
    pagos.exec();
    while (pagos.next()) {
        PagoLinea p;
        p.metodo = pagos.value(0).toString();
        p.monto = pagos.value(1).toDouble();
        p.referencia = pagos.value(2).toString();
        p.ultimos = pagos.value(3).toString();
        d.pagos.append(p);
    }

    QSqlQuery devs;
    devs.prepare("SELECT fecha_devolucion, total_devuelto, tipo_reembolso, estado, referencia_reembolso "
                 "FROM Devoluciones WHERE id_ticket = :id ORDER BY fecha_devolucion, id_devolucion");
    devs.bindValue(":id", idTicket);
    devs.exec();
    while (devs.next()) {
        DevolucionLinea dv;
        dv.fecha = devs.value(0).toDateTime();
        dv.monto = devs.value(1).toDouble();
        dv.tipo = devs.value(2).toString();
        dv.estado = devs.value(3).toString();
        dv.folio = devs.value(4).toString();
        d.devoluciones.append(dv);
    }
    return true;
}

// ---- PDF e impresión -------------------------------------------------------------------------------

QString familiaFuente() {
    static const QString preferida = "Space Mono";
    if (QFontDatabase::families().contains(preferida))
        return preferida;
    return QFontDatabase::systemFont(QFontDatabase::FixedFont).family();
}

namespace {

constexpr double kMargenMm = 4.0;

QFont fuente(double puntos) {
    QFont f(familiaFuente());
    f.setStyleHint(QFont::Monospace);
    f.setFixedPitch(true);
    f.setPointSizeF(puntos);
    return f;
}

// Alto aproximado del ticket en mm para un papel de `anchoMm` (para fijar el largo del rollo).
double altoMm(int lineas, int columnas, double anchoMm) {
    QImage patron(1, 1, QImage::Format_ARGB32);
    patron.setDotsPerMeterX(3937);   // 100 dpi: solo se usa la proporción
    patron.setDotsPerMeterY(3937);
    QFontMetricsF fm(fuente(100), &patron);
    const double anchoUtil = anchoMm - 2 * kMargenMm;
    const double ancho100 = fm.horizontalAdvance(QString(columnas, '0')) / 100.0 * 25.4;
    const double escala = anchoUtil / ancho100;
    const double linea = fm.lineSpacing() / 100.0 * 25.4 * escala;
    return 2 * kMargenMm + (lineas + 2) * linea;
}

bool dibujar(QPagedPaintDevice &dispositivo, const QStringList &lineas, const Configuracion &config) {
    QPainter p(&dispositivo);
    if (!p.isActive())
        return false;
    // Tamaño de letra para que `columnas` caracteres ocupen justo el ancho imprimible.
    QFont f = fuente(10);
    QFontMetricsF fm(f, p.device());
    const double ancho = p.device()->width();
    f.setPointSizeF(qMax(2.0, qFloor(10.0 * ancho / fm.horizontalAdvance(QString(config.columnas, '0')) * 100) / 100.0));
    p.setFont(f);
    fm = QFontMetricsF(f, p.device());

    double y = 0;
    const double alto = p.device()->height();
    for (const QString &texto : lineas) {
        if (y + fm.lineSpacing() > alto) {
            dispositivo.newPage();
            y = 0;
        }
        p.drawText(QPointF(0, y + fm.ascent()), texto);
        y += fm.lineSpacing();
    }
    return p.end();
}

}   // namespace

bool exportarPdf(const QString &ruta, const QStringList &lineas, const Configuracion &config) {
    QPdfWriter pdf(ruta);
    pdf.setResolution(300);
    pdf.setPageSize(QPageSize(QSizeF(config.anchoMm, altoMm(lineas.size(), config.columnas, config.anchoMm)),
                              QPageSize::Millimeter, "Ticket"));
    pdf.setPageMargins(QMarginsF(kMargenMm, kMargenMm, kMargenMm, kMargenMm), QPageLayout::Millimeter);
    pdf.setTitle("Ticket");
    return dibujar(pdf, lineas, config);
}

bool imprimir(QPrinter &impresora, const QStringList &lineas, const Configuracion &config) {
    // Rollo de ancho fijo y largo según el ticket; si el driver no admite un tamaño propio se imprime en el
    // papel que tenga, con la letra ajustada a su ancho y saltando de página si hace falta.
    impresora.setPageSize(QPageSize(QSizeF(config.anchoMm, altoMm(lineas.size(), config.columnas, config.anchoMm)),
                                    QPageSize::Millimeter, "Ticket"));
    impresora.setPageMargins(QMarginsF(kMargenMm, kMargenMm, kMargenMm, kMargenMm), QPageLayout::Millimeter);
    return dibujar(impresora, lineas, config);
}

}   // namespace TicketImpreso
