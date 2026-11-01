#include "asignaciondialog.h"
#include "ui_asignaciondialog.h"
#include "../database/sesionactual.h"

#include <QDate>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

AsignacionDialog::AsignacionDialog(QWidget *parent) : QDialog(parent), ui(new Ui::AsignacionDialog) {
    ui->setupUi(this);
    ui->dateDesde->setDisplayFormat("dd/MM/yyyy");
    ui->dateHasta->setDisplayFormat("dd/MM/yyyy");
    ui->dateDesde->setDate(QDate::currentDate());
    ui->dateHasta->setDate(QDate::currentDate().addDays(7));
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &AsignacionDialog::guardar);
    cargarCombos();
}

AsignacionDialog::~AsignacionDialog() {
    delete ui;
}

void AsignacionDialog::cargarCombos() {
    QSqlQuery empleados(
        "SELECT e.id_empleado, e.nombre, e.apellido_paterno, s.nombre, e.id_sucursal "
        "FROM Empleados e JOIN Sucursales s ON s.id_sucursal = e.id_sucursal "
        "WHERE e.estado = 'activo' ORDER BY e.nombre");
    while (empleados.next()) {
        const QString etiqueta = QString("%1 %2 (%3)").arg(empleados.value(1).toString(),
                                                           empleados.value(2).toString(), empleados.value(3).toString());
        ui->comboEmpleado->addItem(etiqueta, empleados.value(0));
        // Se guarda la sucursal propia de cada empleado para impedir "asignarlo" a donde ya pertenece.
        ui->comboEmpleado->setItemData(ui->comboEmpleado->count() - 1, empleados.value(4), Qt::UserRole + 1);
    }

    QSqlQuery sucursales("SELECT id_sucursal, nombre FROM Sucursales WHERE rol = 'sucursal' AND estado = 'activa' ORDER BY nombre");
    while (sucursales.next())
        ui->comboSucursal->addItem(sucursales.value(1).toString(), sucursales.value(0));
}

void AsignacionDialog::guardar() {
    if (ui->comboEmpleado->currentIndex() < 0 || ui->comboSucursal->currentIndex() < 0) {
        QMessageBox::warning(this, "Validación", "Selecciona un empleado y una sucursal.");
        return;
    }
    if (ui->comboEmpleado->currentData(Qt::UserRole + 1) == ui->comboSucursal->currentData()) {
        QMessageBox::warning(this, "Validación", "El empleado ya pertenece a esa sucursal.");
        return;
    }
    if (ui->dateHasta->date() < ui->dateDesde->date()) {
        QMessageBox::warning(this, "Validación", "La fecha final no puede ser anterior a la inicial.");
        return;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO Asignaciones_Temporales (id_empleado, id_sucursal, fecha_inicio, fecha_fin, id_autoriza) "
                  "VALUES (:empleado, :sucursal, :desde, :hasta, :autoriza)");
    query.bindValue(":empleado", ui->comboEmpleado->currentData());
    query.bindValue(":sucursal", ui->comboSucursal->currentData());
    query.bindValue(":desde", ui->dateDesde->date());
    query.bindValue(":hasta", ui->dateHasta->date());
    const int idUsuario = SesionActual::instance().idUsuario();
    query.bindValue(":autoriza", idUsuario >= 0 ? QVariant(idUsuario) : QVariant(QMetaType(QMetaType::Int)));
    if (!query.exec()) {
        QMessageBox::critical(this, "Error", "No se pudo guardar la asignación:\n" + query.lastError().text());
        return;
    }
    accept();
}
