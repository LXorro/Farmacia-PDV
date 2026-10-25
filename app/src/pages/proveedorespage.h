#pragma once

#include <QWidget>

class QSqlTableModel;

namespace Ui {
class ProveedoresPage;
}

class ProveedoresPage : public QWidget {
    Q_OBJECT
public:
    explicit ProveedoresPage(QWidget *parent = nullptr);
    ~ProveedoresPage();

public slots:
    void refrescar();

private slots:
    void nuevoProveedor();
    void editarProveedor();
    void eliminarProveedor();

private:
    QSqlTableModel *m_model;
    Ui::ProveedoresPage *ui;
};
