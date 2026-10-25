#include "categoriadialog.h"
#include "ui_categoriadialog.h"

#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

CategoriaDialog::CategoriaDialog(QWidget *parent) : QDialog(parent), ui(new Ui::CategoriaDialog) {
    ui->setupUi(this);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &CategoriaDialog::guardar);
}

CategoriaDialog::~CategoriaDialog() {
    delete ui;
}

void CategoriaDialog::setCategoriaId(int id) {
    m_idCategoria = id;
    cargarCategoria();
}

void CategoriaDialog::cargarCategoria() {
    if (m_idCategoria < 0)
        return;

    QSqlQuery query;
    query.prepare("SELECT nombre, tipo, descripcion FROM Categorias WHERE id_categoria = :id");
    query.bindValue(":id", m_idCategoria);
    if (!query.exec() || !query.next())
        return;

    ui->lineNombre->setText(query.value(0).toString());
    ui->lineTipo->setText(query.value(1).toString());
    ui->textDescripcion->setPlainText(query.value(2).toString());
}

void CategoriaDialog::guardar() {
    if (ui->lineNombre->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Validación", "El nombre es obligatorio.");
        return;
    }

    QSqlQuery query;
    if (m_idCategoria < 0) {
        query.prepare("INSERT INTO Categorias (nombre, tipo, descripcion) VALUES (:nombre, :tipo, :descripcion)");
    } else {
        query.prepare("UPDATE Categorias SET nombre = :nombre, tipo = :tipo, descripcion = :descripcion "
                      "WHERE id_categoria = :id");
        query.bindValue(":id", m_idCategoria);
    }

    query.bindValue(":nombre", ui->lineNombre->text().trimmed());
    query.bindValue(":tipo", ui->lineTipo->text().trimmed());
    query.bindValue(":descripcion", ui->textDescripcion->toPlainText());

    if (!query.exec()) {
        QMessageBox::critical(this, "Error", "No se pudo guardar la categoría:\n" + query.lastError().text());
        return;
    }

    accept();
}
