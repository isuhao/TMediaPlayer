
#ifndef FILE_CSONG_TABLE_HEADER
#define FILE_CSONG_TABLE_HEADER

#include <QHeaderView>


class QMenu;


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

    void showColTitle       (bool show = true);
    void showColArtist      (bool show = true);
    void showColAlbum       (bool show = true);
    void showColAlbumArtist (bool show = true);
    void showColComposer    (bool show = true);
    void showColYear        (bool show = true);
    void showColTrackNumber (bool show = true);
    void showColDiscNumber  (bool show = true);
    void showColGenre       (bool show = true);
    void showColRating      (bool show = true);
    void showColComments    (bool show = true);
    void showColPlayCount   (bool show = true);
    void showColLastPlayTime(bool show = true);
    void showColFileName    (bool show = true);
    void showColBitRate     (bool show = true);
    void showColFormat      (bool show = true);
    void showColDuration    (bool show = true);

private:

    QMenu * m_contextMenu;
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
};

#endif // FILE_CSONG_TABLE_HEADER
