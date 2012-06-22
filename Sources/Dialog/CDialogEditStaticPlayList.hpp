
#ifndef FILE_C_DIALOG_EDIT_STATIC_PLAYLIST
#define FILE_C_DIALOG_EDIT_STATIC_PLAYLIST

#include <QDialog>
#include "ui_DialogEditStaticPlayList.h"


class CStaticPlayList;
class CApplication;
class CFolder;
class CSong;


/**
 * Boite de dialogue permettant la création ou la modification d'une liste de lecture statique.
 */

class CDialogEditStaticPlayList : public QDialog
{
    Q_OBJECT

public:

    CDialogEditStaticPlayList(CStaticPlayList * playList, CApplication * application, CFolder * folder = NULL, const QList<CSong *>& songs = QList<CSong *>());
    virtual ~CDialogEditStaticPlayList();

protected slots:

    void save(void);

private:
    
    Ui::DialogEditStaticPlayList * m_uiWidget;
    CStaticPlayList * m_playList; ///< Pointeur sur la liste de lecture.
    CApplication * m_application; ///< Pointeur sur l'application.
    CFolder * m_folder;       ///< Pointeur sur le dossier contenant la liste de lecture.
    QList<CSong *> m_songs;       ///< Liste de morceaux à ajouter à la liste.
};

#endif // FILE_C_DIALOG_EDIT_STATIC_PLAYLIST
