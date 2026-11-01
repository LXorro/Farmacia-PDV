#pragma once

#include <QDialog>

namespace Ui {
class AsignacionDialog;
}

class AsignacionDialog : public QDialog {
    Q_OBJECT
public:
    explicit AsignacionDialog(QWidget *parent = nullptr);
    ~AsignacionDialog();

private slots:
    void guardar();

private:
    void cargarCombos();

    Ui::AsignacionDialog *ui;
};
