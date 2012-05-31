
#ifndef FILE_C_DIALOG_EDIT_METADATA
#define FILE_C_DIALOG_EDIT_METADATA

#include <QDialog>
#include "ui_DialogEditMetadata.h"


class CSong;


/**
 * Boite de dialogue pour afficher et modifier les métadonnées d'un morceau.
 */

class CDialogEditMetadata : public QDialog
{
    Q_OBJECT

public:

    CDialogEditMetadata(CSong * song);
    ~CDialogEditMetadata();

protected slots:

    void apply(void);
    void save(void);
    void reset(void);

protected:

    //void initTabID3v1(void);
    //void initTabID3v2(void);
    //void initTabAPE(void);
    //void initTabXiphComment(void);

private:

    Ui::DialogEditMetadata * m_uiWidget;
    CSong * m_song;
};

#endif // FILE_C_DIALOG_EDIT_METADATA
