#pragma once

#include <QWidget>

class QSqlQueryModel;

namespace Ui {
class EmpleadosPage;
}

class EmpleadosPage : public QWidget {
    Q_OBJECT
public:
    explicit EmpleadosPage(QWidget *parent = nullptr);
    ~EmpleadosPage();

public slots:
    void refrescar();

private slots:
    void nuevoEmpleado();
    void editarEmpleado();
    void eliminarEmpleado();

private:
    QSqlQueryModel *m_model;
    Ui::EmpleadosPage *ui;
};
