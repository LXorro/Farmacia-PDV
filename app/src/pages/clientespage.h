#pragma once

#include <QWidget>

class QSqlQueryModel;

namespace Ui {
class ClientesPage;
}

class ClientesPage : public QWidget {
    Q_OBJECT
public:
    explicit ClientesPage(QWidget *parent = nullptr);
    ~ClientesPage();

public slots:
    void refrescar();

private slots:
    void nuevoCliente();
    void editarCliente();
    void eliminarCliente();

private:
    QSqlQueryModel *m_model;
    Ui::ClientesPage *ui;
};
