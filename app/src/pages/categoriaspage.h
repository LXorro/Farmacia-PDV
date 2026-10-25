#pragma once

#include <QWidget>

class QSqlTableModel;

namespace Ui {
class CategoriasPage;
}

class CategoriasPage : public QWidget {
    Q_OBJECT
public:
    explicit CategoriasPage(QWidget *parent = nullptr);
    ~CategoriasPage();

public slots:
    void refrescar();

private slots:
    void nuevaCategoria();
    void editarCategoria();
    void eliminarCategoria();

private:
    QSqlTableModel *m_model;
    Ui::CategoriasPage *ui;
};
