
CREATE TABLE folder (
    folder_id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    folder_name VARCHAR NOT NULL,
    folder_position INTEGER NOT NULL
);

CREATE TABLE playlist (
    playlist_id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    playlist_name VARCHAR NOT NULL,
    folder_id INTEGER NOT NULL,
    list_position INTEGER NOT NULL,
    list_columns VARCHAR NOT NULL,
    UNIQUE (folder_id, list_position)
);

INSERT INTO "playlist" (playlist_id, playlist_name, folder_id, list_position, list_columns)
VALUES (0, "Library", 0, 0, "1:100;17:50;2+:100;3:100;9:60");

CREATE TABLE dynamic_list (
    "dynamic_list_id" INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    "dynamic_list_union" BOOL NOT NULL,
    "playlist_id" INTEGER NOT NULL,
    UNIQUE ("playlist_id")
);

CREATE TABLE criteria (
    dynamic_list_id INTEGER NOT NULL,
    criteria_parent INTEGER NOT NULL,
    criteria_position INTEGER NOT NULL,
    criteria_union BOOL NOT NULL,
    criteria_type INTEGER NOT NULL,
    criteria_condition INTEGER NOT NULL,
    criteria_value1 VARCHAR NOT NULL,
    criteria_value2 VARCHAR NOT NULL,
    PRIMARY KEY (criteria_parent, criteria_position)
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
    song_filename VARCHAR NOT NULL UNIQUE,
    song_filesize INTEGER NOT NULL,
    song_bitrate INTEGER NOT NULL,
    song_format INTEGER NOT NULL,
    song_channels INTEGER NOT NULL,
    song_duration INTEGER NOT NULL,
    song_creation DATETIME NOT NULL,
    song_modification DATETIME NOT NULL,
    song_title VARCHAR NOT NULL,
    song_title_sort VARCHAR NOT NULL,
    artist_id INTEGER NOT NULL,
    album_id INTEGER NOT NULL,
    album_artist_id INTEGER NOT NULL,
    song_composer VARCHAR NOT NULL,
    song_composer_sort VARCHAR NOT NULL,
    song_year INTEGER NOT NULL,
    song_track_number INTEGER NOT NULL,
    song_track_total INTEGER NOT NULL,
    song_disc_number INTEGER NOT NULL,
    song_disc_total INTEGER NOT NULL,
    genre_id INTEGER NOT NULL,
    song_rating INTEGER NOT NULL,
    song_comments VARCHAR NOT NULL,
    song_lyrics TEXT NOT NULL,
    song_language VARCHAR(2) NOT NULL,
    song_play_count INTEGER NOT NULL,
    song_play_time DATETIME NOT NULL
);

CREATE TABLE album (
    album_id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    album_title VARCHAR NOT NULL UNIQUE,
    album_title_sort VARCHAR
);

CREATE TABLE artist (
    artist_id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    artist_name VARCHAR NOT NULL UNIQUE,
    artist_name_sort VARCHAR
);

CREATE TABLE genre (
    genre_id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    genre_name VARCHAR NOT NULL UNIQUE
);

CREATE TABLE play (
    song_id INTEGER PRIMARY KEY  NOT NULL ,
    play_time DATETIME NOT NULL 
);