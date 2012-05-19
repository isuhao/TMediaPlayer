
#ifndef FILE_CSONGTABLEMODEL
#define FILE_CSONGTABLEMODEL

#include <QAbstractTableModel>
#include <QList>


class CSong;
class QMouseEvent;


class CSongTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:

    CSongTableModel(const QList<CSong *>& data = QList<CSong *>(), QObject * parent = NULL);

    void setData(const QList<CSong *>& data);
    inline const QList<CSong *>& getData() const { return m_data; };

    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    int columnCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    //void sort(int column, Qt::SortOrder order);

    void insertRow(CSong * song, int pos = -1);
    void removeRow(int pos);
    void clear(void);

    CSong * getSong(const QModelIndex& index) const;

private:

    QList<CSong *> m_data;
};

#endif // FILE_CSONGTABLEMODEL
