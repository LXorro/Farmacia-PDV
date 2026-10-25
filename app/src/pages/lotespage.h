#pragma once

#include <QWidget>

class QSqlQueryModel;

namespace Ui {
class LotesPage;
}

class LotesPage : public QWidget {
    Q_OBJECT
public:
    explicit LotesPage(QWidget *parent = nullptr);
    ~LotesPage();

public slots:
    void refrescar();

private slots:
    void nuevoLote();
    void editarLote();

private:
    QSqlQueryModel *m_model;
    Ui::LotesPage *ui;
};
