#pragma once

#include <QWidget>

class ModeloUtc;

namespace Ui {
class PermisosPage;
}

// Permisos temporales («encargado», 1-3 días): los concede un admin y los empleados los usan en su sucursal.
class PermisosPage : public QWidget {
    Q_OBJECT
public:
    explicit PermisosPage(QWidget *parent = nullptr);
    ~PermisosPage();

public slots:
    void refrescar();

private slots:
    void nuevoPermiso();
    void revocarPermiso();

private:
    ModeloUtc *m_model;
    Ui::PermisosPage *ui;
};
