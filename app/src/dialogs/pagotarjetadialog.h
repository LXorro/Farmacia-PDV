#pragma once

#include <QDialog>

class QLineEdit;

// Datos del comprobante de la terminal al cobrar con tarjeta. La app no cobra: el cobro lo hace la
// terminal y aquí solo se registra su autorización (normalmente 6 caracteres) y, opcionalmente, los
// últimos 4 dígitos. Sirve para conciliar el cobro con el cierre de la terminal y para reembolsar después.
class PagoTarjetaDialog : public QDialog {
    Q_OBJECT
public:
    explicit PagoTarjetaDialog(double monto, QWidget *parent = nullptr);

    QString autorizacion() const;
    QString ultimosDigitos() const;   // vacío si no se capturaron

    // Validación común (autorización de 4 a 20 caracteres alfanuméricos): también la usan los folios de reembolso.
    static bool folioValido(const QString &texto);

private slots:
    void aceptar();

private:
    QLineEdit *m_autorizacion;
    QLineEdit *m_ultimos;
};
