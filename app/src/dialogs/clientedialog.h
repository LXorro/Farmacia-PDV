#pragma once

#include <QDialog>

namespace Ui {
class ClienteDialog;
}

class ClienteDialog : public QDialog {
    Q_OBJECT
public:
    explicit ClienteDialog(QWidget *parent = nullptr);
    ~ClienteDialog();

    void setClienteId(int id);

private slots:
    void guardar();

private:
    void cargarCombos();
    void cargarCliente();

    int m_idCliente = -1;
    Ui::ClienteDialog *ui;
};
