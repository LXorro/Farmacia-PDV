#pragma once
// Utilidades compartidas por los tests: entorno aislado, conexión a la BD de pruebas,
// sesiones y un «cazador de modales» para que QMessageBox/QDialog no bloqueen la ejecución.

#include "database/databasemanager.h"
#include "database/sesionactual.h"
#include "util/passwordutil.h"

#include <QAbstractButton>
#include <QApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QTimer>
#include <QtTest>
#include <functional>

namespace T {

inline int puertoPrueba() {
    const int p = qEnvironmentVariableIntValue("FARMACIA_TEST_PORT");
    return p > 0 ? p : 33307;
}

// Se llama ANTES de crear QApplication.
inline void prepararEntorno() {
    qputenv("QT_QPA_PLATFORM", "offscreen");
    // QSettings (credenciales, central, sucursal del nodo) en una carpeta temporal: nunca toca los ajustes reales.
    static QTemporaryDir ajustes;
    qputenv("XDG_CONFIG_HOME", ajustes.path().toUtf8());
    qputenv("HOME", ajustes.path().toUtf8());
}

// Conecta la app a la BD de pruebas. Se niega a correr contra el puerto de producción.
inline bool conectarBd(QString *error = nullptr) {
    const int puerto = puertoPrueba();
    if (puerto == 3306) {
        if (error) *error = "Se rechaza el puerto 3306: los tests solo corren contra una BD de pruebas.";
        return false;
    }
    DatabaseManager::instance().guardarCredenciales("127.0.0.1", puerto, "farmacia_app", "farmacia_app_pw");
    const auto r = DatabaseManager::instance().connect();
    if (r != DatabaseManager::Resultado::Ok) {
        if (error) *error = DatabaseManager::instance().lastError();
        return false;
    }
    return true;
}

inline QVariant escalar(const QString &sql, const QVariantList &args = {}) {
    QSqlQuery q;
    q.prepare(sql);
    for (const auto &a : args) q.addBindValue(a);
    if (!q.exec() || !q.next()) return QVariant();
    return q.value(0);
}
inline int contar(const QString &tabla, const QString &donde = "1=1", const QVariantList &args = {}) {
    return escalar("SELECT COUNT(*) FROM " + tabla + " WHERE " + donde, args).toInt();
}

// Crea (si faltan) un admin y un empleado con contraseña conocida.
inline void sembrarUsuarios() {
    const auto crear = [](const QString &usuario, const QString &clave, const QString &rol, QVariant idEmpleado) {
        const QString sal = PasswordUtil::generarSal();
        QSqlQuery q;
        q.prepare("INSERT IGNORE INTO Usuarios (id_empleado, nombre_usuario, contrasena_hash, sal, rol, estado) "
                  "VALUES (:e, :u, :h, :s, :r, 'activo')");
        q.bindValue(":e", idEmpleado);
        q.bindValue(":u", usuario);
        q.bindValue(":h", PasswordUtil::hashear(clave, sal));
        q.bindValue(":s", sal);
        q.bindValue(":r", rol);
        q.exec();
    };
    crear("admin", "Admin123", "admin", 1);        // Ana García, sucursal 1 (igual que un login real: toda cuenta tiene empleado)
    crear("luis", "Luis12345", "empleado", 2);   // Luis Martínez, Sucursal Norte (id 2)
}

inline void iniciarSesionAdmin() {
    SesionActual::instance().iniciar(escalar("SELECT id_usuario FROM Usuarios WHERE nombre_usuario='admin'").toInt(),
                                     "admin", "admin", 1, 1, "Central");
}
inline void iniciarSesionEmpleado() {
    SesionActual::instance().iniciar(escalar("SELECT id_usuario FROM Usuarios WHERE nombre_usuario='luis'").toInt(),
                                     "luis", "empleado", 2, 2, "Sucursal Norte");
}

inline void pulsarAceptar(QDialog *d) {
    if (auto *bb = d->findChild<QDialogButtonBox *>()) {
        for (auto tipo : {QDialogButtonBox::Ok, QDialogButtonBox::Save, QDialogButtonBox::Apply})
            if (auto *b = bb->button(tipo)) { b->click(); return; }
        for (auto *b : bb->buttons())
            if (bb->buttonRole(b) == QDialogButtonBox::AcceptRole) { b->click(); return; }
    }
    qFatal("El diálogo no tiene botón Aceptar");
}


struct Aviso {
    QString clase, titulo, texto;
};

// Cierra automáticamente todo diálogo modal que aparezca (QMessageBox, QDialog.exec(), QInputDialog…)
// y lo registra. `alAbrir` permite rellenar/aceptar un diálogo concreto antes de cerrarlo.
class Modales : public QObject {
public:
    QList<Aviso> avisos;
    int respuestaPregunta = QMessageBox::Yes;
    std::function<void(QDialog *)> alAbrir;   // opcional; si acepta/rechaza el diálogo, no se toca más

    Modales() { m_timer = startTimer(5); }
    void limpiar() { avisos.clear(); }
    bool huboAviso() const { return !avisos.isEmpty(); }
    QString todosLosTextos() const {
        QStringList l;
        for (const auto &a : avisos) l << a.clase + ": " + a.texto;
        return l.join(" | ");
    }

protected:
    void timerEvent(QTimerEvent *) override {
        QWidget *w = QApplication::activeModalWidget();
        if (!w) return;
        if (auto *mb = qobject_cast<QMessageBox *>(w)) {
            avisos.append({"QMessageBox", mb->windowTitle(), mb->text() + " " + mb->informativeText()});
            // QMessageBox::question()/warning() devuelven el botón PULSADO: hay que hacer clic de verdad.
            QAbstractButton *elegido = mb->button(static_cast<QMessageBox::StandardButton>(respuestaPregunta));
            if (!elegido) elegido = mb->button(QMessageBox::Ok);
            if (!elegido)   // diálogos con botones propios: se prefiere el de «cerrar/rechazar»
                for (auto *b : mb->buttons())
                    if (mb->buttonRole(b) == QMessageBox::RejectRole) { elegido = b; break; }
            if (!elegido && !mb->buttons().isEmpty()) elegido = mb->buttons().last();
            if (elegido) elegido->click(); else mb->done(QMessageBox::Cancel);
            return;
        }
        if (auto *d = qobject_cast<QDialog *>(w)) {
            avisos.append({d->metaObject()->className(), d->windowTitle(), QString()});
            if (alAbrir) {
                alAbrir(d);
                if (!d->isVisible()) return;
            }
            d->reject();
            return;
        }
        w->close();
    }

private:
    int m_timer = 0;
};

}   // namespace T

#define FARMACIA_TEST_MAIN(Clase)                                  \
    int main(int argc, char **argv) {                              \
        T::prepararEntorno();                                      \
        QApplication app(argc, argv);                              \
        app.setOrganizationName("FarmaciaTest");                   \
        app.setApplicationName("FarmaciaTest");                    \
        Clase tc;                                                  \
        return QTest::qExec(&tc, argc, argv);                      \
    }
