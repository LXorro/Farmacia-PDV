#include "clavecentral.h"

#include <QCheckBox>
#include <QDateTime>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QSettings>
#include <QSysInfo>
#include <QVBoxLayout>

namespace {
constexpr int kHorasVigencia = 24;

QByteArray clave() {
    return QSysInfo::machineUniqueId() + "|pdv-central";
}

QByteArray xorear(const QByteArray &datos) {
    const QByteArray k = clave();
    QByteArray salida = datos;
    for (int i = 0; i < salida.size(); ++i)
        salida[i] = salida[i] ^ k[i % k.size()];
    return salida;
}
}

namespace ClaveCentral {

QString recordada() {
    QSettings ajustes;
    const QDateTime hasta = QDateTime::fromSecsSinceEpoch(ajustes.value("central/clave_hasta", 0).toLongLong());
    const QByteArray guardada = ajustes.value("central/clave_recordada").toByteArray();
    if (guardada.isEmpty() || QDateTime::currentDateTime() >= hasta) {
        if (!guardada.isEmpty())
            olvidar();
        return {};
    }
    return QString::fromUtf8(xorear(QByteArray::fromBase64(guardada)));
}

void recordar(const QString &contrasena) {
    QSettings ajustes;
    ajustes.setValue("central/clave_recordada", xorear(contrasena.toUtf8()).toBase64());
    ajustes.setValue("central/clave_hasta", QDateTime::currentDateTime().addSecs(kHorasVigencia * 3600).toSecsSinceEpoch());
}

void olvidar() {
    QSettings ajustes;
    ajustes.remove("central/clave_recordada");
    ajustes.remove("central/clave_hasta");
}

bool pedir(QWidget *parent, const QString &titulo, const QString &texto, QString &contrasena, bool &recordarClave) {
    QDialog dialogo(parent);
    dialogo.setWindowTitle(titulo);
    auto *capa = new QVBoxLayout(&dialogo);
    auto *etiqueta = new QLabel(texto, &dialogo);
    etiqueta->setWordWrap(true);
    capa->addWidget(etiqueta);
    auto *campo = new QLineEdit(&dialogo);
    campo->setEchoMode(QLineEdit::Password);
    capa->addWidget(campo);
    auto *casilla = new QCheckBox("Recordar 24 h en este equipo", &dialogo);
    capa->addWidget(casilla);
    auto *botones = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialogo);
    capa->addWidget(botones);
    QObject::connect(botones, &QDialogButtonBox::accepted, &dialogo, &QDialog::accept);
    QObject::connect(botones, &QDialogButtonBox::rejected, &dialogo, &QDialog::reject);
    dialogo.setMinimumWidth(420);

    if (dialogo.exec() != QDialog::Accepted)
        return false;
    contrasena = campo->text();
    recordarClave = casilla->isChecked();
    return true;
}

}
