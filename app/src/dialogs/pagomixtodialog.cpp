#include "pagomixtodialog.h"
#include "pagotarjetadialog.h"

#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QRegularExpression>
#include <QVBoxLayout>
#include <QtMath>

namespace {
qint64 centavos(double x) { return qRound64(x * 100.0); }
QString dinero(qint64 cent) { return "$" + QString::number(cent / 100.0, 'f', 2); }

QDoubleSpinBox *campoMonto(double maximo, QWidget *padre) {
    auto *c = new QDoubleSpinBox(padre);
    c->setDecimals(2);
    c->setRange(0.0, maximo);
    c->setPrefix("$ ");
    c->setSingleStep(1.0);
    c->setGroupSeparatorShown(false);
    return c;
}
}   // namespace

PagoMixtoDialog::PagoMixtoDialog(double total, QWidget *parent)
    : QDialog(parent), m_totalCent(centavos(total)) {
    setWindowTitle("Pago mixto");

    auto *aviso = new QLabel(
        QString("Total a cobrar: <b>%1</b><br>Captura cuánto se paga con tarjeta y con transferencia; el resto "
                "se cobra en efectivo. La aplicación no procesa la tarjeta: cóbrala en la terminal y registra "
                "aquí su autorización.").arg(dinero(m_totalCent)), this);
    aviso->setWordWrap(true);

    const double maximo = m_totalCent / 100.0;
    m_tarjeta = campoMonto(maximo, this);
    m_transferencia = campoMonto(maximo, this);
    m_recibido = campoMonto(9999999.99, this);

    m_autorizacion = new QLineEdit(this);
    m_autorizacion->setMaxLength(20);
    m_autorizacion->setPlaceholderText("Código de aprobación de la terminal");
    m_ultimos = new QLineEdit(this);
    m_ultimos->setMaxLength(4);
    m_ultimos->setPlaceholderText("Opcional");
    m_referencia = new QLineEdit(this);
    m_referencia->setMaxLength(100);
    m_referencia->setPlaceholderText("Opcional (clave de rastreo o folio)");

    m_resumen = new QLabel(this);
    m_resumen->setWordWrap(true);
    m_resumen->setTextFormat(Qt::RichText);

    auto *formulario = new QFormLayout;
    formulario->addRow("Tarjeta — monto:", m_tarjeta);
    formulario->addRow("Autorización:", m_autorizacion);
    formulario->addRow("Últimos 4 dígitos:", m_ultimos);
    formulario->addRow("Transferencia — monto:", m_transferencia);
    formulario->addRow("Referencia:", m_referencia);
    formulario->addRow("Efectivo recibido:", m_recibido);

    auto *botones = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(botones, &QDialogButtonBox::accepted, this, &PagoMixtoDialog::aceptar);
    connect(botones, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *capa = new QVBoxLayout(this);
    capa->addWidget(aviso);
    capa->addLayout(formulario);
    capa->addWidget(m_resumen);
    capa->addWidget(botones);

    connect(m_tarjeta, &QDoubleSpinBox::valueChanged, this, &PagoMixtoDialog::actualizar);
    connect(m_transferencia, &QDoubleSpinBox::valueChanged, this, &PagoMixtoDialog::actualizar);
    connect(m_recibido, &QDoubleSpinBox::valueChanged, this, [this] {
        if (!m_programatico)
            m_recibidoManual = true;   // el usuario escribió su propio «recibido»: ya no se autocompleta
        actualizar();
    });
    actualizar();
    m_tarjeta->setFocus();
}

qint64 PagoMixtoDialog::efectivoAplicadoCent() const {
    return m_totalCent - centavos(m_tarjeta->value()) - centavos(m_transferencia->value());
}

void PagoMixtoDialog::actualizar() {
    const qint64 tarjeta = centavos(m_tarjeta->value());
    const qint64 transferencia = centavos(m_transferencia->value());
    const qint64 aplicado = efectivoAplicadoCent();
    m_autorizacion->setEnabled(tarjeta > 0);
    m_ultimos->setEnabled(tarjeta > 0);
    m_referencia->setEnabled(transferencia > 0);

    if (aplicado < 0) {
        m_resumen->setText("<span style='color:#b00020'>Tarjeta y transferencia suman más que el total.</span>");
        return;
    }
    if (!m_recibidoManual) {
        m_programatico = true;
        m_recibido->setValue(aplicado / 100.0);
        m_programatico = false;
    }
    const qint64 recibido = centavos(m_recibido->value());
    QString texto = QString("Efectivo a cobrar: <b>%1</b>").arg(dinero(aplicado));
    if (aplicado > 0 && recibido < aplicado)
        texto += QString(" — <span style='color:#b00020'>faltan %1</span>").arg(dinero(aplicado - recibido));
    else if (aplicado > 0)
        texto += QString(" — cambio: <b>%1</b>").arg(dinero(recibido - aplicado));
    m_resumen->setText(texto);
}

void PagoMixtoDialog::aceptar() {
    const qint64 tarjeta = centavos(m_tarjeta->value());
    const qint64 transferencia = centavos(m_transferencia->value());
    const qint64 aplicado = efectivoAplicadoCent();

    if (aplicado < 0) {
        QMessageBox::warning(this, "Pago mixto", "Tarjeta y transferencia suman más que el total.");
        return;
    }
    const int metodos = (tarjeta > 0) + (transferencia > 0) + (aplicado > 0);
    if (metodos < 2) {
        QMessageBox::warning(this, "Pago mixto",
                             "Un pago mixto combina al menos dos métodos. Para un solo método, "
                             "elígelo en la pantalla de cobro.");
        return;
    }
    if (tarjeta > 0 && !PagoTarjetaDialog::folioValido(m_autorizacion->text().trimmed())) {
        QMessageBox::warning(this, "Autorización",
                             "Escribe la autorización del comprobante: de 4 a 20 letras o números, sin espacios.");
        return;
    }
    static const QRegularExpression cuatroDigitos("^\\d{4}$");
    if (tarjeta > 0 && !m_ultimos->text().trimmed().isEmpty() && !cuatroDigitos.match(m_ultimos->text().trimmed()).hasMatch()) {
        QMessageBox::warning(this, "Últimos dígitos", "Deben ser exactamente 4 números (o déjalos vacíos).");
        return;
    }
    if (aplicado > 0 && centavos(m_recibido->value()) < aplicado) {
        QMessageBox::warning(this, "Efectivo",
                             QString("El efectivo recibido no alcanza: faltan %1.")
                                 .arg(dinero(aplicado - centavos(m_recibido->value()))));
        return;
    }
    accept();
}

QVector<PagoParte> PagoMixtoDialog::partes() const {
    QVector<PagoParte> lista;
    const qint64 aplicado = efectivoAplicadoCent();
    if (aplicado > 0)
        lista.append({"efectivo", aplicado / 100.0, QString(), QString()});
    if (centavos(m_tarjeta->value()) > 0)
        lista.append({"tarjeta", centavos(m_tarjeta->value()) / 100.0, m_autorizacion->text().trimmed().toUpper(),
                      m_ultimos->text().trimmed()});
    if (centavos(m_transferencia->value()) > 0)
        lista.append({"transferencia", centavos(m_transferencia->value()) / 100.0, m_referencia->text().trimmed(), QString()});
    return lista;
}

double PagoMixtoDialog::efectivoRecibido() const {
    return efectivoAplicadoCent() > 0 ? m_recibido->value() : 0.0;
}

double PagoMixtoDialog::cambio() const {
    const qint64 aplicado = efectivoAplicadoCent();
    return aplicado > 0 ? qMax<qint64>(0, centavos(m_recibido->value()) - aplicado) / 100.0 : 0.0;
}
