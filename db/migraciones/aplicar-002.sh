#!/usr/bin/env bash
# Aplica la migración 002 en ESTE MariaDB (como root, por socket).
#   sudo ./aplicar-002.sh nodo        equipo con la app: además da permisos a farmacia_app si existe
#   sudo ./aplicar-002.sh central     el central: solo el esquema (los permisos los da preparar-central.sh)
set -euo pipefail
MODO="${1:?Indica «nodo» o «central»}"
AQUI="$(cd "$(dirname "$0")" && pwd)"
mariadb < "$AQUI/002_precios_por_sucursal.sql"

if [ "$MODO" = nodo ]; then
  for host in localhost 127.0.0.1; do
    existe=$(mariadb -N -e "SELECT COUNT(*) FROM mysql.user WHERE user='farmacia_app' AND host='$host'")
    if [ "$existe" = 1 ]; then
      mariadb -e "SET sql_log_bin=0; GRANT SELECT, INSERT, UPDATE, DELETE ON farmacia.Productos_Sucursal TO 'farmacia_app'@'$host'; FLUSH PRIVILEGES;"
    fi
  done
fi
echo "Migración 002 aplicada ($MODO)."
