#include "pagotarjetadialog.h"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QRegularExpression>
#include <QVBoxLayout>

bool PagoTarjetaDialog::folioValido(const QString &texto) {
    static const QRegularExpression patron("^[A-Za-z0-9]{4,20}$");
    return patron.match(texto).hasMatch();
}

PagoTarjetaDialog::PagoTarjetaDialog(double monto, QWidget *parent) : QDialog(parent) {
    setWindowTitle("Pago con tarjeta");

    auto *aviso = new QLabel(
        QString("Cobra <b>$%1</b> en la terminal y captura los datos del comprobante. "
                "La aplicación no procesa el cobro, solo lo registra.").arg(monto, 0, 'f', 2), this);
    aviso->setWordWrap(true);

    m_autorizacion = new QLineEdit(this);
    m_autorizacion->setMaxLength(20);
    m_autorizacion->setPlaceholderText("Ej. 123456 (código de aprobación)");
    m_ultimos = new QLineEdit(this);
    m_ultimos->setMaxLength(4);
    m_ultimos->setPlaceholderText("Opcional");

    auto *formulario = new QFormLayout;
    formulario->addRow("Autorización:", m_autorizacion);
    formulario->addRow("Últimos 4 dígitos:", m_ultimos);

    auto *botones = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(botones, &QDialogButtonBox::accepted, this, &PagoTarjetaDialog::aceptar);
    connect(botones, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *capa = new QVBoxLayout(this);
    capa->addWidget(aviso);
    capa->addLayout(formulario);
    capa->addWidget(botones);
    m_autorizacion->setFocus();
}

QString PagoTarjetaDialog::autorizacion() const {
    return m_autorizacion->text().trimmed().toUpper();
}

QString PagoTarjetaDialog::ultimosDigitos() const {
    return m_ultimos->text().trimmed();
}

void PagoTarjetaDialog::aceptar() {
    if (!folioValido(m_autorizacion->text().trimmed())) {
        QMessageBox::warning(this, "Autorización",
                             "Escribe la autorización del comprobante: de 4 a 20 letras o números, sin espacios.");
        return;
    }
    static const QRegularExpression cuatroDigitos("^\\d{4}$");
    if (!ultimosDigitos().isEmpty() && !cuatroDigitos.match(ultimosDigitos()).hasMatch()) {
        QMessageBox::warning(this, "Últimos dígitos", "Deben ser exactamente 4 números (o déjalos vacíos).");
        return;
    }
    accept();
}
