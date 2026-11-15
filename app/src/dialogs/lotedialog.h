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
    QString estadoDerivado() const;
    QString descripcionLote() const;
    void actualizarEstado();

    int m_idLote = -1;
    QString m_antes;   // cómo estaba el lote al abrir (para la bitácora)
    Ui::LoteDialog *ui;
};
