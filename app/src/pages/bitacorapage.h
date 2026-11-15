#pragma once

#include <QWidget>

class ModeloUtc;

namespace Ui {
class BitacoraPage;
}

// Consulta de la bitácora de cambios (precios y lotes): solo admin.
class BitacoraPage : public QWidget {
    Q_OBJECT
public:
    explicit BitacoraPage(QWidget *parent = nullptr);
    ~BitacoraPage();

public slots:
    void refrescar();

private:
    ModeloUtc *m_model;
    Ui::BitacoraPage *ui;
};
