#!/usr/bin/env bash
# Prepara el MariaDB del central para el modelo «sesión»: administradores trabajan sobre el central y las
# sucursales se dan de alta solas desde el asistente de la app. Idempotente; ejecutar EN el central, como root:
#
#   ssh ubuntu@<central> 'sudo bash -s' < preparar-central.sh
#
# Requisitos previos (ya en /etc/mysql/mariadb.conf.d/60-central.cnf): server-id=1, log-bin, binlog_format=ROW,
# auto_increment_increment=10, auto_increment_offset=1, TLS, bind-address accesible por Tailscale.
# Cuentas (solo desde el tailnet 100.64.0.0/10):
#   farmacia_remoto  — la que usa un administrador al iniciar sesión: escribe lo del central, lee lo operativo.
#   farmacia_registro — solo para dar de alta una sucursal (crea su cuenta de replicación y enlaza la replicación).
set -euo pipefail

HOSTS="100.64.0.0/255.192.0.0"
DB=farmacia
ESCRIBE="Sucursales Empleados Usuarios Asignaciones_Temporales Categorias Proveedores Productos"
LEE="Clientes Lotes Inventario Tickets Ventas Pagos Devoluciones Detalle_Devolucion Productos_Sucursal"
PW_REGISTRO="${PW_REGISTRO:-$(openssl rand -base64 24 | tr -dc 'A-Za-z0-9' | head -c 24)}"

{
  echo "SET sql_log_bin = 0;"

  echo "CREATE USER IF NOT EXISTS 'farmacia_registro'@'$HOSTS' IDENTIFIED BY '$PW_REGISTRO';"
  echo "ALTER USER 'farmacia_registro'@'$HOSTS' IDENTIFIED BY '$PW_REGISTRO';"
  echo "REVOKE ALL PRIVILEGES, GRANT OPTION FROM 'farmacia_registro'@'$HOSTS';"
  echo "GRANT CREATE USER, RELOAD, BINLOG ADMIN, REPLICATION SLAVE ADMIN, REPLICA MONITOR ON *.* TO 'farmacia_registro'@'$HOSTS';"
  echo "GRANT REPLICATION SLAVE ON *.* TO 'farmacia_registro'@'$HOSTS' WITH GRANT OPTION;"
  echo "GRANT SELECT ON $DB.* TO 'farmacia_registro'@'$HOSTS';"

  # La cuenta ya existe (la creó el asistente anterior): se conserva su contraseña y solo se ajustan permisos.
  echo "CREATE USER IF NOT EXISTS 'farmacia_remoto'@'$HOSTS' IDENTIFIED BY '$(openssl rand -hex 16)';"
  echo "REVOKE ALL PRIVILEGES, GRANT OPTION FROM 'farmacia_remoto'@'$HOSTS';"
  for t in $ESCRIBE; do echo "GRANT SELECT, INSERT, UPDATE, DELETE ON $DB.$t TO 'farmacia_remoto'@'$HOSTS';"; done
  for t in $LEE;     do echo "GRANT SELECT ON $DB.$t TO 'farmacia_remoto'@'$HOSTS';"; done
  echo "GRANT REPLICA MONITOR ON *.* TO 'farmacia_remoto'@'$HOSTS';"
  echo "FLUSH PRIVILEGES;"
} | mariadb

echo "Contraseña de farmacia_registro (se pide en el asistente; no se guarda en la app): $PW_REGISTRO"
