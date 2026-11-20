#pragma once

#include <QDialog>

class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QLineEdit;
class QPlainTextEdit;
class QSpinBox;

// Datos del ticket e impresora de ESTE equipo (se guardan con QSettings, no en la base).
class ConfigurarTicketDialog : public QDialog {
    Q_OBJECT
public:
    explicit ConfigurarTicketDialog(QWidget *parent = nullptr);

private slots:
    void aceptar();

private:
    QComboBox *m_impresora;
    QDoubleSpinBox *m_ancho;
    QSpinBox *m_columnas;
    QLineEdit *m_nombre;
    QLineEdit *m_razonSocial;
    QLineEdit *m_rfc;
    QLineEdit *m_domicilio;
    QLineEdit *m_telefono;
    QPlainTextEdit *m_pie;
    QCheckBox *m_medicamentoCero;
};
