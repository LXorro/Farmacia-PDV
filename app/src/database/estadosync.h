#pragma once

#include <QString>

// Estado de la replicación visto desde este equipo (solo tiene sentido en Central y Sucursal).
namespace EstadoSync {

enum class Nivel { Desconocido, Bien, Retraso, SinConexion };

struct Estado {
    Nivel nivel = Nivel::Desconocido;
    QString texto;
    QString detalle;   // para el tooltip
};

// Consulta SHOW REPLICA STATUS en la base local. No toca el central ni pide credenciales.
Estado consultar();

}
