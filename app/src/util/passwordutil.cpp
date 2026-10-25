#include "passwordutil.h"

#include <QCryptographicHash>
#include <QRandomGenerator>

namespace PasswordUtil {

QString generarSal() {
    const QString alfabeto = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    QString sal;
    sal.reserve(32);
    for (int i = 0; i < 32; ++i)
        sal.append(alfabeto.at(QRandomGenerator::global()->bounded(alfabeto.length())));
    return sal;
}

QString hashear(const QString &contrasena, const QString &sal) {
    QByteArray datos = (sal + contrasena).toUtf8();
    return QCryptographicHash::hash(datos, QCryptographicHash::Sha256).toHex();
}

bool verificar(const QString &contrasena, const QString &sal, const QString &hashEsperado) {
    return hashear(contrasena, sal) == hashEsperado;
}

}
