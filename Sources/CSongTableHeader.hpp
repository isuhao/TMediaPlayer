
#ifndef FILE_CSONG_TABLE_HEADER
#define FILE_CSONG_TABLE_HEADER

#include <QHeaderView>
#include "CSongTable.hpp"


class QMenu;

/* (=> Parse error at "T_CREATE_SLOT" lors du deuxième appel, entrainant une recompilation à chaque fois.)
#define T_CREATE_SLOT(colType) \
    inline void show##colType(bool show = true) { emit columnShown(CSongTable::##colType, show); }
*/

class CSongTableHeader : public QHeaderView
{
    Q_OBJECT

    friend class CSongTable;

public:

    CSongTableHeader(QWidget * parent = NULL);
    ~CSongTableHeader();

signals:

    void columnShown(int col, bool shown);

protected:

    void contextMenuEvent(QContextMenuEvent * event);

protected slots:

    inline void showColTitle           (bool show = true) { emit columnShown(CSongTable::ColTitle           , show); }
    inline void showColArtist          (bool show = true) { emit columnShown(CSongTable::ColArtist          , show); }
    inline void showColAlbum           (bool show = true) { emit columnShown(CSongTable::ColAlbum           , show); }
    inline void showColAlbumArtist     (bool show = true) { emit columnShown(CSongTable::ColAlbumArtist     , show); }
    inline void showColComposer        (bool show = true) { emit columnShown(CSongTable::ColComposer        , show); }
    inline void showColYear            (bool show = true) { emit columnShown(CSongTable::ColYear            , show); }
    inline void showColTrackNumber     (bool show = true) { emit columnShown(CSongTable::ColTrackNumber     , show); }
    inline void showColDiscNumber      (bool show = true) { emit columnShown(CSongTable::ColDiscNumber      , show); }
    inline void showColGenre           (bool show = true) { emit columnShown(CSongTable::ColGenre           , show); }
    inline void showColRating          (bool show = true) { emit columnShown(CSongTable::ColRating          , show); }
    inline void showColComments        (bool show = true) { emit columnShown(CSongTable::ColComments        , show); }
    inline void showColPlayCount       (bool show = true) { emit columnShown(CSongTable::ColPlayCount       , show); }
    inline void showColLastPlayTime    (bool show = true) { emit columnShown(CSongTable::ColLastPlayTime    , show); }
    inline void showColFileName        (bool show = true) { emit columnShown(CSongTable::ColFileName        , show); }
    inline void showColBitRate         (bool show = true) { emit columnShown(CSongTable::ColBitRate         , show); }
    inline void showColFormat          (bool show = true) { emit columnShown(CSongTable::ColFormat          , show); }
    inline void showColDuration        (bool show = true) { emit columnShown(CSongTable::ColDuration        , show); }
    inline void showColSampleRate      (bool show = true) { emit columnShown(CSongTable::ColSampleRate      , show); }
    inline void showColCreationDate    (bool show = true) { emit columnShown(CSongTable::ColCreationDate    , show); }
    inline void showColModificationDate(bool show = true) { emit columnShown(CSongTable::ColModificationDate, show); }
    inline void showColChannels        (bool show = true) { emit columnShown(CSongTable::ColChannels        , show); }
    inline void showColFileSize        (bool show = true) { emit columnShown(CSongTable::ColFileSize        , show); }

/*
    T_CREATE_SLOT(ColTitle)
    T_CREATE_SLOT(ColArtist)
    T_CREATE_SLOT(ColAlbum)
    T_CREATE_SLOT(ColAlbumArtist)
    T_CREATE_SLOT(ColComposer)
    T_CREATE_SLOT(ColYear)
    T_CREATE_SLOT(ColTrackNumber)
    T_CREATE_SLOT(ColDiscNumber)
    T_CREATE_SLOT(ColGenre)
    T_CREATE_SLOT(ColRating)
    T_CREATE_SLOT(ColComments)
    T_CREATE_SLOT(ColPlayCount)
    T_CREATE_SLOT(ColLastPlayTime)
    T_CREATE_SLOT(ColFileName)
    T_CREATE_SLOT(ColBitRate)
    T_CREATE_SLOT(ColFormat)
    T_CREATE_SLOT(ColDuration)
    T_CREATE_SLOT(ColSampleRate)
    T_CREATE_SLOT(ColCreationDate)
    T_CREATE_SLOT(ColModificationDate)
    T_CREATE_SLOT(ColChannels)
    T_CREATE_SLOT(ColFileSize)
*/

/*
    void showColTitle           (bool show = true);
    void showColArtist          (bool show = true);
    void showColAlbum           (bool show = true);
    void showColAlbumArtist     (bool show = true);
    void showColComposer        (bool show = true);
    void showColYear            (bool show = true);
    void showColTrackNumber     (bool show = true);
    void showColDiscNumber      (bool show = true);
    void showColGenre           (bool show = true);
    void showColRating          (bool show = true);
    void showColComments        (bool show = true);
    void showColPlayCount       (bool show = true);
    void showColLastPlayTime    (bool show = true);
    void showColFileName        (bool show = true);
    void showColBitRate         (bool show = true);
    void showColFormat          (bool show = true);
    void showColDuration        (bool show = true);
    void showColSampleRate      (bool show = true);
    void showColCreationDate    (bool show = true);
    void showColModificationDate(bool show = true);
*/
private:

    QMenu * m_contextMenu;
    QAction * m_actionShowCol[CSongTable::ColNumber];
/*
    QAction * m_actColTitle;
    QAction * m_actColArtist;
    QAction * m_actColAlbum;
    QAction * m_actColAlbumArtist;
    QAction * m_actColComposer;
    QAction * m_actColYear;
    QAction * m_actColTrackNumber;
    QAction * m_actColDiscNumber;
    QAction * m_actColGenre;
    QAction * m_actColRating;
    QAction * m_actColComments;
    QAction * m_actColPlayCount;
    QAction * m_actColLastPlayTime;
    QAction * m_actColFileName;
    QAction * m_actColBitRate;
    QAction * m_actColFormat;
    QAction * m_actColDuration;
    QAction * m_actColSampleRate;
    QAction * m_actColCreationDate;
    QAction * m_actColModificationDate;
*/
};

#endif // FILE_CSONG_TABLE_HEADER
