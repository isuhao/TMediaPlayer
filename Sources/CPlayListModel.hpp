
#ifndef FILE_CPLAYLIST_MODEL
#define FILE_CPLAYLIST_MODEL

#include <QStandardItemModel>
#include <QStringList>


class CApplication;


class CPlayListModel : public QStandardItemModel
{
    Q_OBJECT

public:

    CPlayListModel(CApplication * application);
    ~CPlayListModel();

    bool dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex& parent);
    QStringList mimeTypes(void) const;
    Qt::DropActions supportedDropActions() const;

private:

    CApplication * m_application; ///< Pointeur sur l'application.
};

#endif // FILE_CPLAYLIST_MODEL
