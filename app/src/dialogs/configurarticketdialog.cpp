#include "configurarticketdialog.h"
#include "../util/ticketimpreso.h"
#include "../util/validacion.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPrinterInfo>
#include <QSpinBox>
#include <QVBoxLayout>

using TicketImpreso::Configuracion;

ConfigurarTicketDialog::ConfigurarTicketDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Configurar ticket e impresora");
    const Configuracion c = Configuracion::cargar();

    m_impresora = new QComboBox(this);
    m_impresora->addItem("(elegir al imprimir)", QString());
    for (const QString &nombre : QPrinterInfo::availablePrinterNames())
        m_impresora->addItem(nombre, nombre);
    const int posicion = m_impresora->findData(c.impresora);
    m_impresora->setCurrentIndex(posicion >= 0 ? posicion : 0);

    m_ancho = new QDoubleSpinBox(this);
    m_ancho->setRange(40.0, 120.0);
    m_ancho->setDecimals(0);
    m_ancho->setSuffix(" mm");
    m_ancho->setValue(c.anchoMm);
    m_columnas = new QSpinBox(this);
    m_columnas->setRange(32, 64);
    m_columnas->setValue(c.columnas);
    m_columnas->setToolTip("80 mm: 42 es lo recomendado (la fuente admite hasta 48). 58 mm: 32.");

    m_nombre = new QLineEdit(c.nombreComercial, this);
    m_nombre->setPlaceholderText("Ej. Farmacia San Rafael");
    m_razonSocial = new QLineEdit(c.razonSocial, this);
    m_razonSocial->setPlaceholderText("Nombre o razón social del contribuyente");
    m_rfc = new QLineEdit(c.rfc, this);
    m_rfc->setMaxLength(13);
    m_rfc->setPlaceholderText("12 o 13 caracteres");
    m_domicilio = new QLineEdit(c.domicilio, this);
    m_domicilio->setPlaceholderText("Vacío: se usa el de la sucursal registrado en el central");
    m_telefono = new QLineEdit(c.telefono, this);
    m_telefono->setPlaceholderText("Vacío: se usa el de la sucursal");
    m_pie = new QPlainTextEdit(c.pie, this);
    m_pie->setFixedHeight(70);

    m_medicamentoCero = new QCheckBox("Medicamentos a tasa 0 % de IVA; el resto a 16 %", this);
    m_medicamentoCero->setChecked(c.medicamentoIvaCero);

    auto *nota = new QLabel(
        "El IVA se desglosa por la categoría del producto (la categoría «medicamento» a tasa 0 %). Es una "
        "aproximación: la tasa 0 % aplica solo a medicina de patente con registro sanitario. "
        "<b>Confírmalo con tu contador.</b> Estos datos se guardan solo en este equipo.", this);
    nota->setWordWrap(true);

    auto *formulario = new QFormLayout;
    formulario->addRow("Impresora:", m_impresora);
    formulario->addRow("Ancho del papel:", m_ancho);
    formulario->addRow("Columnas por línea:", m_columnas);
    formulario->addRow("Nombre comercial:", m_nombre);
    formulario->addRow("Razón social:", m_razonSocial);
    formulario->addRow("RFC:", m_rfc);
    formulario->addRow("Domicilio:", m_domicilio);
    formulario->addRow("Teléfono:", m_telefono);
    formulario->addRow("Pie del ticket:", m_pie);
    formulario->addRow("IVA:", m_medicamentoCero);

    auto *botones = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(botones, &QDialogButtonBox::accepted, this, &ConfigurarTicketDialog::aceptar);
    connect(botones, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *capa = new QVBoxLayout(this);
    capa->addLayout(formulario);
    capa->addWidget(nota);
    capa->addWidget(botones);
    resize(560, sizeHint().height());
}

void ConfigurarTicketDialog::aceptar() {
    {
        const QString error = Validacion::textoComercial(m_nombre->text(), "El nombre comercial", false, 100);
        const QString errorRazon = Validacion::textoComercial(m_razonSocial->text(), "La razón social", false, 100);
        const QString errorDom = Validacion::direccion(m_domicilio->text());
        const QString errorTel = Validacion::telefono(m_telefono->text());
        const QString errorPie = Validacion::textoLibre(m_pie->toPlainText(), "El pie del ticket", 300);
        for (const QString &e : {error, errorRazon, errorDom, errorTel, errorPie}) {
            if (!e.isEmpty()) {
                QMessageBox::warning(this, "Validación", e);
                return;
            }
        }
    }
    if (!Configuracion::rfcValido(m_rfc->text())) {
        QMessageBox::warning(this, "RFC", "El RFC debe tener 12 caracteres (persona moral) o 13 (persona física), "
                                          "por ejemplo ABC123456AB1. Déjalo vacío si aún no lo tienes.");
        return;
    }
    Configuracion c;
    c.impresora = m_impresora->currentData().toString();
    c.anchoMm = m_ancho->value();
    c.columnas = m_columnas->value();
    c.nombreComercial = m_nombre->text().trimmed();
    c.razonSocial = m_razonSocial->text().trimmed();
    c.rfc = m_rfc->text().trimmed();
    c.domicilio = m_domicilio->text().trimmed();
    c.telefono = m_telefono->text().trimmed();
    c.pie = m_pie->toPlainText();
    c.medicamentoIvaCero = m_medicamentoCero->isChecked();
    c.guardar();
    accept();
}
