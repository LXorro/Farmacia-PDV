#pragma once

#include <QWidget>

namespace Ui {
class ReporteDiarioPage;
}

// Reporte diario de movimientos con exportación a PDF. Un empleado ve su sucursal; un admin sobre el
// central puede elegir una sucursal o ver todas.
class ReporteDiarioPage : public QWidget {
    Q_OBJECT
public:
    explicit ReporteDiarioPage(QWidget *parent = nullptr);
    ~ReporteDiarioPage();

public slots:
    void refrescar();

private slots:
    void exportarPdf();

private:
    Ui::ReporteDiarioPage *ui;
};
