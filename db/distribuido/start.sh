#!/usr/bin/env bash
# Levanta las dos instancias MariaDB desechables de la demo distribuida
# (central:13306, sucursal:13307). No tocan ni dependen del servicio
# MariaDB del sistema que usa la app principal.
set -e
DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

mysqld --defaults-file="$DIR/central/my.cnf" > "$DIR/central/server.log" 2>&1 &
echo "central (pid $!) escuchando en 127.0.0.1:13306"
mysqld --defaults-file="$DIR/sucursal/my.cnf" > "$DIR/sucursal/server.log" 2>&1 &
echo "sucursal (pid $!) escuchando en 127.0.0.1:13307"

echo "Esperando a que ambas acepten conexiones..."
sleep 3

mysql --socket="$DIR/sucursal/mysql.sock" -u root -e "START SLAVE;" 2>/dev/null || true

echo "Listo. Verifica con:"
echo "  mysql --socket=$DIR/central/mysql.sock -u root"
echo "  mysql --socket=$DIR/sucursal/mysql.sock -u root"
