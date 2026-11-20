#pragma once

#include <QDialog>
#include <QString>
#include <QVector>

class QDoubleSpinBox;
class QLabel;
class QLineEdit;

// Una parte de un cobro: un método y el monto que cubre.
struct PagoParte {
    QString metodo;        // efectivo | tarjeta | transferencia
    double monto = 0;      // lo aplicado al ticket (en efectivo, sin el cambio)
    QString referencia;    // autorización (tarjeta) o referencia (transferencia)
    QString ultimos;       // últimos 4 dígitos de la tarjeta
};

// Cobro mixto de una venta de contado: parte en tarjeta, parte en transferencia y el resto en efectivo.
// Se capturan los montos de tarjeta y transferencia; el efectivo es lo que falta, y se pide cuánto se
// recibió para calcular el cambio. La app no cobra con tarjeta: la terminal lo hace y aquí solo se
// registra su autorización.
class PagoMixtoDialog : public QDialog {
    Q_OBJECT
public:
    explicit PagoMixtoDialog(double total, QWidget *parent = nullptr);

    QVector<PagoParte> partes() const;   // solo las que tienen monto; la suma es el total
    double efectivoRecibido() const;     // 0 si no hubo parte en efectivo
    double cambio() const;

private slots:
    void actualizar();
    void aceptar();

private:
    qint64 efectivoAplicadoCent() const;

    qint64 m_totalCent;
    QDoubleSpinBox *m_tarjeta;
    QDoubleSpinBox *m_transferencia;
    QDoubleSpinBox *m_recibido;
    QLineEdit *m_autorizacion;
    QLineEdit *m_ultimos;
    QLineEdit *m_referencia;
    QLabel *m_resumen;
    bool m_recibidoManual = false;
    bool m_programatico = false;
};
