#pragma once

#include <QWidget>

class QSqlQueryModel;

namespace Ui {
class AsignacionesPage;
}

class AsignacionesPage : public QWidget {
    Q_OBJECT
public:
    explicit AsignacionesPage(QWidget *parent = nullptr);
    ~AsignacionesPage();

public slots:
    void refrescar();

private slots:
    void nuevaAsignacion();
    void revocarAsignacion();

private:
    QSqlQueryModel *m_model;
    Ui::AsignacionesPage *ui;
};
