
#ifndef FILE_C_PLAYLIST_MODEL
#define FILE_C_PLAYLIST_MODEL

#include <QStandardItemModel>
#include <QStringList>


class CApplication;


class CPlayListModel : public QStandardItemModel
{
    Q_OBJECT

public:

    explicit CPlayListModel(CApplication * application);
    virtual ~CPlayListModel();

    bool dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex& parent);
    QStringList mimeTypes(void) const;
    Qt::DropActions supportedDropActions() const;

private:

    CApplication * m_application;
};

#endif // FILE_C_PLAYLIST_MODEL
