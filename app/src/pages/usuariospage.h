#pragma once

#include <QWidget>

class QSqlQueryModel;

namespace Ui {
class UsuariosPage;
}

class UsuariosPage : public QWidget {
    Q_OBJECT
public:
    explicit UsuariosPage(QWidget *parent = nullptr);
    ~UsuariosPage();

public slots:
    void refrescar();

private slots:
    void nuevoUsuario();
    void editarUsuario();
    void eliminarUsuario();

private:
    QSqlQueryModel *m_model;
    Ui::UsuariosPage *ui;
};
