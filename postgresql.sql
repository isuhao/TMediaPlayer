
CREATE TABLE folder (
    folder_id SERIAL PRIMARY KEY,
    folder_name VARCHAR(512) NOT NULL,
    folder_parent INTEGER NOT NULL,
    folder_position INTEGER NOT NULL,
    folder_expanded INTEGER NOT NULL
    -- ,UNIQUE (folder_parent, folder_position)
) WITH OIDS;

INSERT INTO folder VALUES (0, '', 0, 1, 1);

CREATE TABLE playlist (
    playlist_id SERIAL PRIMARY KEY,
    playlist_name VARCHAR(512) NOT NULL,
    folder_id INTEGER NOT NULL,
    list_position INTEGER NOT NULL,
    list_columns VARCHAR(512) NOT NULL
    -- ,UNIQUE (folder_id, list_position)
) WITH OIDS;

INSERT INTO playlist (playlist_id, playlist_name, folder_id, list_position, list_columns)
VALUES (0, 'Library', 0, -1, '0:40;1:150;17:60;2+:150;3:150;6:50;9:60;12:50;13:120');

CREATE TABLE dynamic_list (
    dynamic_list_id SERIAL PRIMARY KEY,
    criteria_id INTEGER NOT NULL,
    playlist_id INTEGER NOT NULL,
    UNIQUE (playlist_id)
) WITH OIDS;

CREATE TABLE criteria (
    criteria_id SERIAL PRIMARY KEY,
    dynamic_list_id INTEGER NOT NULL,
    criteria_parent INTEGER NOT NULL,
    criteria_position INTEGER NOT NULL,
    criteria_type INTEGER NOT NULL,
    criteria_condition INTEGER NOT NULL,
    criteria_value1 VARCHAR(512),
    criteria_value2 VARCHAR(512),
    UNIQUE (dynamic_list_id, criteria_parent, criteria_position)
) WITH OIDS;

CREATE TABLE static_list (
    static_list_id SERIAL PRIMARY KEY,
    playlist_id INTEGER NOT NULL,
    UNIQUE (playlist_id)
) WITH OIDS;

CREATE TABLE static_list_song (
    static_list_id INTEGER NOT NULL,
    song_id INTEGER NOT NULL,
    song_position INTEGER NOT NULL,
    UNIQUE (static_list_id, song_position)
);

CREATE TABLE song (
    song_id SERIAL PRIMARY KEY,
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
    song_language CHAR(2) NOT NULL,
    song_lyricist VARCHAR(512) NOT NULL,
    song_compilation INTEGER NOT NULL,
    song_skip_shuffle INTEGER NOT NULL,
    song_play_count INTEGER NOT NULL,
    song_play_time TIMESTAMP,
    song_play_time_utc TIMESTAMP
) WITH OIDS;

CREATE TABLE album (
    album_id SERIAL PRIMARY KEY,
    album_title VARCHAR(512) NOT NULL,
    album_title_sort VARCHAR(512),
    UNIQUE (album_title, album_title_sort)
) WITH OIDS;

INSERT INTO album (album_id, album_title, album_title_sort) VALUES (0, '', '');

CREATE TABLE artist (
    artist_id SERIAL PRIMARY KEY,
    artist_name VARCHAR(512) NOT NULL,
    artist_name_sort VARCHAR(512),
    UNIQUE (artist_name, artist_name_sort)
) WITH OIDS;

INSERT INTO artist (artist_id, artist_name, artist_name_sort) VALUES (0, '', '');

CREATE TABLE genre (
    genre_id SERIAL PRIMARY KEY,
    genre_name VARCHAR(512) NOT NULL UNIQUE
) WITH OIDS;

INSERT INTO genre (genre_id, genre_name) VALUES (0, '');

CREATE TABLE play (
    play_id SERIAL PRIMARY KEY,
    song_id INTEGER NOT NULL,
    play_time TIMESTAMP,
    play_time_utc TIMESTAMP
);


CREATE VIEW albums AS SELECT DISTINCT(album_title) FROM album NATURAL JOIN song WHERE song_id IS NOT NULL;

CREATE VIEW artists AS SELECT DISTINCT(artist_name) FROM artist NATURAL JOIN song WHERE song_id IS NOT NULL;

CREATE VIEW genres AS SELECT DISTINCT(genre_name) FROM genre NATURAL JOIN song WHERE song_id IS NOT NULL;
