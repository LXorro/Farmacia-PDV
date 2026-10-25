#pragma once

#include <QDialog>

namespace Ui {
class LoteDialog;
}

class LoteDialog : public QDialog {
    Q_OBJECT
public:
    explicit LoteDialog(QWidget *parent = nullptr);
    ~LoteDialog();

    void setLoteId(int id);

private slots:
    void guardar();

private:
    void cargarCombos();
    void cargarLote();

    int m_idLote = -1;
    Ui::LoteDialog *ui;
};
