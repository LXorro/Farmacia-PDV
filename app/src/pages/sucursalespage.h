#pragma once

#include <QWidget>

class QSqlTableModel;

namespace Ui {
class SucursalesPage;
}

class SucursalesPage : public QWidget {
    Q_OBJECT
public:
    explicit SucursalesPage(QWidget *parent = nullptr);
    ~SucursalesPage();

public slots:
    void refrescar();

private slots:
    void nuevaSucursal();
    void editarSucursal();
    void eliminarSucursal();

private:
    QSqlTableModel *m_model;
    Ui::SucursalesPage *ui;
};
