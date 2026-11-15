#include "lotedialog.h"
#include "ui_lotedialog.h"
#include "../database/sesionactual.h"
#include "../util/bitacora.h"

#include <QDate>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

LoteDialog::LoteDialog(QWidget *parent) : QDialog(parent), ui(new Ui::LoteDialog) {
    ui->setupUi(this);
    ui->dateFechaCaducidad->setDisplayFormat("dd/MM/yyyy");
    ui->dateFechaCaducidad->setDate(QDate::currentDate().addYears(1));
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &LoteDialog::guardar);
    connect(ui->spinCantidad, qOverload<int>(&QSpinBox::valueChanged), this, &LoteDialog::actualizarEstado);
    connect(ui->dateFechaCaducidad, &QDateEdit::dateChanged, this, &LoteDialog::actualizarEstado);
    cargarCombos();
    actualizarEstado();

    if (!SesionActual::instance().veTodasLasSucursales()) {
        int idx = ui->comboSucursal->findData(SesionActual::instance().idSucursal());
        if (idx >= 0)
            ui->comboSucursal->setCurrentIndex(idx);
        ui->comboSucursal->setEnabled(false);
    }
}

LoteDialog::~LoteDialog() {
    delete ui;
}

// El estado no se captura: se deduce de la cantidad y la caducidad.
QString LoteDialog::estadoDerivado() const {
    if (ui->spinCantidad->value() <= 0)
        return "agotado";
    if (ui->dateFechaCaducidad->date() < QDate::currentDate())
        return "caducado";
    return "disponible";
}

void LoteDialog::actualizarEstado() {
    ui->labelEstadoValor->setText(estadoDerivado());
}

void LoteDialog::cargarCombos() {
    QSqlQuery productos("SELECT id_producto, nombre FROM Productos WHERE estado = 'activo' ORDER BY nombre");
    while (productos.next())
        ui->comboProducto->addItem(productos.value(1).toString(), productos.value(0));

    QSqlQuery sucursales("SELECT id_sucursal, nombre FROM Sucursales ORDER BY nombre");
    while (sucursales.next())
        ui->comboSucursal->addItem(sucursales.value(1).toString(), sucursales.value(0));
}

void LoteDialog::setLoteId(int id) {
    m_idLote = id;
    cargarLote();
}

void LoteDialog::cargarLote() {
    if (m_idLote < 0)
        return;

    QSqlQuery query;
    query.prepare("SELECT id_producto, id_sucursal, numero_lote, fecha_caducidad, cantidad_disponible, costo_unitario "
                  "FROM Lotes WHERE id_lote = :id");
    query.bindValue(":id", m_idLote);
    if (!query.exec() || !query.next())
        return;

    // Un producto descontinuado no sale en el combo, pero sus lotes siguen siendo editables.
    int idxProducto = ui->comboProducto->findData(query.value(0));
    if (idxProducto < 0) {
        QSqlQuery nombre;
        nombre.prepare("SELECT nombre FROM Productos WHERE id_producto = :id");
        nombre.bindValue(":id", query.value(0));
        if (nombre.exec() && nombre.next()) {
            ui->comboProducto->addItem(nombre.value(0).toString(), query.value(0));
            idxProducto = ui->comboProducto->count() - 1;
        }
    }
    ui->comboProducto->setCurrentIndex(idxProducto);
    ui->comboSucursal->setCurrentIndex(ui->comboSucursal->findData(query.value(1)));
    ui->lineNumeroLote->setText(query.value(2).toString());
    ui->dateFechaCaducidad->setDate(query.value(3).toDate());

    // Al editar, el producto/sucursal de un lote ya existente no se cambian
    // (hay ventas/movimientos de inventario ligados a este lote por id).
    ui->comboProducto->setEnabled(false);
    ui->comboSucursal->setEnabled(false);
    ui->labelCantidad->setText("Cantidad disponible:");
    // Al editar se permite dejar el lote en 0 (agotado manualmente / merma).
    ui->spinCantidad->setMinimum(0);
    ui->spinCantidad->setValue(query.value(4).toInt());
    ui->spinCosto->setValue(query.value(5).toDouble());
    m_antes = descripcionLote();
}

QString LoteDialog::descripcionLote() const {
    return QString("lote %1, caduca %2, disponible %3, costo $%4")
        .arg(ui->lineNumeroLote->text().trimmed(), ui->dateFechaCaducidad->date().toString("dd/MM/yyyy"))
        .arg(ui->spinCantidad->value())
        .arg(ui->spinCosto->value(), 0, 'f', 2);
}

void LoteDialog::guardar() {
    if (ui->comboProducto->currentIndex() < 0 || ui->comboSucursal->currentIndex() < 0) {
        QMessageBox::warning(this, "Validación", "Selecciona producto y sucursal.");
        return;
    }

    // El permiso pudo vencer o revocarse con la ventana abierta.
    if (!SesionActual::instance().puedeGestionarSucursal()) {
        QMessageBox::warning(this, "Sin permiso",
                             "Tu permiso temporal ya no está vigente. Pide a un administrador que lo renueve.");
        reject();
        return;
    }

    QSqlDatabase base = QSqlDatabase::database();
    base.transaction();
    QSqlQuery query;
    if (m_idLote < 0) {
        query.prepare(
            "INSERT INTO Lotes (id_producto, id_sucursal, numero_lote, fecha_caducidad, "
            "cantidad_inicial, cantidad_disponible, costo_unitario, estado) "
            "VALUES (:producto, :sucursal, :numero, :caducidad, :cantidad, :cantidad, :costo, :estado)");
    } else {
        query.prepare(
            "UPDATE Lotes SET numero_lote = :numero, fecha_caducidad = :caducidad, "
            "cantidad_disponible = :cantidad, costo_unitario = :costo, estado = :estado WHERE id_lote = :id");
        query.bindValue(":id", m_idLote);
    }

    query.bindValue(":producto", ui->comboProducto->currentData());
    query.bindValue(":sucursal", ui->comboSucursal->currentData());
    query.bindValue(":numero", ui->lineNumeroLote->text().trimmed());
    query.bindValue(":caducidad", ui->dateFechaCaducidad->date());
    query.bindValue(":cantidad", ui->spinCantidad->value());
    query.bindValue(":costo", ui->spinCosto->value());
    query.bindValue(":estado", estadoDerivado());

    const bool alta = m_idLote < 0;
    const bool guardado = query.exec();
    const QString idRegistro = QString::number(alta ? query.lastInsertId().toInt() : m_idLote);
    QString errorBitacora;
    const QString producto = ui->comboProducto->currentText();
    const QString detalle = alta ? QString("%1: nuevo, %2").arg(producto, descripcionLote())
                                 : QString("%1: %2 → %3").arg(producto, m_antes, descripcionLote());
    if (!guardado || !Bitacora::registrar(alta ? "lote_alta" : "lote_edicion", "Lotes", idRegistro, detalle,
                                          errorBitacora)) {
        base.rollback();
        QMessageBox::critical(this, "Error",
                              "No se pudo guardar el lote:\n" + (guardado ? errorBitacora : query.lastError().text()));
        return;
    }
    base.commit();

    accept();
}
