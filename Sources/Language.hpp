/*
Copyright (C) 2012 Teddy Michel

This file is part of TMediaPlayer.

TMediaPlayer is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

TMediaPlayer is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with TMediaPlayer. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef FILE_LANGUAGE
#define FILE_LANGUAGE

#include <QString>
#include <QStringList>


enum TLanguage
{
    LangUnknown    =  0, ///< Langue inconnue.
    LangEnglish    =  1, ///< Anglais.
    LangFrench     =  2, ///< Français.
    LangGerman     =  3, ///< Allemand.
    LangItalian    =  4, ///< Italien.
    LangRussian    =  5, ///< Russe.
    LangSpanish    =  6, ///< Espagnol.
    LangChinese    =  7, ///< Chinois.
    LangHindi      =  8, ///< Hindi.
    LangPortuguese =  9, ///< Portugais.
    LangArabic     = 10  ///< Arabe.
};

inline TLanguage getLanguageFromInteger(int language);
inline QString getLanguageName(TLanguage language);
inline QStringList getLanguageList();
inline TLanguage getLanguageForISO2Code(const QString& code);
inline TLanguage getLanguageForISO3Code(const QString& code);
inline QString getISO2CodeForLanguage(TLanguage language);
inline QString getISO3CodeForLanguage(TLanguage language);


inline TLanguage getLanguageFromInteger(int language)
{
    switch (language)
    {
        default:
        case  0: return LangUnknown   ;
        case  1: return LangEnglish   ;
        case  2: return LangFrench    ;
        case  3: return LangGerman    ;
        case  4: return LangItalian   ;
        case  5: return LangRussian   ;
        case  6: return LangSpanish   ;
        case  7: return LangChinese   ;
        case  8: return LangHindi     ;
        case  9: return LangPortuguese;
        case 10: return LangArabic    ;
    }
}


/**
 * Retourne le nom correspondant à une langue.
 *
 * \param language Langue.
 * \return Nom de la langue.
 */

inline QString getLanguageName(TLanguage language)
{
    switch (language)
    {
        default:
        case LangUnknown   : return QObject::tr("Unknown", "Unknown language");
        case LangEnglish   : return QObject::tr("English"                    );
        case LangFrench    : return QObject::tr("French"                     );
        case LangGerman    : return QObject::tr("German"                     );
        case LangItalian   : return QObject::tr("Italian"                    );
        case LangRussian   : return QObject::tr("Russian"                    );
        case LangSpanish   : return QObject::tr("Spanish"                    );
        case LangChinese   : return QObject::tr("Chinese"                    );
        case LangHindi     : return QObject::tr("Hindi"                      );
        case LangPortuguese: return QObject::tr("Portuguese"                 );
        case LangArabic    : return QObject::tr("Arabic"                     );
    }
}


/**
 * Retourne la liste des langues.
 *
 * \return Liste des langues.
 */

inline QStringList getLanguageList()
{
    QStringList langList;

    langList << getLanguageName(LangUnknown   );
    langList << getLanguageName(LangEnglish   );
    langList << getLanguageName(LangFrench    );
    langList << getLanguageName(LangGerman    );
    langList << getLanguageName(LangItalian   );
    langList << getLanguageName(LangRussian   );
    langList << getLanguageName(LangSpanish   );
    langList << getLanguageName(LangChinese   );
    langList << getLanguageName(LangHindi     );
    langList << getLanguageName(LangPortuguese);
    langList << getLanguageName(LangArabic    );

    return langList;
}


/**
 * Retourne langue correspondant à un code ISO 639-1.
 *
 * \param code Code ISO 639-1.
 */

inline TLanguage getLanguageForISO2Code(const QString& code)
{
    const QString codeLower = code.toLower();

    if (codeLower == "en") return LangEnglish   ;
    if (codeLower == "fr") return LangFrench    ;
    if (codeLower == "de") return LangGerman    ;
    if (codeLower == "it") return LangItalian   ;
    if (codeLower == "ru") return LangRussian   ;
    if (codeLower == "es") return LangSpanish   ;
    if (codeLower == "zh") return LangChinese   ;
    if (codeLower == "hi") return LangHindi     ;
    if (codeLower == "pt") return LangPortuguese;
    if (codeLower == "ar") return LangArabic    ;

    return LangUnknown;
}


/**
 * Retourne langue correspondant à un code ISO 639-2.
 *
 * \param code Code ISO 639-2.
 */

inline TLanguage getLanguageForISO3Code(const QString& code)
{
    const QString codeLower = code.toLower();

    if (codeLower == "eng") return LangEnglish   ;
    if (codeLower == "fra") return LangFrench    ;
    if (codeLower == "deu") return LangGerman    ;
    if (codeLower == "ita") return LangItalian   ;
    if (codeLower == "rus") return LangRussian   ;
    if (codeLower == "spa") return LangSpanish   ;
    if (codeLower == "zho") return LangChinese   ;
    if (codeLower == "hin") return LangHindi     ;
    if (codeLower == "por") return LangPortuguese;
    if (codeLower == "ara") return LangArabic    ;

    return LangUnknown;
}


/**
 * Retourne le code ISO 639-1 correspondant à une langue.
 */

inline QString getISO2CodeForLanguage(TLanguage language)
{
    switch (language)
    {
        default:
        case LangUnknown   : return "xx";
        case LangEnglish   : return "en";
        case LangFrench    : return "fr";
        case LangGerman    : return "de";
        case LangItalian   : return "it";
        case LangRussian   : return "ru";
        case LangSpanish   : return "es";
        case LangChinese   : return "zh";
        case LangHindi     : return "hi";
        case LangPortuguese: return "pt";
        case LangArabic    : return "ar";
    }
}


/**
 * Retourne le code ISO 639-2 correspondant à une langue.
 */

inline QString getISO3CodeForLanguage(TLanguage language)
{
    switch (language)
    {
        default:
        case LangUnknown   : return "xxx";
        case LangEnglish   : return "eng";
        case LangFrench    : return "fra";
        case LangGerman    : return "deu";
        case LangItalian   : return "ita";
        case LangRussian   : return "rus";
        case LangSpanish   : return "spa";
        case LangChinese   : return "zho";
        case LangHindi     : return "hin";
        case LangPortuguese: return "por";
        case LangArabic    : return "ara";
    }
}

#endif // FILE_LANGUAGE
