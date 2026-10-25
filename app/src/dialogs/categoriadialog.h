#pragma once

#include <QDialog>

namespace Ui {
class CategoriaDialog;
}

class CategoriaDialog : public QDialog {
    Q_OBJECT
public:
    explicit CategoriaDialog(QWidget *parent = nullptr);
    ~CategoriaDialog();

    void setCategoriaId(int id);

private slots:
    void guardar();

private:
    void cargarCategoria();

    int m_idCategoria = -1;
    Ui::CategoriaDialog *ui;
};
