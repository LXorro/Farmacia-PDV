#include "vistaticketdialog.h"
#include "configurarticketdialog.h"

#include <QDir>
#include <QFileDialog>
#include <QFontMetricsF>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPrintDialog>
#include <QPrinter>
#include <QPrinterInfo>
#include <QPushButton>
#include <QStandardPaths>
#include <QVBoxLayout>

VistaTicketDialog::VistaTicketDialog(int idTicket, double efectivoRecibido, double cambio, QWidget *parent)
    : QDialog(parent), m_idTicket(idTicket), m_efectivoRecibido(efectivoRecibido), m_cambio(cambio) {
    setWindowTitle(QString("Ticket #%1").arg(idTicket));

    m_info = new QLabel(this);
    m_info->setWordWrap(true);

    m_papel = new QPlainTextEdit(this);
    m_papel->setReadOnly(true);
    m_papel->setLineWrapMode(QPlainTextEdit::NoWrap);
    // La fuente va en la hoja de estilos de este widget: la de la aplicación manda sobre setFont() y
    // sin esto la letra dejaría de ser de ancho fijo y las columnas se verían desalineadas.
    m_papel->setStyleSheet(QString("QPlainTextEdit { background: #ffffff; color: #111111; border: 1px solid #c5d0d5; "
                                   "padding: 8px; font-family: \"%1\"; font-size: 10pt; }")
                               .arg(TicketImpreso::familiaFuente()));

    auto *btnImprimir = new QPushButton("Imprimir…", this);
    btnImprimir->setDefault(true);
    auto *btnPdf = new QPushButton("Guardar PDF…", this);
    auto *btnConfig = new QPushButton("Configurar…", this);
    auto *btnCerrar = new QPushButton("Cerrar", this);
    connect(btnImprimir, &QPushButton::clicked, this, &VistaTicketDialog::imprimir);
    connect(btnPdf, &QPushButton::clicked, this, &VistaTicketDialog::guardarPdf);
    connect(btnConfig, &QPushButton::clicked, this, &VistaTicketDialog::configurar);
    connect(btnCerrar, &QPushButton::clicked, this, &QDialog::accept);

    auto *papel = new QHBoxLayout;   // el «papel» va centrado y con el ancho de las columnas
    papel->addStretch();
    papel->addWidget(m_papel);
    papel->addStretch();

    auto *botones = new QHBoxLayout;
    botones->addWidget(btnConfig);
    botones->addStretch();
    botones->addWidget(btnPdf);
    botones->addWidget(btnImprimir);
    botones->addWidget(btnCerrar);

    auto *capa = new QVBoxLayout(this);
    capa->addWidget(m_info);
    capa->addLayout(papel, 1);
    capa->addLayout(botones);

    recargar();
}

void VistaTicketDialog::recargar() {
    m_config = TicketImpreso::Configuracion::cargar();

    TicketImpreso::Datos datos;
    QString error;
    if (!TicketImpreso::cargar(m_idTicket, m_config, datos, error)) {
        m_lineas = {error};
    } else {
        datos.efectivoRecibido = m_efectivoRecibido;
        datos.cambio = m_cambio;
        m_lineas = TicketImpreso::renderizar(datos, m_config.columnas);
    }

    QFont f(TicketImpreso::familiaFuente(), 10);
    f.setStyleHint(QFont::Monospace);
    f.setFixedPitch(true);
    m_papel->setFont(f);
    m_papel->setPlainText(m_lineas.join('\n'));
    // Ancho = las columnas del ticket + márgenes, para que se vea exactamente como saldría.
    const QFontMetricsF fm(f);
    m_papel->setFixedWidth(qRound(fm.horizontalAdvance(QString(m_config.columnas, '0'))) + 40);

    QString info = QString("Papel de %1 mm · %2 columnas · Impresora: %3.")
                       .arg(m_config.anchoMm, 0, 'f', 0).arg(m_config.columnas)
                       .arg(m_config.impresora.isEmpty() ? QString("se elige al imprimir") : m_config.impresora);
    if (m_config.rfc.trimmed().isEmpty() || m_config.razonSocial.trimmed().isEmpty())
        info += " <b>Faltan datos fiscales</b> (razón social y RFC): usa «Configurar…».";
    m_info->setText(info);
    m_info->setTextFormat(Qt::RichText);
    resize(m_papel->width() + 120, 720);
}

void VistaTicketDialog::configurar() {
    ConfigurarTicketDialog dialogo(this);
    if (dialogo.exec() == QDialog::Accepted)
        recargar();
}

void VistaTicketDialog::guardarPdf() {
    const QString carpeta = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    const QString sugerido = QDir(carpeta).filePath(QString("ticket-%1.pdf").arg(m_idTicket, 6, 10, QChar('0')));
    QString ruta = QFileDialog::getSaveFileName(this, "Guardar ticket en PDF", sugerido, "PDF (*.pdf)");
    if (ruta.isEmpty())
        return;
    if (!ruta.endsWith(".pdf", Qt::CaseInsensitive))
        ruta += ".pdf";
    if (!TicketImpreso::exportarPdf(ruta, m_lineas, m_config)) {
        QMessageBox::warning(this, "Guardar PDF", "No se pudo guardar el PDF en:\n" + ruta);
        return;
    }
    QMessageBox::information(this, "Guardar PDF", "Ticket guardado en:\n" + ruta);
}

void VistaTicketDialog::imprimir() {
    if (QPrinterInfo::availablePrinterNames().isEmpty()) {
        QMessageBox::information(this, "Imprimir",
                                 "No hay impresoras instaladas en este equipo. Puedes guardar el ticket en PDF "
                                 "o instalar la impresora y volver a intentarlo.");
        return;
    }
    QPrinter impresora(QPrinterInfo::printerInfo(m_config.impresora), QPrinter::HighResolution);
    if (m_config.impresora.isEmpty() || QPrinterInfo::printerInfo(m_config.impresora).isNull()) {
        // Sin impresora configurada (o ya no existe): el usuario la elige.
        QPrintDialog elegir(&impresora, this);
        if (elegir.exec() != QDialog::Accepted)
            return;
    }
    if (!TicketImpreso::imprimir(impresora, m_lineas, m_config))
        QMessageBox::warning(this, "Imprimir", "No se pudo imprimir el ticket. Revisa que la impresora esté "
                                              "encendida y con papel, o guarda el ticket en PDF.");
}
