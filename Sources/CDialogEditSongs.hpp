
#ifndef FILE_C_DIALOG_EDIT_SONGS
#define FILE_C_DIALOG_EDIT_SONGS

#include <QDialog>
#include "CSongTableModel.hpp"
#include "ui_DialogEditSongs.h"


/**
 * Boite de dialogue pour modifier les informations de plusieurs morceaux.
 */

class CDialogEditSongs : public QDialog
{
    Q_OBJECT

public:

    explicit CDialogEditSongs(QList<CSongTableItem *> songItemList, QWidget * parent = NULL);
    virtual ~CDialogEditSongs();

protected slots:

    void apply(void);
    void save(void);

    void onTitleChange(const QString& title);
    void onTitleSortChange(const QString& title);
    void onArtistChange(const QString& artistName);
    void onArtistSortChange(const QString& artistName);
    void onAlbumChange(const QString& albumTitle);
    void onAlbumSortChange(const QString& albumTitle);

    void onTitleChecked(bool checked);
    void onTitleSortChecked(bool checked);
    void onArtistChecked(bool checked);
    void onArtistSortChecked(bool checked);
    void onAlbumChecked(bool checked);
    void onAlbumSortChecked(bool checked);

private:
    
    Ui::DialogEditSongs * m_uiWidget;
    QList<CSongTableItem *> m_songItemList;
};

#endif // FILE_C_DIALOG_EDIT_SONGS
