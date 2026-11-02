#!/usr/bin/env bash
# Prepara el MariaDB de ESTE equipo para replicar con el central (lo que MariaDB solo acepta en su archivo
# de configuración). Es el único paso manual; el resto lo hace el asistente de la app.
#
#   sudo ./preparar-nodo.sh <ranura>      (la ranura 2..10 la indica el asistente al fallar la comprobación)
set -euo pipefail

[ "$(id -u)" = 0 ] || { echo "Ejecútalo con sudo." >&2; exit 1; }
RANURA="${1:?Falta la ranura (2..10). El asistente de la app la muestra.}"
[[ "$RANURA" =~ ^([2-9]|10)$ ]] || { echo "La ranura debe estar entre 2 y 10." >&2; exit 1; }

if   [ -d /etc/my.cnf.d ];                    then DESTINO=/etc/my.cnf.d/farmacia-nodo.cnf
elif [ -d /etc/mysql/mariadb.conf.d ];        then DESTINO=/etc/mysql/mariadb.conf.d/60-farmacia-nodo.cnf
else echo "No encuentro el directorio de configuración de MariaDB." >&2; exit 1; fi

TABLAS_DEL_CENTRAL="Sucursales Empleados Usuarios Asignaciones_Temporales Categorias Proveedores Productos"
{
  echo "[mysqld]"
  echo "server-id=$((10 + RANURA))"
  echo "gtid_domain_id=$((10 + RANURA))"
  echo "log-bin=farmacia-bin"
  echo "binlog_format=ROW"
  echo "auto_increment_increment=10"
  echo "auto_increment_offset=$RANURA"
  echo "bind-address=0.0.0.0"
  for t in $TABLAS_DEL_CENTRAL; do echo "replicate-do-table=farmacia.$t"; done
} > "$DESTINO"
echo "Escrito $DESTINO"

systemctl restart mariadb 2>/dev/null || systemctl restart mysql
echo "MariaDB reiniciado. Vuelve al asistente y pulsa «Configurar»."
