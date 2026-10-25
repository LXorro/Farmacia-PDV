#pragma once

#include <QWidget>

class QSqlQueryModel;

namespace Ui {
class ProductosPage;
}

class ProductosPage : public QWidget {
    Q_OBJECT
public:
    explicit ProductosPage(QWidget *parent = nullptr);
    ~ProductosPage();

public slots:
    void refrescar();

private slots:
    void nuevoProducto();
    void editarProducto();
    void eliminarProducto();

private:
    QSqlQueryModel *m_model;
    Ui::ProductosPage *ui;
};
