#pragma once

#include <QDateTime>
#include <QSet>
#include <QSqlQueryModel>
#include <QTimeZone>

// QSqlQueryModel que muestra en hora local las columnas guardadas en UTC (Permisos_Temporales y
// Bitacora_Cambios usan UTC_TIMESTAMP para que el central y las sucursales coincidan sin importar su zona).
class ModeloUtc : public QSqlQueryModel {
public:
    using QSqlQueryModel::QSqlQueryModel;

    void setColumnasUtc(const QSet<int> &columnas) { m_columnas = columnas; }

    QVariant data(const QModelIndex &indice, int rol = Qt::DisplayRole) const override {
        const QVariant valor = QSqlQueryModel::data(indice, rol);
        if (rol != Qt::DisplayRole || !m_columnas.contains(indice.column()))
            return valor;
        QDateTime fecha = valor.toDateTime();
        if (!fecha.isValid())
            return valor;
        fecha.setTimeZone(QTimeZone::utc());   // el valor viene sin zona: se interpreta como UTC
        return fecha.toLocalTime().toString("dd/MM/yyyy HH:mm");
    }

private:
    QSet<int> m_columnas;
};
