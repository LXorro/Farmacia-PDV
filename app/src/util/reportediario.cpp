#include "reportediario.h"
#include "../database/sesionactual.h"

#include <QDateTime>
#include <QLocale>
#include <QSqlQuery>
#include <QStringList>
#include <QTimeZone>
#include <QVariantList>

namespace ReporteDiario {

namespace {

QString esc(const QString &texto) { return texto.toHtmlEscaped(); }

QString dinero(double valor) {
    return "$" + QLocale(QLocale::English).toString(valor, 'f', 2);
}

// Ejecuta una consulta con :d (día) y :s (sucursal, -1 = todas) si los usa.
QSqlQuery consulta(const QString &sql, const QDate &dia, int idSucursal) {
    QSqlQuery query;
    query.prepare(sql);
    if (sql.contains(":d"))
        query.bindValue(":d", dia.toString("yyyy-MM-dd"));
    if (sql.contains(":s"))
        query.bindValue(":s", idSucursal);
    query.exec();
    return query;
}

QString titulo(const QString &texto) {
    return QString("<h3 style='color:#0b7560; margin-top:14px; margin-bottom:4px;'>%1</h3>").arg(esc(texto));
}

// Tabla con encabezado; `alineaDerecha` marca las columnas numéricas.
QString tabla(const QStringList &encabezados, const QList<QStringList> &filas, const QList<int> &alineaDerecha = {},
              const QString &vacio = "Sin movimientos.") {
    if (filas.isEmpty())
        return QString("<p style='color:#666666;'><i>%1</i></p>").arg(esc(vacio));
    // <thead>: Qt repite el encabezado cuando la tabla continúa en otra página del PDF.
    QString html = "<table width='100%' cellspacing='0' cellpadding='4' style='border:1px solid #bbbbbb;'><thead><tr>";
    for (int i = 0; i < encabezados.size(); ++i)
        html += QString("<th align='%1' bgcolor='#e3f1ee' style='border-bottom:1px solid #bbbbbb;'>%2</th>")
                    .arg(alineaDerecha.contains(i) ? "right" : "left", esc(encabezados.at(i)));
    html += "</tr></thead>";
    for (const QStringList &fila : filas) {
        html += "<tr>";
        for (int i = 0; i < fila.size(); ++i)
            html += QString("<td align='%1' style='border-bottom:1px solid #eeeeee;'>%2</td>")
                        .arg(alineaDerecha.contains(i) ? "right" : "left", fila.at(i));
        html += "</tr>";
    }
    return html + "</table>";
}

QString celda(const QSqlQuery &q, int i) { return esc(q.value(i).toString()); }

}  // namespace

QString generarHtml(const QDate &dia, int idSucursal, const QString &nombreSucursal) {
    const QLocale es(QLocale::Spanish, QLocale::Mexico);
    const QString filtroTicket = " AND (:s < 0 OR t.id_sucursal = :s)";
    QString html;

    html += "<div style='font-family:sans-serif; font-size:10pt;'>";
    html += QString("<h2 style='color:#0b7560; margin-bottom:0;'>Farmacia — Reporte diario de movimientos</h2>");
    html += QString("<p style='margin-top:2px;'><b>%1</b><br>Sucursal: <b>%2</b><br>"
                    "<span style='color:#666666;'>Generado por %3 el %4</span></p>")
                .arg(esc(es.toString(dia, "dddd d 'de' MMMM 'de' yyyy")), esc(nombreSucursal),
                     esc(SesionActual::instance().nombreUsuario()),
                     esc(QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm")));

    // ---- Resumen ------------------------------------------------------------------------------
    double ventas = 0, cobrado = 0, devuelto = 0, porCobrar = 0;
    int tickets = 0;
    {
        QSqlQuery q = consulta("SELECT COUNT(*), COALESCE(SUM(t.total), 0), COALESCE(SUM(t.saldo_pendiente), 0) "
                               "FROM Tickets t WHERE DATE(t.fecha_venta) = :d AND t.estado_ticket <> 'cancelado'" + filtroTicket,
                               dia, idSucursal);
        if (q.next()) {
            tickets = q.value(0).toInt();
            ventas = q.value(1).toDouble();
            porCobrar = q.value(2).toDouble();
        }
        q = consulta("SELECT COALESCE(SUM(p.monto), 0) FROM Pagos p JOIN Tickets t ON t.id_ticket = p.id_ticket "
                     "WHERE DATE(p.fecha_pago) = :d AND p.estado = 'aplicado'" + filtroTicket, dia, idSucursal);
        if (q.next())
            cobrado = q.value(0).toDouble();
        q = consulta("SELECT COALESCE(SUM(d.total_devuelto), 0) FROM Devoluciones d JOIN Tickets t ON t.id_ticket = d.id_ticket "
                     "WHERE DATE(d.fecha_devolucion) = :d AND d.estado <> 'rechazada'" + filtroTicket, dia, idSucursal);
        if (q.next())
            devuelto = q.value(0).toDouble();
    }
    html += titulo("Resumen");
    html += tabla({"Concepto", "Valor"},
                  {{"Tickets del día", QString::number(tickets)},
                   {"Ventas del día", dinero(ventas)},
                   {"Ticket promedio", dinero(tickets > 0 ? ventas / tickets : 0)},
                   {"Cobrado del día (incluye abonos)", dinero(cobrado)},
                   {"Reembolsado / devuelto", dinero(devuelto)},
                   {"<b>Cobrado neto</b>", "<b>" + dinero(cobrado - devuelto) + "</b>"},
                   {"Pendiente de cobro de los tickets del día", dinero(porCobrar)}},
                  {1});

    // ---- Por sucursal (solo con todas) --------------------------------------------------------
    if (idSucursal < 0) {
        QSqlQuery q = consulta(
            "SELECT s.nombre, COUNT(t.id_ticket), COALESCE(SUM(t.total), 0) FROM Sucursales s "
            "LEFT JOIN Tickets t ON t.id_sucursal = s.id_sucursal AND DATE(t.fecha_venta) = :d AND t.estado_ticket <> 'cancelado' "
            "WHERE s.rol = 'sucursal' AND s.estado = 'activa' GROUP BY s.id_sucursal, s.nombre ORDER BY s.nombre", dia, idSucursal);
        QList<QStringList> filas;
        while (q.next())
            filas.append({celda(q, 0), q.value(1).toString(), dinero(q.value(2).toDouble())});
        html += titulo("Ventas por sucursal");
        html += tabla({"Sucursal", "Tickets", "Ventas"}, filas, {1, 2});
    }

    // ---- Cobros por método y corte de efectivo ------------------------------------------------
    {
        QSqlQuery q = consulta("SELECT p.metodo_pago, COUNT(*), SUM(p.monto) FROM Pagos p JOIN Tickets t ON t.id_ticket = p.id_ticket "
                               "WHERE DATE(p.fecha_pago) = :d AND p.estado = 'aplicado'" + filtroTicket +
                               " GROUP BY p.metodo_pago ORDER BY p.metodo_pago", dia, idSucursal);
        QList<QStringList> filas;
        double efectivo = 0;
        while (q.next()) {
            if (q.value(0).toString() == "efectivo")
                efectivo = q.value(2).toDouble();
            filas.append({celda(q, 0), q.value(1).toString(), dinero(q.value(2).toDouble())});
        }
        html += titulo("Cobros por método de pago");
        html += tabla({"Método", "Pagos", "Monto"}, filas, {1, 2}, "No hubo cobros.");

        q = consulta("SELECT COALESCE(SUM(d.total_devuelto), 0) FROM Devoluciones d JOIN Tickets t ON t.id_ticket = d.id_ticket "
                     "WHERE DATE(d.fecha_devolucion) = :d AND d.tipo_reembolso = 'efectivo' AND d.estado <> 'rechazada'" +
                     filtroTicket, dia, idSucursal);
        const double efectivoDevuelto = q.next() ? q.value(0).toDouble() : 0;
        html += titulo("Corte de efectivo");
        html += tabla({"Concepto", "Monto"},
                      {{"Efectivo cobrado", dinero(efectivo)},
                       {"Efectivo devuelto", "- " + dinero(efectivoDevuelto)},
                       {"<b>Efectivo esperado en caja</b>", "<b>" + dinero(efectivo - efectivoDevuelto) + "</b>"}},
                      {1});
        html += "<p style='color:#666666;'><i>Sin fondo inicial de caja. Los cobros con tarjeta y transferencia no entran al efectivo.</i></p>";
    }

    // ---- Devoluciones -------------------------------------------------------------------------
    {
        QSqlQuery q = consulta(
            "SELECT TIME(d.fecha_devolucion), d.id_ticket, d.tipo_reembolso, d.modalidad_tarjeta, d.total_devuelto, d.estado, "
            "d.referencia_reembolso, COALESCE(d.motivo, '') FROM Devoluciones d JOIN Tickets t ON t.id_ticket = d.id_ticket "
            "WHERE DATE(d.fecha_devolucion) = :d" + filtroTicket + " ORDER BY d.fecha_devolucion", dia, idSucursal);
        QList<QStringList> filas;
        while (q.next()) {
            QString tipo = q.value(2).toString();
            if (!q.value(3).isNull())
                tipo += q.value(3).toString() == "cancelacion" ? " · cancel." : " · devol.";
            const QString estado = q.value(5).toString() == "solicitada" ? "pendiente" : q.value(5).toString();
            filas.append({q.value(0).toTime().toString("HH:mm"), "#" + q.value(1).toString(), esc(tipo),
                          dinero(q.value(4).toDouble()), esc(estado), celda(q, 6), celda(q, 7)});
        }
        html += titulo("Devoluciones del día");
        html += tabla({"Hora", "Ticket", "Reembolso", "Importe", "Estado", "Folio", "Motivo"}, filas, {3}, "Sin devoluciones.");

        q = consulta("SELECT COUNT(*), COALESCE(SUM(d.total_devuelto), 0) FROM Devoluciones d JOIN Tickets t ON t.id_ticket = d.id_ticket "
                     "WHERE d.tipo_reembolso = 'tarjeta' AND d.estado = 'solicitada'" + filtroTicket, dia, idSucursal);
        if (q.next() && q.value(0).toInt() > 0)
            html += QString("<p style='color:#b00020;'><b>Atención:</b> %1 reembolso(s) a tarjeta por %2 siguen pendientes en la "
                            "terminal (de cualquier fecha).</p>").arg(q.value(0).toInt()).arg(dinero(q.value(1).toDouble()));
    }

    // ---- Productos más vendidos ---------------------------------------------------------------
    {
        QSqlQuery q = consulta(
            "SELECT p.nombre, SUM(v.cantidad), SUM(v.subtotal) FROM Ventas v JOIN Tickets t ON t.id_ticket = v.id_ticket "
            "JOIN Lotes l ON l.id_lote = v.id_lote JOIN Productos p ON p.id_producto = l.id_producto "
            "WHERE DATE(t.fecha_venta) = :d AND t.estado_ticket <> 'cancelado'" + filtroTicket +
            " GROUP BY p.id_producto, p.nombre ORDER BY SUM(v.cantidad) DESC, p.nombre LIMIT 10", dia, idSucursal);
        QList<QStringList> filas;
        while (q.next())
            filas.append({celda(q, 0), q.value(1).toString(), dinero(q.value(2).toDouble())});
        html += titulo("Productos más vendidos");
        html += tabla({"Producto", "Unidades", "Importe"}, filas, {1, 2}, "Sin ventas.");
    }

    // ---- Movimientos de inventario ------------------------------------------------------------
    {
        QSqlQuery q = consulta(
            "SELECT i.motivo, i.tipo_movimiento, COUNT(*), SUM(i.cantidad) FROM Inventario i JOIN Lotes l ON l.id_lote = i.id_lote "
            "WHERE DATE(i.fecha_movimiento) = :d AND (:s < 0 OR l.id_sucursal = :s) "
            "GROUP BY i.motivo, i.tipo_movimiento ORDER BY i.tipo_movimiento, i.motivo", dia, idSucursal);
        QList<QStringList> filas;
        while (q.next())
            filas.append({celda(q, 0), celda(q, 1), q.value(2).toString(), q.value(3).toString()});
        html += titulo("Movimientos de inventario");
        html += tabla({"Motivo", "Tipo", "Movimientos", "Unidades"}, filas, {2, 3});
    }

    // ---- Cambios registrados (bitácora): se guardan en UTC, el día se juzga en hora local ------
    {
        QSqlQuery q;
        q.prepare("SELECT b.fecha, s.nombre, b.nombre_usuario, b.accion, b.detalle, b.con_permiso "
                  "FROM Bitacora_Cambios b JOIN Sucursales s ON s.id_sucursal = b.id_sucursal "
                  "WHERE b.fecha >= :a AND b.fecha < :b AND (:s < 0 OR b.id_sucursal = :s) ORDER BY b.fecha");
        q.bindValue(":a", dia.addDays(-1).toString("yyyy-MM-dd"));
        q.bindValue(":b", dia.addDays(2).toString("yyyy-MM-dd"));
        q.bindValue(":s", idSucursal);
        q.exec();
        QList<QStringList> filas;
        while (q.next()) {
            QDateTime utc = q.value(0).toDateTime();
            utc.setTimeZone(QTimeZone::utc());
            const QDateTime local = utc.toLocalTime();
            if (local.date() != dia)
                continue;
            const QString accion = q.value(3).toString() == "precio_sucursal" ? "Precio en sucursal"
                                   : q.value(3).toString() == "lote_alta"      ? "Lote nuevo"
                                   : q.value(3).toString() == "lote_edicion"   ? "Lote editado" : q.value(3).toString();
            filas.append({local.toString("HH:mm"), celda(q, 1), celda(q, 2), esc(accion), celda(q, 4),
                          q.value(5).toBool() ? "permiso temporal" : "administrador"});
        }
        html += titulo("Cambios de precios y lotes (bitácora)");
        html += tabla({"Hora", "Sucursal", "Usuario", "Acción", "Detalle", "Autorización"}, filas, {}, "Sin cambios registrados.");
    }

    // ---- Alertas ------------------------------------------------------------------------------
    {
        QStringList alertas;
        QSqlQuery q = consulta("SELECT "
                               "COALESCE(SUM(CASE WHEN l.fecha_caducidad < CURDATE() THEN 1 END), 0), "
                               "COALESCE(SUM(CASE WHEN l.fecha_caducidad >= CURDATE() AND l.fecha_caducidad <= DATE_ADD(CURDATE(), INTERVAL 30 DAY) THEN 1 END), 0) "
                               "FROM Lotes l WHERE l.cantidad_disponible > 0 AND (:s < 0 OR l.id_sucursal = :s)", dia, idSucursal);
        if (q.next()) {
            if (q.value(0).toInt() > 0)
                alertas << QString("%1 lote(s) <b>caducado(s)</b> con existencias.").arg(q.value(0).toInt());
            if (q.value(1).toInt() > 0)
                alertas << QString("%1 lote(s) caducan en los próximos 30 días.").arg(q.value(1).toInt());
        }
        q = consulta("SELECT COUNT(*) FROM (SELECT ps.id_producto, ps.id_sucursal FROM Productos_Sucursal ps "
                     "JOIN Lotes l ON l.id_producto = ps.id_producto AND l.id_sucursal = ps.id_sucursal "
                     "AND (l.fecha_caducidad IS NULL OR l.fecha_caducidad >= CURDATE()) "
                     "WHERE ps.stock_minimo > 0 AND ps.activo AND (:s < 0 OR ps.id_sucursal = :s) "
                     "GROUP BY ps.id_producto, ps.id_sucursal, ps.stock_minimo HAVING SUM(l.cantidad_disponible) <= ps.stock_minimo) x",
                     dia, idSucursal);
        if (q.next() && q.value(0).toInt() > 0)
            alertas << QString("%1 producto(s) en o por debajo de su <b>mínimo de existencias</b>.").arg(q.value(0).toInt());
        q = consulta("SELECT COUNT(*), COALESCE(SUM(t.saldo_pendiente), 0) FROM Tickets t "
                     "WHERE t.saldo_pendiente > 0 AND t.estado_ticket <> 'cancelado' AND (:s < 0 OR t.id_sucursal = :s)", dia, idSucursal);
        if (q.next() && q.value(0).toInt() > 0)
            alertas << QString("%1 ticket(s) con saldo pendiente por %2 (de cualquier fecha).")
                           .arg(q.value(0).toInt()).arg(dinero(q.value(1).toDouble()));
        html += titulo("Alertas al cierre");
        if (alertas.isEmpty())
            html += "<p style='color:#666666;'><i>Sin alertas.</i></p>";
        else
            html += "<ul><li>" + alertas.join("</li><li>") + "</li></ul>";
    }

    html += "<p style='color:#888888; font-size:8pt; margin-top:16px;'>Cada sucursal registra con la hora de su equipo. "
            "Con la vista de todas las sucursales, los datos de una sucursal sin conexión con el central pueden estar incompletos.</p>";
    html += "</div>";
    return html;
}

}  // namespace ReporteDiario
