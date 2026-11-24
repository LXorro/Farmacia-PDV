#!/usr/bin/env bash
# Levanta (o detiene) una MariaDB DESECHABLE para las pruebas: otro puerto (33307), otro directorio, su propio
# socket. No toca el MariaDB real de la app (puerto 3306).
#   ./bd_pruebas.sh start   → crea la instancia y muestra la variable FARMACIA_TEST_SOCKET para run_tests.sh
#   ./bd_pruebas.sh stop    → la apaga y borra sus datos
set -euo pipefail
DIR="${FARMACIA_TEST_DIR:-/tmp/farmacia-test-db}"
PORT="${FARMACIA_TEST_PORT:-33307}"
[ "$PORT" != "3306" ] || { echo "Puerto 3306 rechazado"; exit 2; }
case "${1:-start}" in
  start)
    mkdir -p "$DIR"
    [ -d "$DIR/data/mysql" ] || mariadb-install-db --no-defaults --datadir="$DIR/data" \
        --auth-root-authentication-method=normal --skip-test-db >"$DIR/install.log" 2>&1
    nohup mariadbd --no-defaults --datadir="$DIR/data" --socket="$DIR/m.sock" --port="$PORT" \
        --bind-address=127.0.0.1 --pid-file="$DIR/m.pid" --log-error="$DIR/err.log" \
        --sql-mode=STRICT_TRANS_TABLES,NO_ENGINE_SUBSTITUTION >/dev/null 2>&1 &
    for _ in $(seq 30); do mariadb --no-defaults -uroot -S "$DIR/m.sock" -e 'select 1' >/dev/null 2>&1 && break; sleep 0.5; done
    echo "Lista. Ejecuta:  FARMACIA_TEST_SOCKET=$DIR/m.sock ./run_tests.sh" ;;
  stop)
    [ -f "$DIR/m.pid" ] && kill "$(cat "$DIR/m.pid")" 2>/dev/null || true
    sleep 1; rm -rf "$DIR"; echo "BD de pruebas eliminada." ;;
esac
