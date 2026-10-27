#pragma once

#include <QString>

class QTableView;

// Formato común para todas las tablas de la app, sin tocar los modelos:
//  - importes como moneda ($1,234.50) y alineados a la derecha,
//  - fechas dd/MM/yyyy, y caducidades coloreadas (vencida / próxima a vencer),
//  - columnas de estado como "píldoras" de color (Pagado, Pendiente, Caducado...),
//  - aviso centrado cuando la tabla no tiene filas.
// Las columnas se reconocen por el texto de su encabezado. Funciona con
// QTableView (cualquier modelo) y con QTableWidget.
namespace Tablas {
void aplicar(QTableView *vista, const QString &textoVacio = "Sin registros");
}
