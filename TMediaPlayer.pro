#-------------------------------------------------
# TMediaPlayer
#
# Copyright (C) 2012-2014 Teddy Michel
#-------------------------------------------------
# Dependencies :
#
# - Qt (including Webkit)
# - FMODEx
# - TagLib
#-------------------------------------------------

QT += core gui sql xml network webkit
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TMediaPlayer
TEMPLATE = app

SOURCES += \
    Sources/Utils.cpp \
    Sources/main.cpp \
    Sources/CCDRomDrive.cpp \
    Sources/CCriterion.cpp \
    Sources/CDRomDrive.cpp \
    Sources/CDynamicList.cpp \
    Sources/CEqualizerPreset.cpp \
    Sources/CFolder.cpp \
    Sources/CLibrary.cpp \
    Sources/CLibraryFolder.cpp \
    Sources/CLibraryModel.cpp \
    Sources/CLibraryView.cpp \
    Sources/CLyricWiki.cpp \
    Sources/CMainWindow.cpp \
    Sources/CMediaManager.cpp \
    Sources/CMediaTableHeader.cpp \
    Sources/CMediaTableModel.cpp \
    Sources/CMediaTableView.cpp \
    Sources/CMultiCriteria.cpp \
    Sources/CQueuePlayList.cpp \
    Sources/CRating.cpp \
    Sources/CRatingDelegate.cpp \
    Sources/CRatingEditor.cpp \
    Sources/CSong.cpp \
    Sources/CSpecialSpinBox.cpp \
    Sources/CStaticList.cpp \
    Sources/CWidgetCriterion.cpp \
    Sources/CWidgetLineEditMultiple.cpp \
    Sources/CWidgetLyrics.cpp \
    Sources/CWidgetMultiCriteria.cpp \
    Sources/ICriterion.cpp \
    Sources/IMediaTable.cpp \
    Sources/IPlayList.cpp \
    Sources/IWidgetCriterion.cpp \
    Sources/MusicBrainz/base64.cpp \
    Sources/MusicBrainz/CMusicBrainzLookup.cpp \
    Sources/MusicBrainz/sha1.cpp \
    Sources/Last.fm/CAuthentication.cpp \
    Sources/Last.fm/CGetRecentTracks.cpp \
    Sources/Last.fm/CScrobble.cpp \
    Sources/Last.fm/CUpdateNowPlaying.cpp \
    Sources/Last.fm/ILastFmService.cpp \
    Sources/Importer/CImporterITunes.cpp \
    Sources/Dialog/CDialogAbout.cpp \
    Sources/Dialog/CDialogCDRomDriveInfos.cpp \
    Sources/Dialog/CDialogEditDynamicList.cpp \
    Sources/Dialog/CDialogEditFolder.cpp \
    Sources/Dialog/CDialogEditMetadata.cpp \
    Sources/Dialog/CDialogEditSong.cpp \
    Sources/Dialog/CDialogEditSongs.cpp \
    Sources/Dialog/CDialogEditStaticPlayList.cpp \
    Sources/Dialog/CDialogEditXiphComment.cpp \
    Sources/Dialog/CDialogEqualizer.cpp \
    Sources/Dialog/CDialogLastPlays.cpp \
    Sources/Dialog/CDialogNotifications.cpp \
    Sources/Dialog/CDialogPreferences.cpp \
    Sources/Dialog/CDialogPreferencesFolder.cpp \
    Sources/Dialog/CDialogRemoveFolder.cpp

HEADERS += \
    Sources/Utils.hpp \
    Sources/CCDRomDrive.hpp \
    Sources/CCriterion.hpp \
    Sources/CDRomDrive.hpp \
    Sources/CDynamicList.hpp \
    Sources/CEqualizerPreset.hpp \
    Sources/CFolder.hpp \
    Sources/CLibrary.hpp \
    Sources/CLibraryFolder.hpp \
    Sources/CLibraryModel.hpp \
    Sources/CLibraryView.hpp \
    Sources/CLyricWiki.hpp \
    Sources/CMainWindow.hpp \
    Sources/CMediaManager.hpp \
    Sources/CMediaTableHeader.hpp \
    Sources/CMediaTableItem.hpp \
    Sources/CMediaTableModel.hpp \
    Sources/CMediaTableView.hpp \
    Sources/CMultiCriteria.hpp \
    Sources/CQueuePlayList.hpp \
    Sources/CRating.hpp \
    Sources/CRatingDelegate.hpp \
    Sources/CRatingEditor.hpp \
    Sources/CSliderStyle.hpp \
    Sources/CSong.hpp \
    Sources/CSpecialSpinBox.hpp \
    Sources/CStaticList.hpp \
    Sources/CWidgetCriterion.hpp \
    Sources/CWidgetLineEditMultiple.hpp \
    Sources/CWidgetLyrics.hpp \
    Sources/CWidgetMultiCriteria.hpp \
    Sources/ICriterion.hpp \
    Sources/IMediaTable.hpp \
    Sources/IPlayList.hpp \
    Sources/IWidgetCriterion.hpp \
    Sources/Language.hpp \
    Sources/nullptr.h \
    Sources/MusicBrainz/base64.h \
    Sources/MusicBrainz/CMusicBrainzLookup.hpp \
    Sources/MusicBrainz/sha1.h \
    Sources/Last.fm/CAuthentication.hpp \
    Sources/Last.fm/CGetRecentTracks.hpp \
    Sources/Last.fm/CScrobble.hpp \
    Sources/Last.fm/CUpdateNowPlaying.hpp \
    Sources/Last.fm/ILastFmService.hpp \
    Sources/Importer/CImporterITunes.hpp \
    Sources/Dialog/CDialogAbout.hpp \
    Sources/Dialog/CDialogCDRomDriveInfos.hpp \
    Sources/Dialog/CDialogEditDynamicList.hpp \
    Sources/Dialog/CDialogEditFolder.hpp \
    Sources/Dialog/CDialogEditMetadata.hpp \
    Sources/Dialog/CDialogEditSong.hpp \
    Sources/Dialog/CDialogEditSongs.hpp \
    Sources/Dialog/CDialogEditStaticPlayList.hpp \
    Sources/Dialog/CDialogEditXiphComment.hpp \
    Sources/Dialog/CDialogEqualizer.hpp \
    Sources/Dialog/CDialogLastPlays.hpp \
    Sources/Dialog/CDialogNotifications.hpp \
    Sources/Dialog/CDialogPreferences.hpp \
    Sources/Dialog/CDialogPreferencesFolder.hpp \
    Sources/Dialog/CDialogRemoveFolder.hpp

TRANSLATIONS += \
    Lang/TMediaPlayer_fr.ts \
    Lang/TMediaPlayer_en.ts

RESOURCES += \
    TMediaPlayer.qrc

FORMS += \
    Form/TMediaPlayer.ui \
    Form/DialogAbout.ui \
    Form/DialogCDRomDriveInfos.ui \
    Form/DialogEditDynamicPlayList.ui \
    Form/DialogEditFolder.ui \
    Form/DialogEditMetadata.ui \
    Form/DialogEditMetadataXiph.ui \
    Form/DialogEditSong.ui \
    Form/DialogEditSongs.ui \
    Form/DialogEditStaticPlayList.ui \
    Form/DialogEqualizer.ui \
    Form/DialogLastPlays.ui \
    Form/DialogNotifications.ui \
    Form/DialogPreferences.ui \
    Form/DialogPreferencesFolder.ui \
    Form/DialogRemoveFolder.ui \
    Form/ImporterITunesPage3.ui \
    Form/WidgetControl.ui \
    Form/WidgetCriterion.ui \
    Form/WidgetMultiCriteria.ui

unix: INCLUDEPATH += /usr/include/taglib

unix: LIBS += -lfmodex64
unix: LIBS += -ltag
