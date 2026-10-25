#pragma once

#include <QDialog>

namespace Ui {
class TicketDetalleDialog;
}

class TicketDetalleDialog : public QDialog {
    Q_OBJECT
public:
    explicit TicketDetalleDialog(QWidget *parent = nullptr);
    ~TicketDetalleDialog();

    void setTicketId(int id);

private slots:
    void registrarAbono();
    void registrarDevolucion();

private:
    void cargarDatos();

    int m_idTicket = -1;
    int m_idSucursal = -1;
    Ui::TicketDetalleDialog *ui;
};
