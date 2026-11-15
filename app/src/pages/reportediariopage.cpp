#include "reportediariopage.h"
#include "ui_reportediariopage.h"
#include "../database/databasemanager.h"
#include "../database/sesionactual.h"
#include "../util/iconos.h"
#include "../util/reportediario.h"

#include <QDate>
#include <QFileDialog>
#include <QMarginsF>
#include <QMessageBox>
#include <QPageSize>
#include <QPdfWriter>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QTextDocument>

ReporteDiarioPage::ReporteDiarioPage(QWidget *parent) : QWidget(parent), ui(new Ui::ReporteDiarioPage) {
    ui->setupUi(this);

    ui->dateFecha->setDisplayFormat("dd/MM/yyyy");
    ui->dateFecha->setMaximumDate(QDate::currentDate());
    ui->dateFecha->setDate(QDate::currentDate());

    const SesionActual &sesion = SesionActual::instance();
    if (sesion.veTodasLasSucursales()) {
        ui->comboSucursal->addItem("Todas las sucursales", -1);
        QSqlQuery sucursales("SELECT id_sucursal, nombre FROM Sucursales WHERE rol = 'sucursal' AND estado = 'activa' ORDER BY nombre");
        while (sucursales.next())
            ui->comboSucursal->addItem(sucursales.value(1).toString(), sucursales.value(0));
    } else {
        // Un equipo de sucursal solo reporta la suya.
        ui->comboSucursal->addItem(sesion.nombreSucursal(), sesion.idSucursal());
        ui->comboSucursal->setEnabled(false);
    }

    ui->btnRefrescar->setIcon(Iconos::refrescar());
    ui->btnPdf->setIcon(Iconos::tickets());
    connect(ui->btnRefrescar, &QPushButton::clicked, this, &ReporteDiarioPage::refrescar);
    connect(ui->btnPdf, &QPushButton::clicked, this, &ReporteDiarioPage::exportarPdf);
    connect(ui->dateFecha, &QDateEdit::dateChanged, this, &ReporteDiarioPage::refrescar);
    connect(ui->comboSucursal, qOverload<int>(&QComboBox::currentIndexChanged), this, &ReporteDiarioPage::refrescar);

    refrescar();
}

ReporteDiarioPage::~ReporteDiarioPage() {
    delete ui;
}

void ReporteDiarioPage::refrescar() {
    ui->textoReporte->setHtml(ReporteDiario::generarHtml(ui->dateFecha->date(), ui->comboSucursal->currentData().toInt(),
                                                         ui->comboSucursal->currentText()));
}

void ReporteDiarioPage::exportarPdf() {
    const QString sucursal = ui->comboSucursal->currentText().toLower().replace(' ', '-');
    const QString sugerido = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) +
                             QString("/reporte-%1-%2.pdf").arg(ui->dateFecha->date().toString("yyyy-MM-dd"), sucursal);
    QString ruta = QFileDialog::getSaveFileName(this, "Exportar reporte a PDF", sugerido, "PDF (*.pdf)");
    if (ruta.isEmpty())
        return;
    if (!ruta.endsWith(".pdf", Qt::CaseInsensitive))
        ruta += ".pdf";

    // El mismo HTML que se ve en pantalla, impreso sobre hojas A4.
    QPdfWriter escritor(ruta);
    escritor.setPageSize(QPageSize(QPageSize::A4));
    escritor.setPageMargins(QMarginsF(15, 15, 15, 15), QPageLayout::Millimeter);
    escritor.setTitle("Reporte diario de movimientos");
    escritor.setCreator("Farmacia PDV");

    QTextDocument documento;
    documento.setHtml(ReporteDiario::generarHtml(ui->dateFecha->date(), ui->comboSucursal->currentData().toInt(),
                                                 ui->comboSucursal->currentText()));
    documento.print(&escritor);

    QMessageBox::information(this, "Reporte exportado", "Se guardó el PDF en:\n" + ruta);
}
