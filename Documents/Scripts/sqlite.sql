
CREATE TABLE folder (
    folder_id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    folder_name VARCHAR(512) NOT NULL,
    folder_parent INTEGER NOT NULL,
    folder_position INTEGER NOT NULL,
    folder_expanded INTEGER NOT NULL
    -- ,UNIQUE (folder_parent, folder_position)
);

INSERT INTO folder VALUES (0, '', 0, 1, 1);

CREATE TABLE playlist (
    playlist_id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    playlist_name VARCHAR(512) NOT NULL,
    folder_id INTEGER NOT NULL,
    list_position INTEGER NOT NULL,
    list_columns VARCHAR(512) NOT NULL
    -- ,UNIQUE (folder_id, list_position)
);

INSERT INTO playlist (playlist_id, playlist_name, folder_id, list_position, list_columns)
VALUES (0, 'Library', 0, -1, '0:40;1:150;17:60;2+:150;3:150;6:50;9:60;12:50;13:120');

CREATE TABLE dynamic_list (
    dynamic_list_id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    criteria_id INTEGER NOT NULL,
    playlist_id INTEGER NOT NULL,
    auto_update INTEGER NOT NULL,
    only_checked INTEGER NOT NULL,
    UNIQUE (playlist_id)
);

CREATE TABLE criteria (
    criteria_id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    dynamic_list_id INTEGER NOT NULL,
    criteria_parent INTEGER NOT NULL,
    criteria_position INTEGER NOT NULL,
    criteria_type INTEGER NOT NULL,
    criteria_condition INTEGER NOT NULL,
    criteria_value1 VARCHAR(512),
    criteria_value2 VARCHAR(512),
    UNIQUE (dynamic_list_id, criteria_parent, criteria_position)
);

CREATE TABLE static_list (
    static_list_id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    playlist_id INTEGER NOT NULL,
    UNIQUE (playlist_id)
);

CREATE TABLE static_list_song (
    static_list_id INTEGER NOT NULL,
    song_id INTEGER NOT NULL,
    song_position INTEGER NOT NULL,
    UNIQUE (static_list_id, song_position)
);

CREATE TABLE song (
    song_id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    song_filename VARCHAR(512) NOT NULL UNIQUE,
    song_filesize INTEGER NOT NULL,
    song_bitrate INTEGER NOT NULL,
    song_sample_rate INTEGER NOT NULL,
    song_format INTEGER NOT NULL,
    song_channels INTEGER NOT NULL,
    song_duration INTEGER NOT NULL,
    song_creation TIMESTAMP NOT NULL,
    song_modification TIMESTAMP NOT NULL,
    song_enabled INTEGER NOT NULL,
    song_title VARCHAR(512) NOT NULL,
    song_title_sort VARCHAR(512) NOT NULL,
    song_subtitle VARCHAR(512) NOT NULL,
    song_grouping VARCHAR(512) NOT NULL,
    artist_id INTEGER NOT NULL,
    album_id INTEGER NOT NULL,
    album_artist_id INTEGER NOT NULL,
    song_composer VARCHAR(512) NOT NULL,
    song_composer_sort VARCHAR(512) NOT NULL,
    song_year INTEGER NOT NULL,
    song_track_number INTEGER NOT NULL,
    song_track_count INTEGER NOT NULL,
    song_disc_number INTEGER NOT NULL,
    song_disc_count INTEGER NOT NULL,
    genre_id INTEGER NOT NULL,
    song_rating INTEGER NOT NULL,
    song_comments TEXT NOT NULL,
    song_bpm INTEGER NOT NULL,
    song_lyrics TEXT NOT NULL,
    song_language VARCHAR(2) NOT NULL,
    song_lyricist VARCHAR(512) NOT NULL,
    song_compilation INTEGER NOT NULL,
    song_skip_shuffle INTEGER NOT NULL,
    song_play_count INTEGER NOT NULL,
    song_play_time TIMESTAMP,
    song_play_time_utc TIMESTAMP,
    song_track_gain FLOAT NOT NULL,
    song_track_peak FLOAT NOT NULL,
    song_album_gain FLOAT NOT NULL,
    song_album_peak FLOAT NOT NULL
);

CREATE TABLE album (
    album_id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    album_title VARCHAR(512) NOT NULL,
    album_title_sort VARCHAR(512),
    UNIQUE (album_title, album_title_sort)
);

INSERT INTO album (album_id, album_title, album_title_sort) VALUES (0, '', '');

CREATE TABLE artist (
    artist_id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    artist_name VARCHAR(512) NOT NULL,
    artist_name_sort VARCHAR(512),
    UNIQUE (artist_name, artist_name_sort)
);

INSERT INTO artist (artist_id, artist_name, artist_name_sort) VALUES (0, '', '');

CREATE TABLE genre (
    genre_id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    genre_name VARCHAR(512) NOT NULL UNIQUE
);

INSERT INTO genre (genre_id, genre_name) VALUES (0, '');

CREATE TABLE play (
    play_id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    song_id INTEGER NOT NULL,
    play_time TIMESTAMP,
    play_time_utc TIMESTAMP
);

CREATE TABLE libpath (
    path_id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    path_location VARCHAR(512) NOT NULL UNIQUE,
    path_keep_organized INTEGER NOT NULL,
    path_format VARCHAR(512) NOT NULL,
    path_format_items TEXT
);

CREATE TABLE equalizer (
    equalizer_id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    equalizer_name VARCHAR(512) NOT NULL UNIQUE,
    equalizer_val0 FLOAT NOT NULL DEFAULT 1.0,
    equalizer_val1 FLOAT NOT NULL DEFAULT 1.0,
    equalizer_val2 FLOAT NOT NULL DEFAULT 1.0,
    equalizer_val3 FLOAT NOT NULL DEFAULT 1.0,
    equalizer_val4 FLOAT NOT NULL DEFAULT 1.0,
    equalizer_val5 FLOAT NOT NULL DEFAULT 1.0,
    equalizer_val6 FLOAT NOT NULL DEFAULT 1.0,
    equalizer_val7 FLOAT NOT NULL DEFAULT 1.0,
    equalizer_val8 FLOAT NOT NULL DEFAULT 1.0,
    equalizer_val9 FLOAT NOT NULL DEFAULT 1.0
);


CREATE VIEW albums AS SELECT DISTINCT(album_title) FROM album NATURAL JOIN song WHERE song_id IS NOT NULL;

CREATE VIEW artists AS SELECT DISTINCT(artist_name) FROM artist NATURAL JOIN song WHERE song_id IS NOT NULL;

CREATE VIEW genres AS SELECT DISTINCT(genre_name) FROM genre NATURAL JOIN song WHERE song_id IS NOT NULL;
