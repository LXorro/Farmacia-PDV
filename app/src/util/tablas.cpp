#include "tablas.h"

#include <QAbstractItemModel>
#include <QApplication>
#include <QDate>
#include <QEvent>
#include <QLabel>
#include <QLocale>
#include <QMetaType>
#include <QPointer>
#include <QPainter>
#include <QSet>
#include <QStyledItemDelegate>
#include <QTableView>

namespace {

const QSet<QString> kColumnasMoneda = {"Total", "Total devuelto", "Precio compra", "Precio venta",
                                       "Precio unitario", "Subtotal", "Monto"};
const QSet<QString> kColumnasCantidad = {"Cantidad", "Disponible", "Devuelto"};
const QSet<QString> kColumnasTextoEnum = {"Modalidad", "Tipo de reembolso", "Rol", "Método"};

const QLocale kLocalMoneda(QLocale::Spanish, QLocale::Mexico);

struct Colores {
    QColor texto;
    QColor fondo;
};

// Valores de los ENUM del esquema agrupados por significado.
bool coloresDeEstado(const QString &valor, Colores *c) {
    static const QSet<QString> bien = {"activo", "activa", "disponible", "pagado", "completado",
                                       "completada", "aplicado", "aprobada", "sellado"};
    static const QSet<QString> aviso = {"parcial", "pendiente", "solicitada", "suspendido", "abierto"};
    static const QSet<QString> mal = {"inactivo", "inactiva", "agotado", "caducado", "cancelado",
                                      "vencido", "rechazada", "dañado"};
    static const QSet<QString> neutro = {"devuelto"};
    const QString v = valor.toLower();
    if (bien.contains(v))        *c = {QColor("#0b7560"), QColor("#d9f0ea")};
    else if (aviso.contains(v))  *c = {QColor("#8a5a00"), QColor("#fdecc8")};
    else if (mal.contains(v))    *c = {QColor("#a53224"), QColor("#f8dad6")};
    else if (neutro.contains(v)) *c = {QColor("#4a5560"), QColor("#e3e7ea")};
    else return false;
    return true;
}

QString bonito(QString s) {
    s.replace('_', ' ');
    if (!s.isEmpty())
        s[0] = s[0].toUpper();
    return s;
}

QString encabezado(const QModelIndex &idx) {
    return idx.model()->headerData(idx.column(), Qt::Horizontal).toString();
}

bool esNumerico(const QVariant &v) {
    switch (v.metaType().id()) {
    case QMetaType::Int: case QMetaType::UInt: case QMetaType::LongLong: case QMetaType::ULongLong:
    case QMetaType::Double: case QMetaType::Float:
        return true;
    default:
        return false;
    }
}

bool esColumnaEstado(const QString &h) {
    return h.contains("estado", Qt::CaseInsensitive);
}

class FormatoDelegate : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    void paint(QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &idx) const override {
        const QString h = encabezado(idx);
        Colores c;
        if (!esColumnaEstado(h) || !coloresDeEstado(idx.data(Qt::DisplayRole).toString(), &c)) {
            QStyledItemDelegate::paint(p, opt, idx);
            return;
        }

        QStyleOptionViewItem o = opt;
        initStyleOption(&o, idx);
        const QString texto = o.text;
        o.text.clear();
        QStyle *estilo = o.widget ? o.widget->style() : QApplication::style();
        estilo->drawControl(QStyle::CE_ItemViewItem, &o, p, o.widget);   // fondo / selección

        QFont f = o.font;
        f.setBold(true);
        f.setPointSizeF(f.pointSizeF() - 1);
        const QFontMetrics fm(f);
        const int alto = fm.height() + 6;
        const QRect r(o.rect.left() + 6, o.rect.center().y() - alto / 2 + 1,
                      fm.horizontalAdvance(texto) + 18, alto);
        const bool seleccionada = o.state & QStyle::State_Selected;

        p->save();
        p->setRenderHint(QPainter::Antialiasing);
        p->setPen(Qt::NoPen);
        p->setBrush(seleccionada ? QColor(255, 255, 255, 240) : c.fondo);
        p->drawRoundedRect(r, alto / 2.0, alto / 2.0);
        p->setPen(c.texto);
        p->setFont(f);
        p->drawText(r, Qt::AlignCenter, texto);
        p->restore();
    }

    QSize sizeHint(const QStyleOptionViewItem &opt, const QModelIndex &idx) const override {
        QSize s = QStyledItemDelegate::sizeHint(opt, idx);
        if (esColumnaEstado(encabezado(idx)))
            s.rwidth() += 24;   // holgura para la píldora
        s.rheight() += 4;
        return s;
    }

protected:
    void initStyleOption(QStyleOptionViewItem *o, const QModelIndex &idx) const override {
        QStyledItemDelegate::initStyleOption(o, idx);
        const QString h = encabezado(idx);
        const QVariant v = idx.data(Qt::DisplayRole);

        if (kColumnasMoneda.contains(h)) {
            bool ok = false;
            const double d = v.toDouble(&ok);
            if (ok)
                o->text = kLocalMoneda.toCurrencyString(d);
            o->displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
        } else if (kColumnasCantidad.contains(h) || esNumerico(v)) {
            o->displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
        }

        const int tipo = v.metaType().id();
        if (tipo == QMetaType::QDateTime)
            o->text = v.toDateTime().toString("dd/MM/yyyy HH:mm");
        else if (tipo == QMetaType::QDate)
            o->text = v.toDate().toString("dd/MM/yyyy");

        if (h == "Caducidad" && tipo == QMetaType::QDate) {
            const qint64 dias = QDate::currentDate().daysTo(v.toDate());
            if (dias < 0)
                o->palette.setColor(QPalette::Text, QColor("#c0392b"));
            else if (dias <= 30)
                o->palette.setColor(QPalette::Text, QColor("#b9770e"));
            if (dias <= 30)
                o->font.setBold(true);
        }

        if (h == "Receta") {
            o->text = v.toInt() ? "Sí" : "No";
            o->displayAlignment = Qt::AlignLeft | Qt::AlignVCenter;
        }

        if (esColumnaEstado(h) || kColumnasTextoEnum.contains(h))
            o->text = bonito(o->text);
    }
};

// Etiqueta centrada sobre el viewport que se muestra solo si no hay filas.
// Guarda QPointer a todo: al destruir la tabla, el modelo emite modelReset
// mientras los widgets ya se están borrando, y tocarlos ahí cierra la app.
class AvisoVacio : public QObject {
public:
    AvisoVacio(QTableView *vista, const QString &texto) : QObject(vista), m_modelo(vista->model()) {
        m_label = new QLabel(texto, vista->viewport());
        m_label->setAlignment(Qt::AlignCenter);
        m_label->setStyleSheet("color: #8a9399; font-size: 15px; background: transparent;");
        m_label->setAttribute(Qt::WA_TransparentForMouseEvents);
        vista->viewport()->installEventFilter(this);
        conectar();
        actualizar();
    }

    bool eventFilter(QObject *obj, QEvent *e) override {
        if (e->type() == QEvent::Resize && m_label && obj == m_label->parentWidget())
            m_label->setGeometry(m_label->parentWidget()->rect());
        return QObject::eventFilter(obj, e);
    }

private:
    void conectar() {
        if (!m_modelo)
            return;
        connect(m_modelo, &QAbstractItemModel::modelReset, this, [this] { actualizar(); });
        connect(m_modelo, &QAbstractItemModel::layoutChanged, this, [this] { actualizar(); });
        connect(m_modelo, &QAbstractItemModel::rowsInserted, this, [this] { actualizar(); });
        connect(m_modelo, &QAbstractItemModel::rowsRemoved, this, [this] { actualizar(); });
    }

    void actualizar() {
        if (!m_label || !m_label->parentWidget() || !m_modelo)
            return;
        m_label->setGeometry(m_label->parentWidget()->rect());
        const bool vacia = m_modelo->rowCount() == 0;
        m_label->setVisible(vacia);
        if (vacia)
            m_label->raise();
    }

    QPointer<QAbstractItemModel> m_modelo;
    QPointer<QLabel> m_label;
};

}  // namespace

void Tablas::aplicar(QTableView *vista, const QString &textoVacio) {
    vista->setItemDelegate(new FormatoDelegate(vista));
    new AvisoVacio(vista, textoVacio);
}
