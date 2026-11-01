#pragma once

#include <QMainWindow>

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
};
