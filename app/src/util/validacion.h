#pragma once

#include <QString>

class QLineEdit;
class QWidget;

// Reglas de captura de los formularios. Cada validador devuelve "" si el valor es correcto o el mensaje
// (en español, listo para mostrar) si no. Se usan de dos formas: `filtrar*` impide TECLEAR o PEGAR caracteres
// que la regla no admite, y los validadores se vuelven a ejecutar al guardar (defensa en profundidad: el
// filtro del campo no basta, p. ej. un texto puesto por código, y la base solo limita el tamaño).
//
// Los largos coinciden con las columnas de la base (en modo estricto, un texto más largo falla al guardar).
namespace Validacion {

// «  Juan   Pérez » → «Juan Pérez»: quita espacios de los extremos y junta los repetidos.
QString normalizar(const QString &texto);

// ---- Validadores -------------------------------------------------------------------------------------

// Nombres y apellidos de personas: solo letras (con acentos y ñ), espacios, apóstrofo, guion y punto.
// Sin números ni símbolos. 2 a 50 caracteres.
QString nombrePersona(const QString &texto, const QString &campo, bool obligatorio);

// Nombres de cosas (producto, marca, empresa, sucursal, categoría, puesto): letras, números, espacios y
// . , ' & ( ) / % + # : - °. Debe llevar al menos una letra.
QString textoComercial(const QString &texto, const QString &campo, bool obligatorio, int largoMaximo = 100);

QString direccion(const QString &texto);                       // opcional, hasta 200
QString telefono(const QString &texto);                        // opcional, 7 a 15 dígitos
QString correo(const QString &texto);                          // opcional, hasta 100
QString nombreUsuario(const QString &texto);                   // 3 a 30: letras, números, . _ -
QString contrasena(const QString &texto);                      // 6 a 64, con letra y número, sin espacios
QString codigoBarras(const QString &texto);                    // opcional, 4 a 30: letras, números y guion
QString numeroLote(const QString &texto);                      // obligatorio, hasta 50
QString textoLibre(const QString &texto, const QString &campo, int largoMaximo);   // descripciones y motivos
QString direccionRed(const QString &texto);                    // nombre de equipo o IP del central

// ---- Filtros al teclear / pegar -----------------------------------------------------------------------

void filtrarNombrePersona(QLineEdit *campo);
void filtrarTextoComercial(QLineEdit *campo, int largoMaximo = 100);
void filtrarDireccion(QLineEdit *campo);
void filtrarTelefono(QLineEdit *campo);
void filtrarCorreo(QLineEdit *campo);
void filtrarNombreUsuario(QLineEdit *campo);
void filtrarCodigoBarras(QLineEdit *campo);
void filtrarNumeroLote(QLineEdit *campo);
void limitarLargo(QLineEdit *campo, int largo);               // solo el largo (contraseñas, búsquedas…)

// Si `error` no está vacío: muestra el aviso, deja el cursor en el campo y devuelve true (el formulario no debe
// guardar). Uso: `if (Validacion::rechazar(this, ui->lineNombre, Validacion::nombrePersona(...))) return;`
bool rechazar(QWidget *padre, QLineEdit *campo, const QString &error);

}   // namespace Validacion
