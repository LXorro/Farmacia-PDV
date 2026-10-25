#pragma once

#include <QIcon>
#include <QString>

// Iconos Tabler (MIT, https://tabler.io/icons) empaquetados en el .qrc bajo
// resources/icons/ — no dependen del tema de iconos del sistema. Todos son
// blancos salvo "app" (insignia a color, logo-mark.svg), porque van sobre botones verdes o el
// menú oscuro.
namespace Iconos {
inline QIcon icono(const QString &nombre) { return QIcon(":/icons/" + nombre + ".svg"); }

inline QIcon nuevo()      { return icono("plus"); }
inline QIcon editar()     { return icono("edit"); }
inline QIcon eliminar()   { return icono("trash"); }
inline QIcon refrescar()  { return icono("refresh"); }

inline QIcon inicio()           { return icono("home"); }
inline QIcon puntoDeVenta()     { return icono("cash-register"); }
inline QIcon productos()        { return icono("pill"); }
inline QIcon clientes()         { return icono("user"); }
inline QIcon tickets()          { return icono("receipt"); }
inline QIcon inventario()       { return icono("packages"); }
inline QIcon categorias()       { return icono("tag"); }
inline QIcon proveedores()      { return icono("truck-delivery"); }
inline QIcon empleados()        { return icono("users"); }
inline QIcon sucursales()       { return icono("building-store"); }
inline QIcon usuarios()         { return icono("user-cog"); }

inline QIcon agregarAlCarrito() { return icono("shopping-cart-plus"); }
inline QIcon quitarLinea()      { return icono("minus"); }
inline QIcon cancelar()         { return icono("arrow-back-up"); }
inline QIcon cobrar()           { return icono("circle-check"); }
inline QIcon verDetalle()       { return icono("eye"); }
inline QIcon app()            { return QIcon(":/logo-mark.svg"); }
inline QIcon cerrar()           { return icono("x"); }
inline QIcon minimizar()        { return icono("minus"); }
inline QIcon ingresar()         { return icono("login"); }
inline QIcon registrarCuenta()  { return icono("user-plus"); }
}
