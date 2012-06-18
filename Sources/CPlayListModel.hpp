
#ifndef FILE_C_PLAYLIST_MODEL
#define FILE_C_PLAYLIST_MODEL

#include <QStandardItemModel>
#include <QStringList>


class CApplication;
class CSong;


/**
 * Modèle permettant de stocker les listes de lecture et les dossiers.
 * Doit être utilisé avec la classe CPlayListView.
 *
 * \todo Les listes de lecture et les dossiers devraient être gérées uniquement par cette classe.
 */

class CPlayListModel : public QStandardItemModel
{
    Q_OBJECT

public:

    explicit CPlayListModel(CApplication * application);
    virtual ~CPlayListModel();

    //int rowCount(const QModelIndex& parent = QModelIndex()) const;
    //int columnCount(const QModelIndex& parent = QModelIndex()) const;
    //QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    //bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);

    // Glisser-déposer
    //Qt::ItemFlags flags(const QModelIndex& index) const;
    //QMimeData * mimeData(const QModelIndexList& indexes) const;
    bool dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex& parent);
    QStringList mimeTypes(void) const;
    Qt::DropActions supportedDropActions() const;

private:

    QList<CSong *> decodeData(const QByteArray& encodedData) const;

    CApplication * m_application; ///< Pointeur sur l'application.
};

#endif // FILE_C_PLAYLIST_MODEL
