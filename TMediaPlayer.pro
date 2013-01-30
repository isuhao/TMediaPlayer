#-------------------------------------------------
#
# Project created by QtCreator 2013-01-29T11:09:08
#
#-------------------------------------------------

QT += core gui sql xml webkit network
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TMediaPlayer
TEMPLATE = app

INCLUDEPATH += /usr/include/taglib

SOURCES += \
    Sources/Utils.cpp \
    Sources/main.cpp \
    Sources/IWidgetCriteria.cpp \
    Sources/IPlayList.cpp \
    Sources/ICriteria.cpp \
    Sources/CWidgetMultiCriterion.cpp \
    Sources/CWidgetLyrics.cpp \
    Sources/CWidgetCriteria.cpp \
    Sources/CStaticPlayList.cpp \
    Sources/CSpecialSpinBox.cpp \
    Sources/CSongTableModel.cpp \
    Sources/CSongTableHeader.cpp \
    Sources/CSongTable.cpp \
    Sources/CSong.cpp \
    Sources/CRatingEditor.cpp \
    Sources/CRatingDelegate.cpp \
    Sources/CRating.cpp \
    Sources/CPlayListView.cpp \
    Sources/CMultiCriterion.cpp \
    Sources/CLyricWiki.cpp \
    Sources/CListModel.cpp \
    Sources/CLibraryFolder.cpp \
    Sources/CLibrary.cpp \
    Sources/CFolder.cpp \
    Sources/CEqualizerPreset.cpp \
    Sources/CDynamicList.cpp \
    Sources/CDRomDrive.cpp \
    Sources/CCriteria.cpp \
    Sources/CCDRomDrive.cpp \
    Sources/CApplication.cpp \
    Sources/Dialog/CDialogRemoveFolder.cpp \
    Sources/Dialog/CDialogPreferencesFolder.cpp \
    Sources/Dialog/CDialogPreferences.cpp \
    Sources/Dialog/CDialogNotifications.cpp \
    Sources/Dialog/CDialogLastPlays.cpp \
    Sources/Dialog/CDialogEqualizer.cpp \
    Sources/Dialog/CDialogEditStaticPlayList.cpp \
    Sources/Dialog/CDialogEditSongs.cpp \
    Sources/Dialog/CDialogEditSong.cpp \
    Sources/Dialog/CDialogEditMetadata.cpp \
    Sources/Dialog/CDialogEditFolder.cpp \
    Sources/Dialog/CDialogEditDynamicList.cpp \
    Sources/Dialog/CDialogCDRomDriveInfos.cpp \
    Sources/Dialog/CDialogAbout.cpp \
    Sources/Importer/CImporterITunes.cpp \
    Sources/Last.fm/ILastFmService.cpp \
    Sources/Last.fm/CUpdateNowPlaying.cpp \
    Sources/Last.fm/CScrobble.cpp \
    Sources/Last.fm/CAuthentication.cpp \
    Sources/MusicBrainz/sha1.cpp \
    Sources/MusicBrainz/CMusicBrainzLookup.cpp \
    Sources/MusicBrainz/base64.cpp

HEADERS += \
    Sources/Utils.hpp \
    Sources/Language.hpp \
    Sources/IWidgetCriteria.hpp \
    Sources/IPlayList.hpp \
    Sources/ICriteria.hpp \
    Sources/CWidgetMultiCriterion.hpp \
    Sources/CWidgetLyrics.hpp \
    Sources/CWidgetCriteria.hpp \
    Sources/CStaticPlayList.hpp \
    Sources/CSpecialSpinBox.hpp \
    Sources/CSongTableModel.hpp \
    Sources/CSongTableHeader.hpp \
    Sources/CSongTable.hpp \
    Sources/CSong.hpp \
    Sources/CSliderStyle.hpp \
    Sources/CRatingEditor.hpp \
    Sources/CRatingDelegate.hpp \
    Sources/CRating.hpp \
    Sources/CPlayListView.hpp \
    Sources/CMultiCriterion.hpp \
    Sources/CLyricWiki.hpp \
    Sources/CListModel.hpp \
    Sources/CLibraryFolder.hpp \
    Sources/CLibrary.hpp \
    Sources/CFolder.hpp \
    Sources/CEqualizerPreset.hpp \
    Sources/CDynamicList.hpp \
    Sources/CDRomDrive.hpp \
    Sources/CCriteria.hpp \
    Sources/CCDRomDrive.hpp \
    Sources/CApplication.hpp \
    Sources/Dialog/CDialogRemoveFolder.hpp \
    Sources/Dialog/CDialogPreferencesFolder.hpp \
    Sources/Dialog/CDialogPreferences.hpp \
    Sources/Dialog/CDialogNotifications.hpp \
    Sources/Dialog/CDialogLastPlays.hpp \
    Sources/Dialog/CDialogEqualizer.hpp \
    Sources/Dialog/CDialogEditStaticPlayList.hpp \
    Sources/Dialog/CDialogEditSongs.hpp \
    Sources/Dialog/CDialogEditSong.hpp \
    Sources/Dialog/CDialogEditMetadata.hpp \
    Sources/Dialog/CDialogEditFolder.hpp \
    Sources/Dialog/CDialogEditDynamicList.hpp \
    Sources/Dialog/CDialogCDRomDriveInfos.hpp \
    Sources/Dialog/CDialogAbout.hpp \
    Sources/Importer/CImporterITunes.hpp \
    Sources/Last.fm/ILastFmService.hpp \
    Sources/Last.fm/CUpdateNowPlaying.hpp \
    Sources/Last.fm/CScrobble.hpp \
    Sources/Last.fm/CAuthentication.hpp \
    Sources/MusicBrainz/sha1.h \
    Sources/MusicBrainz/CMusicBrainzLookup.hpp \
    Sources/MusicBrainz/base64.h

TRANSLATIONS += \
    Lang/TMediaPlayer_fr.ts \
    Lang/TMediaPlayer_en.ts

RESOURCES += \
    TMediaPlayer.qrc

FORMS += \
    Form/WidgetMultiCriterion.ui \
    Form/WidgetCriteria.ui \
    Form/WidgetControl.ui \
    Form/TMediaPlayer.ui \
    Form/ImporterITunesPage3.ui \
    Form/DialogRemoveFolder.ui \
    Form/DialogPreferencesFolder.ui \
    Form/DialogPreferences.ui \
    Form/DialogNotifications.ui \
    Form/DialogLastPlays.ui \
    Form/DialogEqualizer.ui \
    Form/DialogEditStaticPlayList.ui \
    Form/DialogEditSongs.ui \
    Form/DialogEditSong.ui \
    Form/DialogEditMetadata.ui \
    Form/DialogEditFolder.ui \
    Form/DialogEditDynamicPlayList.ui \
    Form/DialogCDRomDriveInfos.ui \
    Form/DialogAbout.ui

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../usr/lib/release/ -lfmodex64
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../usr/lib/debug/ -lfmodex64
else:symbian: LIBS += -lfmodex64
else:unix: LIBS += -L$$PWD/../../../usr/lib/ -lfmodex64

INCLUDEPATH += $$PWD/../../../usr/include
DEPENDPATH += $$PWD/../../../usr/include

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../usr/lib/x86_64-linux-gnu/release/ -ltag
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../usr/lib/x86_64-linux-gnu/debug/ -ltag
else:symbian: LIBS += -ltag
else:unix: LIBS += -L$$PWD/../../../usr/lib/x86_64-linux-gnu/ -ltag

INCLUDEPATH += $$PWD/../../../usr/lib/x86_64-linux-gnu
DEPENDPATH += $$PWD/../../../usr/lib/x86_64-linux-gnu
