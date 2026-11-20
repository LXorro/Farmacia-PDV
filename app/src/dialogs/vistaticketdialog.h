#pragma once

#include <QDialog>
#include <QStringList>

#include "../util/ticketimpreso.h"

class QLabel;
class QPlainTextEdit;

// Muestra el ticket tal como saldría en el papel (misma fuente, mismas columnas) y permite imprimirlo,
// guardarlo en PDF o ajustar el ticket. No necesita impresora: sirve para verlo y como comprobante en PDF.
class VistaTicketDialog : public QDialog {
    Q_OBJECT
public:
    // `efectivoRecibido` y `cambio` solo se conocen al cobrar (no se guardan en la base).
    explicit VistaTicketDialog(int idTicket, double efectivoRecibido = -1, double cambio = 0, QWidget *parent = nullptr);

private slots:
    void configurar();
    void imprimir();
    void guardarPdf();

private:
    void recargar();

    int m_idTicket;
    double m_efectivoRecibido;
    double m_cambio;
    TicketImpreso::Configuracion m_config;
    QStringList m_lineas;
    QLabel *m_info;
    QPlainTextEdit *m_papel;
};
