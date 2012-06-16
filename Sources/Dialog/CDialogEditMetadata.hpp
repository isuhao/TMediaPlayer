
#ifndef FILE_C_DIALOG_EDIT_METADATA
#define FILE_C_DIALOG_EDIT_METADATA

#include <QDialog>
#include "ui_DialogEditMetadata.h"


class CSong;
class QStandardItemModel;

namespace TagLib
{
    namespace ID3v1 { class Tag; }
    namespace ID3v2 { class Tag; }
    namespace APE   { class Tag; }
    namespace Ogg   { class XiphComment; }
}


/**
 * Boite de dialogue pour afficher et modifier les métadonnées d'un morceau.
 */

class CDialogEditMetadata : public QDialog
{
    Q_OBJECT

public:

    explicit CDialogEditMetadata(CSong * song);
    virtual ~CDialogEditMetadata();

protected slots:

    void apply(void);
    void save(void);
    void reset(void);

protected:

    void initTagID3v1(TagLib::ID3v1::Tag * tags);
    void initTagID3v2(TagLib::ID3v2::Tag * tags);
    void initTagAPE(TagLib::APE::Tag * tags);
    void initTagXiphComment(TagLib::Ogg::XiphComment * tags);

private:

    Ui::DialogEditMetadata * m_uiWidget;
    QStandardItemModel * m_modelID3v2Text;
    QStandardItemModel * m_modelID3v2Comments;
    CSong * m_song;
};

#endif // FILE_C_DIALOG_EDIT_METADATA
