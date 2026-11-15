#pragma once

#include <QHash>
#include <QMainWindow>
#include <QString>

namespace Ui {
class MainWindow;
}

class QLabel;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void irAPagina(const QString &nombrePagina);
    void actualizarSync();

private:
    Ui::MainWindow *ui;
    QLabel *m_labelSync = nullptr;
    QHash<QString, QString> m_ayudaPagina;   // texto de ayuda bajo el título de cada pantalla
};
