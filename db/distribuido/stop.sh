#!/usr/bin/env bash
# Apaga limpiamente las dos instancias de la demo distribuida.
set -e
DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

mysqladmin --socket="$DIR/central/mysql.sock" -u root shutdown 2>/dev/null || true
mysqladmin --socket="$DIR/sucursal/mysql.sock" -u root shutdown 2>/dev/null || true
echo "Ambas instancias apagadas."
