#include "productosucursaldialog.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QMessageBox>
#include <QSpinBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QVBoxLayout>

ProductoSucursalDialog::ProductoSucursalDialog(int idProducto, int idSucursal, const QString &nombreProducto,
                                               double precioSugerido, QWidget *parent)
    : QDialog(parent), m_idProducto(idProducto), m_idSucursal(idSucursal) {
    setWindowTitle("Producto en esta sucursal");

    m_precio = new QDoubleSpinBox(this);
    m_precio->setRange(0.0, 999999.0);
    m_precio->setDecimals(2);
    m_precio->setPrefix("$ ");
    // 0 significa «sin precio propio»: se cobra el sugerido del catálogo.
    m_precio->setSpecialValueText(QString("Usar el sugerido ($ %1)").arg(precioSugerido, 0, 'f', 2));
    m_activo = new QCheckBox("Esta sucursal maneja el producto", this);
    m_minimo = new QSpinBox(this);
    m_minimo->setRange(0, 999999);
    m_minimo->setToolTip("Con existencias iguales o menores se marca «bajo mínimo». 0 = sin aviso.");

    auto *formulario = new QFormLayout;
    formulario->addRow("Precio de venta:", m_precio);
    formulario->addRow("Existencias mínimas:", m_minimo);
    formulario->addRow(QString(), m_activo);

    auto *botones = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);
    connect(botones, &QDialogButtonBox::accepted, this, &ProductoSucursalDialog::guardar);
    connect(botones, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *capa = new QVBoxLayout(this);
    capa->addWidget(new QLabel("<b>" + nombreProducto.toHtmlEscaped() + "</b>", this));
    capa->addLayout(formulario);
    capa->addWidget(botones);

    QSqlQuery actual;
    actual.prepare("SELECT precio_venta, activo, stock_minimo FROM Productos_Sucursal "
                   "WHERE id_producto = :p AND id_sucursal = :s");
    actual.bindValue(":p", idProducto);
    actual.bindValue(":s", idSucursal);
    if (actual.exec() && actual.next()) {
        m_precio->setValue(actual.value(0).isNull() ? 0.0 : actual.value(0).toDouble());
        m_activo->setChecked(actual.value(1).toBool());
        m_minimo->setValue(actual.value(2).toInt());
    } else {
        m_activo->setChecked(true);
    }
}

void ProductoSucursalDialog::guardar() {
    QSqlQuery query;
    query.prepare("INSERT INTO Productos_Sucursal (id_producto, id_sucursal, precio_venta, activo, stock_minimo) "
                  "VALUES (:p, :s, :precio, :activo, :minimo) "
                  "ON DUPLICATE KEY UPDATE precio_venta = VALUES(precio_venta), activo = VALUES(activo), "
                  "stock_minimo = VALUES(stock_minimo)");
    query.bindValue(":p", m_idProducto);
    query.bindValue(":s", m_idSucursal);
    query.bindValue(":precio", m_precio->value() > 0.0 ? QVariant(m_precio->value())
                                                        : QVariant(QMetaType(QMetaType::Double)));
    query.bindValue(":activo", m_activo->isChecked());
    query.bindValue(":minimo", m_minimo->value());
    if (!query.exec()) {
        QMessageBox::critical(this, "Error", "No se pudo guardar:\n" + query.lastError().text());
        return;
    }
    accept();
}
