
#ifndef FILE_C_LIBRARY
#define FILE_C_LIBRARY

#include "CSongTable.hpp"

class CSong;


class CLibrary : public CSongTable
{
    Q_OBJECT

public:

    explicit CLibrary(CApplication * application);
    
    void deleteSongs(void);
    
public slots:

    virtual void addSong(CSong * song, int pos = -1);
    virtual void addSongs(const QList<CSong *>& songs);
    virtual void removeSong(CSong * song);
    virtual void removeSong(int pos);
};

#endif // FILE_C_LIBRARY
