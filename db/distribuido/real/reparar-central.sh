#!/usr/bin/env bash
# Recrea la base «farmacia» del central (esquema + lo que baja del central) a partir del esquema del
# repositorio y de la copia que tiene ESTE equipo. Sin sudo; pide la contraseña de sudo para el dump local.
# Solo hace falta si el central perdió su base (p. ej. por un DROP DATABASE replicado hacia arriba).
#
#   LLAVE=/tmp/k.pem ./reparar-central.sh
set -euo pipefail
cd "$(dirname "$0")/../../.."

CENTRAL="${CENTRAL:-ubuntu@100.106.156.124}"
LLAVE="${LLAVE:?Indica la llave SSH: LLAVE=/ruta/llave.pem}"
TABLAS="Sucursales Empleados Usuarios Asignaciones_Temporales Categorias Proveedores Productos"

# sql_log_bin=0: nada de esto debe viajar por replicación (las sucursales ya tienen estos datos).
{
  echo "SET sql_log_bin = 0;"
  cat db/schema.sql
  echo "USE farmacia; SET FOREIGN_KEY_CHECKS = 0;"
  # shellcheck disable=SC2086
  sudo mariadb-dump --no-create-info --skip-triggers --skip-add-locks --complete-insert --single-transaction farmacia $TABLAS
} | ssh -i "$LLAVE" -o BatchMode=yes "$CENTRAL" 'sudo mariadb'
echo "Base «farmacia» recreada en el central."
