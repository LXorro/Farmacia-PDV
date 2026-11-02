#!/usr/bin/env bash
# Demo de autonomía local: central + dos sucursales (norte, sur) con el esquema
# REAL de la app y replicación en ambos sentidos.
#
#   central (13310) --- catálogo, sucursales, empleados y usuarios ---> norte (13311), sur (13312)
#   central (13310) <--- clientes, lotes, tickets, ventas, pagos... --- norte, sur
#
# Cada sucursal opera contra SU instancia local; si se corta el central, sigue
# vendiendo y al volver la conexión la replicación se pone al día sola.
#
# Uso: ./demo.sh init | start | stop | status | corte | restaurar | prueba | limpiar
set -euo pipefail

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SCHEMA="$DIR/../../schema.sql"
NODOS=(central norte sur)
declare -A PUERTO=([central]=13310 [norte]=13311 [sur]=13312)
declare -A ID=([central]=1 [norte]=2 [sur]=3)
PW_REMOTO=remoto_pw   # solo demo
INCREMENTO=10   # auto_increment_increment; el offset de cada nodo es su ID

# Tablas que se capturan en el central y bajan a las sucursales.
TABLAS_ABAJO=(Sucursales Empleados Usuarios Asignaciones_Temporales Categorias Proveedores Productos)
# Tablas que cada sucursal genera y suben al central.
TABLAS_ARRIBA=(Clientes Lotes Inventario Tickets Ventas Pagos Devoluciones Detalle_Devolucion)

sock() { echo "$DIR/nodos/$1/mysql.sock"; }
sql()  { local n=$1; shift; mariadb --socket="$(sock "$n")" -uroot "$@"; }

escribir_config() {
    local n=$1 d="$DIR/nodos/$1"
    cat > "$d/my.cnf" <<CNF
[mysqld]
datadir=$d/data
socket=$d/mysql.sock
pid-file=$d/mysqld.pid
port=${PUERTO[$n]}
bind-address=127.0.0.1
server-id=${ID[$n]}
gtid_domain_id=${ID[$n]}
log-bin=$n-bin
# ROW: los triggers no se vuelven a disparar en el nodo que replica.
binlog_format=ROW
auto_increment_increment=$INCREMENTO
auto_increment_offset=${ID[$n]}
CNF
    if [[ $n == central ]]; then
        for s in norte sur; do
            for t in "${TABLAS_ARRIBA[@]}"; do echo "$s.replicate-do-table=farmacia.$t" >> "$d/my.cnf"; done
        done
    else
        for t in "${TABLAS_ABAJO[@]}"; do echo "replicate-do-table=farmacia.$t" >> "$d/my.cnf"; done
    fi
}

arrancar() {
    local n=$1
    mariadbd --defaults-file="$DIR/nodos/$n/my.cnf" > "$DIR/nodos/$n/server.log" 2>&1 &
    for _ in $(seq 1 60); do sql "$n" -e "SELECT 1" >/dev/null 2>&1 && return 0; sleep 0.5; done
    echo "No arrancó $n; revisa nodos/$n/server.log" >&2; return 1
}

detener() { mariadb-admin --socket="$(sock "$1")" -uroot shutdown 2>/dev/null || true; }

# Espera hasta que una consulta (que devuelve un número) alcance el valor esperado.
esperar() { # nodo consulta esperado descripción
    local n=$1 q=$2 esperado=$3 desc=$4 v=""
    for _ in $(seq 1 60); do
        v=$(sql "$n" -N -e "$q" 2>/dev/null || echo "")
        [[ "$v" == "$esperado" ]] && { echo "  ok: $desc ($n = $v)"; return 0; }
        sleep 0.5
    done
    echo "  FALLO: $desc ($n = '$v', esperado $esperado)" >&2; return 1
}

cmd_init() {
    for n in "${NODOS[@]}"; do
        [[ -d "$DIR/nodos/$n/data" ]] && { echo "Ya existe nodos/$n; usa ./demo.sh limpiar primero" >&2; exit 1; }
    done
    for n in "${NODOS[@]}"; do
        mkdir -p "$DIR/nodos/$n"
        mariadb-install-db --no-defaults --datadir="$DIR/nodos/$n/data" --auth-root-authentication-method=normal \
            --skip-test-db > "$DIR/nodos/$n/install.log" 2>&1
        escribir_config "$n"
        arrancar "$n"
        # Esquema sin registrar en el binlog: cada nodo parte con las mismas tablas y la
        # replicación solo transporta datos (no DDL ni triggers).
        { echo "SET sql_log_bin=0;"; cat "$SCHEMA"; } | sql "$n"
        sql "$n" -e "SET sql_log_bin=0; CREATE USER 'repl'@'127.0.0.1' IDENTIFIED BY 'repl_pw'; GRANT REPLICATION SLAVE ON *.* TO 'repl'@'127.0.0.1';"
        echo "nodo $n listo (puerto ${PUERTO[$n]}, offset ${ID[$n]})"
    done

    # Cuenta con la que un admin de una sucursal administra Usuarios EN el central (la crea el
    # asistente de la app cuando el equipo se configura como "Central"). Sin binlog: es solo del central.
    sql central -e "SET sql_log_bin=0;
        CREATE USER 'farmacia_remoto'@'%' IDENTIFIED BY '$PW_REMOTO';
        GRANT SELECT, INSERT, UPDATE, DELETE ON farmacia.Usuarios TO 'farmacia_remoto'@'%';
        GRANT SELECT ON farmacia.Empleados TO 'farmacia_remoto'@'%';
        GRANT SELECT ON farmacia.Sucursales TO 'farmacia_remoto'@'%';"

    # Sucursales: replican del central las tablas "hacia abajo".
    for n in norte sur; do
        sql "$n" -e "CHANGE MASTER TO MASTER_HOST='127.0.0.1', MASTER_PORT=${PUERTO[central]}, MASTER_USER='repl', MASTER_PASSWORD='repl_pw', MASTER_USE_GTID=slave_pos, MASTER_CONNECT_RETRY=2; START SLAVE;"
    done
    # Central: una conexión de replicación por sucursal ("hacia arriba").
    for n in norte sur; do
        sql central -e "CHANGE MASTER '$n' TO MASTER_HOST='127.0.0.1', MASTER_PORT=${PUERTO[$n]}, MASTER_USER='repl', MASTER_PASSWORD='repl_pw', MASTER_USE_GTID=slave_pos, MASTER_CONNECT_RETRY=2;"
    done
    sql central -e "START ALL SLAVES;"

    echo "Cargando datos del central…"
    sql central < "$DIR/datos_central.sql"
    for n in norte sur; do
        esperar "$n" "SELECT COUNT(*) FROM farmacia.Productos" 3 "catálogo llegó"
        { echo "SET @suc=${ID[$n]};"; cat "$DIR/datos_sucursal.sql"; } | sql "$n"
    done
    echo "Listo. Prueba automática: ./demo.sh prueba"
}

cmd_start() {
    for n in "${NODOS[@]}"; do
        if sql "$n" -e "SELECT 1" >/dev/null 2>&1; then echo "$n ya estaba arriba"; else arrancar "$n"; echo "$n arriba (puerto ${PUERTO[$n]})"; fi
    done
}

cmd_stop() { for n in "${NODOS[@]}"; do detener "$n"; done; echo "Nodos apagados."; }

cmd_status() {
    for n in norte sur; do
        echo "== $n replicando del central"
        sql "$n" -e "SHOW SLAVE STATUS\G" 2>/dev/null | grep -E "Slave_IO_Running:|Slave_SQL_Running:|Seconds_Behind_Master|Last_IO_Error:" || echo "  (nodo apagado)"
    done
    echo "== central replicando de las sucursales"
    sql central -e "SHOW ALL SLAVES STATUS\G" 2>/dev/null | grep -E "Connection_name|Slave_IO_Running:|Slave_SQL_Running:" || echo "  (nodo apagado)"
}

cmd_corte() { detener central; echo "Central apagado: las sucursales siguen operando con su base local."; }
cmd_restaurar() { arrancar central; echo "Central de vuelta: la replicación se reanuda sola."; }

# Una venta en una sucursal; imprime el id del ticket.
vender() { # nodo empleado cantidad
    { echo "SET @suc=${ID[$1]}, @emp=$2, @cant=$3;"; cat "$DIR/venta.sql"; } | sql "$1" -N | tail -1
}

cmd_prueba() {
    local fallos=0 t_norte t_sur
    echo "1) Operación normal (ambos sentidos)"
    esperar central "SELECT COUNT(*) FROM farmacia.Lotes" 4 "lotes de las sucursales llegaron al central" || fallos=$((fallos+1))
    t_norte=$(vender norte 2 5); t_sur=$(vender sur 3 3)
    echo "  ticket norte=$t_norte, ticket sur=$t_sur"
    [[ "$t_norte" != "$t_sur" ]] && echo "  ok: IDs distintos entre nodos" || { echo "  FALLO: IDs repetidos" >&2; fallos=$((fallos+1)); }
    esperar central "SELECT COUNT(*) FROM farmacia.Tickets" 2 "ventas llegaron al central" || fallos=$((fallos+1))

    echo "2) Corte del central: las sucursales siguen vendiendo"
    cmd_corte
    vender norte 2 2 >/dev/null; vender sur 3 4 >/dev/null; vender norte 2 1 >/dev/null
    echo "  ok: 3 ventas más sin conexión al central"
    esperar norte "SELECT SUM(cantidad_disponible) FROM farmacia.Lotes WHERE id_producto=1" 92 "stock local descontado (100-5-2-1)" || fallos=$((fallos+1))

    echo "3) Central de vuelta: se pone al día solo"
    cmd_restaurar
    esperar central "SELECT COUNT(*) FROM farmacia.Tickets" 5 "central recibió las 5 ventas" || fallos=$((fallos+1))
    esperar central "SELECT COUNT(DISTINCT id_ticket) FROM farmacia.Ventas" 5 "sin tickets duplicados ni perdidos" || fallos=$((fallos+1))
    esperar central "SELECT SUM(cantidad_disponible) FROM farmacia.Lotes WHERE id_producto=1" 185 "stock consolidado (200-5-3-2-4-1)" || fallos=$((fallos+1))

    echo "4) Catálogo: central -> sucursales"
    sql central -e "UPDATE farmacia.Productos SET precio_sugerido = 30.00 WHERE id_producto = 1;"
    esperar norte "SELECT precio_sugerido FROM farmacia.Productos WHERE id_producto=1" 30.00 "nuevo precio llegó a norte" || fallos=$((fallos+1))
    esperar sur   "SELECT precio_sugerido FROM farmacia.Productos WHERE id_producto=1" 30.00 "nuevo precio llegó a sur" || fallos=$((fallos+1))

    echo "5) Usuarios: se crean en el central (con conexión) y bajan a las sucursales"
    local remoto=(mariadb -h127.0.0.1 -P"${PUERTO[central]}" -ufarmacia_remoto -p"$PW_REMOTO" farmacia)
    "${remoto[@]}" -e "INSERT INTO Usuarios (nombre_usuario, contrasena_hash, sal, rol) VALUES ('nuevo_remoto', SHA2('x',256), 'x', 'empleado')"
    esperar norte "SELECT COUNT(*) FROM farmacia.Usuarios WHERE nombre_usuario='nuevo_remoto'" 1 "cuenta nueva llegó a norte" || fallos=$((fallos+1))
    esperar sur   "SELECT COUNT(*) FROM farmacia.Usuarios WHERE nombre_usuario='nuevo_remoto'" 1 "cuenta nueva llegó a sur" || fallos=$((fallos+1))
    if "${remoto[@]}" -e "UPDATE Productos SET precio_sugerido = 1 WHERE id_producto = 1" 2>/dev/null; then
        echo "  FALLO: la cuenta remota pudo modificar Productos" >&2; fallos=$((fallos+1))
    else
        echo "  ok: la cuenta remota no puede tocar nada fuera de Usuarios"
    fi
    echo "6) Asignación temporal: se autoriza en el central y baja a las sucursales"
    sql central -e "INSERT INTO farmacia.Asignaciones_Temporales (id_empleado, id_sucursal, fecha_inicio, fecha_fin, id_autoriza)
                    VALUES (2, 3, CURDATE(), DATE_ADD(CURDATE(), INTERVAL 7 DAY), 1)"
    esperar sur   "SELECT COUNT(*) FROM farmacia.Asignaciones_Temporales WHERE id_empleado=2 AND id_sucursal=3" 1 "asignación llegó a sur (donde se aprovecha)" || fallos=$((fallos+1))
    esperar norte "SELECT COUNT(*) FROM farmacia.Asignaciones_Temporales" 1 "asignación llegó a norte" || fallos=$((fallos+1))

    cmd_corte
    if "${remoto[@]}" -e "SELECT 1" 2>/dev/null; then
        echo "  FALLO: con el central caído no debería haber conexión" >&2; fallos=$((fallos+1))
    else
        echo "  ok: sin conexión al central no se pueden crear cuentas"
    fi
    cmd_restaurar

    [[ $fallos -eq 0 ]] && echo "TODO OK" || { echo "$fallos verificación(es) fallaron" >&2; exit 1; }
}

cmd_limpiar() { cmd_stop; rm -rf "$DIR/nodos"; echo "nodos/ eliminado."; }

case "${1:-}" in
    init) cmd_init ;; start) cmd_start ;; stop) cmd_stop ;; status) cmd_status ;;
    corte) cmd_corte ;; restaurar) cmd_restaurar ;; prueba) cmd_prueba ;; limpiar) cmd_limpiar ;;
    *) echo "Uso: $0 init|start|stop|status|corte|restaurar|prueba|limpiar"; exit 1 ;;
esac
