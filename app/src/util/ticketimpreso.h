#pragma once

#include <QDate>
#include <QDateTime>
#include <QString>
#include <QStringList>
#include <QVector>

class QPagedPaintDevice;
class QPrinter;

// Ticket de venta en texto plano por columnas (impresora térmica de 80 mm). Todo el layout trabaja en
// columnas con una fuente de ancho fijo, así que ninguna línea puede partirse sola: cada línea mide
// exactamente lo que se calculó. El mismo texto sirve para la vista en pantalla, el PDF y la impresora.
namespace TicketImpreso {

struct Linea {
    int cantidad = 0;
    QString descripcion;
    double precio = 0;      // precio unitario (con IVA incluido)
    double importe = 0;     // cantidad × precio
    double tasaIva = 16.0;  // porcentaje: 0 o 16
};

struct PagoLinea {
    QString metodo;         // efectivo | tarjeta | transferencia
    double monto = 0;
    QString referencia;     // autorización (tarjeta) o referencia (transferencia)
    QString ultimos;        // últimos 4 dígitos de la tarjeta
};

struct DevolucionLinea {
    QDateTime fecha;
    double monto = 0;
    QString tipo;           // efectivo | transferencia | tarjeta | cambio | saldo_a_favor
    QString estado;         // solicitada | completada…
    QString folio;          // folio del reembolso a tarjeta
};

struct Datos {
    // Establecimiento
    QString nombreComercial, razonSocial, rfc, domicilio, telefono, sucursal, pie;
    // Venta
    int folio = 0;
    QDateTime fecha;
    QString cajero, cliente, modalidad;   // modalidad: contado | diferido
    QVector<Linea> lineas;
    double descuento = 0;
    double totalGuardado = -1;            // Tickets.total si se leyó de la base (<0: se calcula)
    QVector<PagoLinea> pagos;
    double efectivoRecibido = -1;         // <0: no se conoce (p. ej. al reimprimir)
    double cambio = 0;
    double saldo = 0;
    QDate fechaLimite;
    QVector<DevolucionLinea> devoluciones;
};

struct Configuracion {
    int columnas = 42;            // 80 mm: 40-48 (48 es lo máximo de la fuente A; 42 deja margen)
    double anchoMm = 80.0;
    QString impresora;            // vacío: preguntar al imprimir
    QString nombreComercial, razonSocial, rfc, domicilio, telefono, pie;
    bool medicamentoIvaCero = true;   // categoría «medicamento» a tasa 0 %, el resto a 16 % (validar con contador)

    static Configuracion cargar();
    void guardar() const;
    static QString pieInicial();
    static bool rfcValido(const QString &rfc);   // vacío = válido (aún no configurado)
};

// Texto del ticket, una cadena por línea, ninguna más ancha que `columnas`.
QStringList renderizar(const Datos &datos, int columnas);

// Lee un ticket de la conexión por defecto (la base de la sucursal o, para un admin, la del central).
bool cargar(int idTicket, const Configuracion &config, Datos &datos, QString &error);

// Salida. `lineas` es lo que devolvió renderizar().
bool exportarPdf(const QString &ruta, const QStringList &lineas, const Configuracion &config);
bool imprimir(QPrinter &impresora, const QStringList &lineas, const Configuracion &config);

// Fuente de ancho fijo del ticket (Space Mono, incluida en la app; si no, la monoespaciada del sistema).
QString familiaFuente();

}   // namespace TicketImpreso
