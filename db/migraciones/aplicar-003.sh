#!/usr/bin/env bash
# Aplica la migración 003 en ESTE MariaDB (como root, por socket).
#   sudo ./aplicar-003.sh nodo        equipo con la app: además da permisos a farmacia_app si existe
#   sudo ./aplicar-003.sh central     el central: solo el esquema (los permisos los da preparar-central.sh)
# En un nodo, después hay que volver a ejecutar db/distribuido/real/preparar-nodo.sh <ranura> para que
# Permisos_Temporales entre en la lista de tablas que replica del central.
set -euo pipefail
MODO="${1:?Indica «nodo» o «central»}"
AQUI="$(cd "$(dirname "$0")" && pwd)"
mariadb < "$AQUI/003_permisos_y_bitacora.sql"

if [ "$MODO" = nodo ]; then
  for host in localhost 127.0.0.1; do
    existe=$(mariadb -N -e "SELECT COUNT(*) FROM mysql.user WHERE user='farmacia_app' AND host='$host'")
    if [ "$existe" = 1 ]; then
      mariadb -e "SET sql_log_bin=0;
        GRANT SELECT ON farmacia.Permisos_Temporales TO 'farmacia_app'@'$host';
        GRANT SELECT, INSERT, UPDATE, DELETE ON farmacia.Bitacora_Cambios TO 'farmacia_app'@'$host';
        FLUSH PRIVILEGES;"
    fi
  done
  echo "Recuerda: sudo db/distribuido/real/preparar-nodo.sh <ranura>   (añade Permisos_Temporales a la réplica)"
fi
echo "Migración 003 aplicada ($MODO)."
