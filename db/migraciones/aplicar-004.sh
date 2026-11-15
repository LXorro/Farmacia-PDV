#!/usr/bin/env bash
# Aplica la migración 004 en ESTE MariaDB (como root, por socket). Igual en un equipo y en el central.
#   sudo ./aplicar-004.sh
set -euo pipefail
AQUI="$(cd "$(dirname "$0")" && pwd)"
mariadb < "$AQUI/004_reembolsos_tarjeta.sql"
echo "Migración 004 aplicada."
