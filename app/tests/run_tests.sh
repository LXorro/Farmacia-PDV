#!/usr/bin/env bash
# Ejecuta las pruebas contra una MariaDB de PRUEBAS (nunca la de producción: se rechaza el puerto 3306).
#   FARMACIA_TEST_PORT   puerto de la BD de pruebas (por defecto 33307)
#   FARMACIA_TEST_SOCKET socket de la BD de pruebas (acceso root para recrear la base)
set -euo pipefail
cd "$(dirname "$0")"
PORT="${FARMACIA_TEST_PORT:-33307}"
SOCK="${FARMACIA_TEST_SOCKET:?define FARMACIA_TEST_SOCKET con el socket de la MariaDB de pruebas}"
[ "$PORT" != "3306" ] || { echo "Puerto 3306 rechazado: no se prueba contra producción"; exit 2; }
export FARMACIA_TEST_PORT="$PORT"
SQL() { mariadb --no-defaults -uroot -S "$SOCK" "$@"; }

reiniciar_bd() {
    SQL -e "DROP DATABASE IF EXISTS farmacia"
    SQL < ../../db/schema.sql
    SQL < ../../db/seed.sql
    SQL -e "CREATE USER IF NOT EXISTS 'farmacia_app'@'%' IDENTIFIED BY 'farmacia_app_pw';
            GRANT ALL PRIVILEGES ON farmacia.* TO 'farmacia_app'@'%'; FLUSH PRIVILEGES;"
}

cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug >/dev/null
cmake --build build -j"$(nproc)" 2>&1 | tail -n 3

fallos=0
for t in ${@:-validacion humo ventas bd formularios acceso}; do
    echo "=== test_$t ==="
    reiniciar_bd
    ./build/test_$t || fallos=$((fallos+1))
done
exit $fallos
