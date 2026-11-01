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
    // En una sucursal las cuentas se administran en el central: pide su contraseña si hace falta.
    bool asegurarConexion();

    QSqlQueryModel *m_model;
    Ui::UsuariosPage *ui;
};
