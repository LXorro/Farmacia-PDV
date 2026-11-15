#pragma once

#include <QDate>
#include <QString>

// Reporte diario de movimientos: ventas, cobros por método, corte de efectivo, devoluciones, productos más
// vendidos, inventario, cambios registrados y alertas. Devuelve HTML (Qt rich text) que se muestra en la
// pantalla y se imprime igual a PDF. idSucursal < 0 = todas (solo un admin sobre el central).
namespace ReporteDiario {
QString generarHtml(const QDate &fecha, int idSucursal, const QString &nombreSucursal);
}
