
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
    song_sample_rate INTEGER NOT NULL,
    song_encoder VARCHAR NOT NULL,
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
    album_title VARCHAR NOT NULL,
    album_title_sort VARCHAR,
    UNIQUE (album_title, album_title_sort)
);

CREATE TABLE artist (
    artist_id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    artist_name VARCHAR NOT NULL,
    artist_name_sort VARCHAR,
    UNIQUE (artist_name, artist_name_sort)
);

CREATE TABLE genre (
    genre_id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    genre_name VARCHAR NOT NULL UNIQUE
);

INSERT INTO genre (genre_id, genre_name) VALUES (  0, "Blues");
INSERT INTO genre (genre_id, genre_name) VALUES (  1, "Classic rock");
INSERT INTO genre (genre_id, genre_name) VALUES (  2, "Country");
INSERT INTO genre (genre_id, genre_name) VALUES (  3, "Dance");
INSERT INTO genre (genre_id, genre_name) VALUES (  4, "Disco");
INSERT INTO genre (genre_id, genre_name) VALUES (  5, "Funk");
INSERT INTO genre (genre_id, genre_name) VALUES (  6, "Grunge");
INSERT INTO genre (genre_id, genre_name) VALUES (  7, "Hip-Hop");
INSERT INTO genre (genre_id, genre_name) VALUES (  8, "Jazz");
INSERT INTO genre (genre_id, genre_name) VALUES (  9, "Metal");
INSERT INTO genre (genre_id, genre_name) VALUES ( 10, "New Age");
INSERT INTO genre (genre_id, genre_name) VALUES ( 11, "Oldies");
INSERT INTO genre (genre_id, genre_name) VALUES ( 12, "Other");
INSERT INTO genre (genre_id, genre_name) VALUES ( 13, "Pop");
INSERT INTO genre (genre_id, genre_name) VALUES ( 14, "R'n'B");
INSERT INTO genre (genre_id, genre_name) VALUES ( 15, "Rap");
INSERT INTO genre (genre_id, genre_name) VALUES ( 16, "Reggae");
INSERT INTO genre (genre_id, genre_name) VALUES ( 17, "Rock");
INSERT INTO genre (genre_id, genre_name) VALUES ( 18, "Techno");
INSERT INTO genre (genre_id, genre_name) VALUES ( 19, "Industrial");
INSERT INTO genre (genre_id, genre_name) VALUES ( 20, "Alternative");
INSERT INTO genre (genre_id, genre_name) VALUES ( 21, "Ska");
INSERT INTO genre (genre_id, genre_name) VALUES ( 22, "Death metal");
INSERT INTO genre (genre_id, genre_name) VALUES ( 23, "Pranks");
INSERT INTO genre (genre_id, genre_name) VALUES ( 24, "Soundtrack");
INSERT INTO genre (genre_id, genre_name) VALUES ( 25, "Euro-techno");
INSERT INTO genre (genre_id, genre_name) VALUES ( 26, "Ambient");
INSERT INTO genre (genre_id, genre_name) VALUES ( 27, "Trip-hop");
INSERT INTO genre (genre_id, genre_name) VALUES ( 28, "Vocal");
INSERT INTO genre (genre_id, genre_name) VALUES ( 29, "Jazz-Funk");
INSERT INTO genre (genre_id, genre_name) VALUES ( 30, "Fusion");
INSERT INTO genre (genre_id, genre_name) VALUES ( 31, "Trance");
INSERT INTO genre (genre_id, genre_name) VALUES ( 32, "Classical");
INSERT INTO genre (genre_id, genre_name) VALUES ( 33, "Instrumental");
INSERT INTO genre (genre_id, genre_name) VALUES ( 34, "Acid");
INSERT INTO genre (genre_id, genre_name) VALUES ( 35, "House");
INSERT INTO genre (genre_id, genre_name) VALUES ( 36, "Game");
INSERT INTO genre (genre_id, genre_name) VALUES ( 37, "Sound clip");
INSERT INTO genre (genre_id, genre_name) VALUES ( 38, "Gospel");
INSERT INTO genre (genre_id, genre_name) VALUES ( 39, "Noise");
INSERT INTO genre (genre_id, genre_name) VALUES ( 40, "Alternative rock");
INSERT INTO genre (genre_id, genre_name) VALUES ( 41, "Bass");
INSERT INTO genre (genre_id, genre_name) VALUES ( 42, "Soul");
INSERT INTO genre (genre_id, genre_name) VALUES ( 43, "Punk");
INSERT INTO genre (genre_id, genre_name) VALUES ( 44, "Space");
INSERT INTO genre (genre_id, genre_name) VALUES ( 45, "Meditative");
INSERT INTO genre (genre_id, genre_name) VALUES ( 46, "Instrumental pop");
INSERT INTO genre (genre_id, genre_name) VALUES ( 47, "Instrumental rock");
INSERT INTO genre (genre_id, genre_name) VALUES ( 48, "Ethnic");
INSERT INTO genre (genre_id, genre_name) VALUES ( 49, "Gothic");
INSERT INTO genre (genre_id, genre_name) VALUES ( 50, "Darkwave");
INSERT INTO genre (genre_id, genre_name) VALUES ( 51, "Techno-Industrial");
INSERT INTO genre (genre_id, genre_name) VALUES ( 52, "Electronic");
INSERT INTO genre (genre_id, genre_name) VALUES ( 53, "Pop-Folk");
INSERT INTO genre (genre_id, genre_name) VALUES ( 54, "Eurodance");
INSERT INTO genre (genre_id, genre_name) VALUES ( 55, "Dream");
INSERT INTO genre (genre_id, genre_name) VALUES ( 56, "Southern Rock");
INSERT INTO genre (genre_id, genre_name) VALUES ( 57, "Comedy");
INSERT INTO genre (genre_id, genre_name) VALUES ( 58, "Cult");
INSERT INTO genre (genre_id, genre_name) VALUES ( 59, "Gangsta");
INSERT INTO genre (genre_id, genre_name) VALUES ( 60, "Top 40");
INSERT INTO genre (genre_id, genre_name) VALUES ( 61, "Christian rap");
INSERT INTO genre (genre_id, genre_name) VALUES ( 62, "Pop/Funk");
INSERT INTO genre (genre_id, genre_name) VALUES ( 63, "Jungle");
INSERT INTO genre (genre_id, genre_name) VALUES ( 64, "Native US");
INSERT INTO genre (genre_id, genre_name) VALUES ( 65, "Cabaret");
INSERT INTO genre (genre_id, genre_name) VALUES ( 66, "New Wave");
INSERT INTO genre (genre_id, genre_name) VALUES ( 67, "Psychadelic");
INSERT INTO genre (genre_id, genre_name) VALUES ( 68, "Rave");
INSERT INTO genre (genre_id, genre_name) VALUES ( 69, "Showtunes");
INSERT INTO genre (genre_id, genre_name) VALUES ( 70, "Trailer");
INSERT INTO genre (genre_id, genre_name) VALUES ( 71, "Lo-Fi");
INSERT INTO genre (genre_id, genre_name) VALUES ( 72, "Tribal");
INSERT INTO genre (genre_id, genre_name) VALUES ( 73, "Acid Punk");
INSERT INTO genre (genre_id, genre_name) VALUES ( 74, "Acid Jazz");
INSERT INTO genre (genre_id, genre_name) VALUES ( 75, "Polka");
INSERT INTO genre (genre_id, genre_name) VALUES ( 76, "Retro");
INSERT INTO genre (genre_id, genre_name) VALUES ( 77, "Musical");
INSERT INTO genre (genre_id, genre_name) VALUES ( 78, "Rock & Roll");
INSERT INTO genre (genre_id, genre_name) VALUES ( 79, "Hard Rock");
INSERT INTO genre (genre_id, genre_name) VALUES ( 80, "Folk");
INSERT INTO genre (genre_id, genre_name) VALUES ( 81, "Folk-Rock");
INSERT INTO genre (genre_id, genre_name) VALUES ( 82, "National Folk");
INSERT INTO genre (genre_id, genre_name) VALUES ( 83, "Swing");
INSERT INTO genre (genre_id, genre_name) VALUES ( 84, "Fast Fusion");
INSERT INTO genre (genre_id, genre_name) VALUES ( 85, "Bebob");
INSERT INTO genre (genre_id, genre_name) VALUES ( 86, "Latin");
INSERT INTO genre (genre_id, genre_name) VALUES ( 87, "Revival");
INSERT INTO genre (genre_id, genre_name) VALUES ( 88, "Celtic");
INSERT INTO genre (genre_id, genre_name) VALUES ( 89, "Bluegrass");
INSERT INTO genre (genre_id, genre_name) VALUES ( 90, "Avantgarde");
INSERT INTO genre (genre_id, genre_name) VALUES ( 91, "Gothic Rock");
INSERT INTO genre (genre_id, genre_name) VALUES ( 92, "Progressive Rock");
INSERT INTO genre (genre_id, genre_name) VALUES ( 93, "Psychedelic Rock");
INSERT INTO genre (genre_id, genre_name) VALUES ( 94, "Symphonic Rock");

CREATE TABLE play (
    song_id INTEGER PRIMARY KEY  NOT NULL ,
    play_time DATETIME NOT NULL 
);
