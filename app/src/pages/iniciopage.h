#pragma once

#include <QWidget>

namespace Ui {
class InicioPage;
}

class InicioPage : public QWidget {
    Q_OBJECT
public:
    explicit InicioPage(QWidget *parent = nullptr);
    ~InicioPage();

public slots:
    void refrescar();

signals:
    void abrirPagina(const QString &nombrePagina);

private:
    void cargarEstadisticas();

    Ui::InicioPage *ui;
};
