#!/usr/bin/env bash
# Aplica la migración 005 en ESTE MariaDB (como root, por socket). Igual en un equipo y en el central.
#   sudo ./aplicar-005.sh
set -euo pipefail
AQUI="$(cd "$(dirname "$0")" && pwd)"
mariadb < "$AQUI/005_totales_calculados.sql"
echo "Migración 005 aplicada."
