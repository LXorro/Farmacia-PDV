#pragma once

#include <QWidget>

class QSqlQueryModel;

namespace Ui {
class TicketsPage;
}

class TicketsPage : public QWidget {
    Q_OBJECT
public:
    explicit TicketsPage(QWidget *parent = nullptr);
    ~TicketsPage();

public slots:
    void refrescar();

private slots:
    void verDetalle();

private:
    QSqlQueryModel *m_model;
    QSqlQueryModel *m_modelDevoluciones;
    Ui::TicketsPage *ui;
};
