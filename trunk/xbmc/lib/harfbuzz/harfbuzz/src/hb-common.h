/*
 * Copyright © 2007,2008,2009  Red Hat, Inc.
 * Copyright © 2011  Google, Inc.
 *
 *  This is part of HarfBuzz, a text shaping library.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and its documentation for any purpose, provided that the
 * above copyright notice and the following two paragraphs appear in
 * all copies of this software.
 *
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES
 * ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN
 * IF THE COPYRIGHT HOLDER HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *
 * THE COPYRIGHT HOLDER SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING,
 * BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND THE COPYRIGHT HOLDER HAS NO OBLIGATION TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 *
 * Red Hat Author(s): Behdad Esfahbod
 * Google Author(s): Behdad Esfahbod
 */

#ifndef HB_COMMON_H
#define HB_COMMON_H

# ifdef __cplusplus
#  define HB_BEGIN_DECLS	extern "C" {
#  define HB_END_DECLS		}
# else /* !__cplusplus */
#  define HB_BEGIN_DECLS
#  define HB_END_DECLS
# endif /* !__cplusplus */

HB_BEGIN_DECLS


#ifdef _MSC_VER
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef signed short int16_t;
typedef unsigned short uint16_t;
typedef signed int int32_t;
typedef unsigned int uint32_t;
typedef signed long long int64_t;
typedef unsigned long long uint64_t;
#else
#include <stdint.h>
#endif

typedef int hb_bool_t;

typedef uint32_t hb_codepoint_t;
typedef int32_t hb_position_t;
typedef uint32_t hb_mask_t;

typedef union _hb_var_int_t {
  uint32_t u32;
  int32_t i32;
  uint16_t u16[2];
  int16_t i16[2];
  uint8_t u8[4];
  int8_t i8[4];
} hb_var_int_t;


/* hb_tag_t */

typedef uint32_t hb_tag_t;

#define HB_TAG(a,b,c,d) ((hb_tag_t)((((uint8_t)(a))<<24)|(((uint8_t)(b))<<16)|(((uint8_t)(c))<<8)|((uint8_t)(d))))

#define HB_TAG_NONE HB_TAG(0,0,0,0)

hb_tag_t hb_tag_from_string (const char *s);


/* hb_direction_t */

typedef enum _hb_direction_t {
  HB_DIRECTION_INVALID = -1,
  HB_DIRECTION_LTR = 0,
  HB_DIRECTION_RTL,
  HB_DIRECTION_TTB,
  HB_DIRECTION_BTT
} hb_direction_t;

hb_direction_t
hb_direction_from_string (const char *str);

const char *
hb_direction_to_string (hb_direction_t direction);

#define HB_DIRECTION_IS_HORIZONTAL(dir)	((((unsigned int) (dir)) & ~1U) == 0)
#define HB_DIRECTION_IS_VERTICAL(dir)	((((unsigned int) (dir)) & ~1U) == 2)
#define HB_DIRECTION_IS_FORWARD(dir)	((((unsigned int) (dir)) & ~2U) == 0)
#define HB_DIRECTION_IS_BACKWARD(dir)	((((unsigned int) (dir)) & ~2U) == 1)
#define HB_DIRECTION_REVERSE(dir)	((hb_direction_t) (((unsigned int) (dir)) ^ 1))


/* hb_language_t */

typedef struct _hb_language_t *hb_language_t;

hb_language_t
hb_language_from_string (const char *str);

const char *
hb_language_to_string (hb_language_t language);


/* hb_unicode_general_category_t */

typedef enum
{
  HB_UNICODE_GENERAL_CATEGORY_CONTROL,			/* Cc */
  HB_UNICODE_GENERAL_CATEGORY_FORMAT,			/* Cf */
  HB_UNICODE_GENERAL_CATEGORY_UNASSIGNED,		/* Cn */
  HB_UNICODE_GENERAL_CATEGORY_PRIVATE_USE,		/* Co */
  HB_UNICODE_GENERAL_CATEGORY_SURROGATE,		/* Cs */
  HB_UNICODE_GENERAL_CATEGORY_LOWERCASE_LETTER,		/* Ll */
  HB_UNICODE_GENERAL_CATEGORY_MODIFIER_LETTER,		/* Lm */
  HB_UNICODE_GENERAL_CATEGORY_OTHER_LETTER,		/* Lo */
  HB_UNICODE_GENERAL_CATEGORY_TITLECASE_LETTER,		/* Lt */
  HB_UNICODE_GENERAL_CATEGORY_UPPERCASE_LETTER,		/* Lu */
  HB_UNICODE_GENERAL_CATEGORY_COMBINING_MARK,		/* Mc */
  HB_UNICODE_GENERAL_CATEGORY_ENCLOSING_MARK,		/* Me */
  HB_UNICODE_GENERAL_CATEGORY_NON_SPACING_MARK,		/* Mn */
  HB_UNICODE_GENERAL_CATEGORY_DECIMAL_NUMBER,		/* Nd */
  HB_UNICODE_GENERAL_CATEGORY_LETTER_NUMBER,		/* Nl */
  HB_UNICODE_GENERAL_CATEGORY_OTHER_NUMBER,		/* No */
  HB_UNICODE_GENERAL_CATEGORY_CONNECT_PUNCTUATION,	/* Pc */
  HB_UNICODE_GENERAL_CATEGORY_DASH_PUNCTUATION,		/* Pd */
  HB_UNICODE_GENERAL_CATEGORY_CLOSE_PUNCTUATION,	/* Pe */
  HB_UNICODE_GENERAL_CATEGORY_FINAL_PUNCTUATION,	/* Pf */
  HB_UNICODE_GENERAL_CATEGORY_INITIAL_PUNCTUATION,	/* Pi */
  HB_UNICODE_GENERAL_CATEGORY_OTHER_PUNCTUATION,	/* Po */
  HB_UNICODE_GENERAL_CATEGORY_OPEN_PUNCTUATION,		/* Ps */
  HB_UNICODE_GENERAL_CATEGORY_CURRENCY_SYMBOL,		/* Sc */
  HB_UNICODE_GENERAL_CATEGORY_MODIFIER_SYMBOL,		/* Sk */
  HB_UNICODE_GENERAL_CATEGORY_MATH_SYMBOL,		/* Sm */
  HB_UNICODE_GENERAL_CATEGORY_OTHER_SYMBOL,		/* So */
  HB_UNICODE_GENERAL_CATEGORY_LINE_SEPARATOR,		/* Zl */
  HB_UNICODE_GENERAL_CATEGORY_PARAGRAPH_SEPARATOR,	/* Zp */
  HB_UNICODE_GENERAL_CATEGORY_SPACE_SEPARATOR		/* Zs */
} hb_unicode_general_category_t;


/* hb_script_t */

/* http://unicode.org/iso15924/ */
typedef enum
{
  HB_SCRIPT_COMMON                  = HB_TAG ('Z','y','y','y'),
  HB_SCRIPT_INHERITED               = HB_TAG ('Z','i','n','h'),
  HB_SCRIPT_ARABIC                  = HB_TAG ('A','r','a','b'),
  HB_SCRIPT_ARMENIAN                = HB_TAG ('A','r','m','n'),
  HB_SCRIPT_BENGALI                 = HB_TAG ('B','e','n','g'),
  HB_SCRIPT_BOPOMOFO                = HB_TAG ('B','o','p','o'),
  HB_SCRIPT_CHEROKEE                = HB_TAG ('C','h','e','r'),
  HB_SCRIPT_COPTIC                  = HB_TAG ('C','o','p','t'),
  HB_SCRIPT_CYRILLIC                = HB_TAG ('C','y','r','l'),
  HB_SCRIPT_DESERET                 = HB_TAG ('D','s','r','t'),
  HB_SCRIPT_DEVANAGARI              = HB_TAG ('D','e','v','a'),
  HB_SCRIPT_ETHIOPIC                = HB_TAG ('E','t','h','i'),
  HB_SCRIPT_GEORGIAN                = HB_TAG ('G','e','o','r'),
  HB_SCRIPT_GOTHIC                  = HB_TAG ('G','o','t','h'),
  HB_SCRIPT_GREEK                   = HB_TAG ('G','r','e','k'),
  HB_SCRIPT_GUJARATI                = HB_TAG ('G','u','j','r'),
  HB_SCRIPT_GURMUKHI                = HB_TAG ('G','u','r','u'),
  HB_SCRIPT_HAN                     = HB_TAG ('H','a','n','i'),
  HB_SCRIPT_HANGUL                  = HB_TAG ('H','a','n','g'),
  HB_SCRIPT_HEBREW                  = HB_TAG ('H','e','b','r'),
  HB_SCRIPT_HIRAGANA                = HB_TAG ('H','i','r','a'),
  HB_SCRIPT_KANNADA                 = HB_TAG ('K','n','d','a'),
  HB_SCRIPT_KATAKANA                = HB_TAG ('K','a','n','a'),
  HB_SCRIPT_KHMER                   = HB_TAG ('K','h','m','r'),
  HB_SCRIPT_LAO                     = HB_TAG ('L','a','o','o'),
  HB_SCRIPT_LATIN                   = HB_TAG ('L','a','t','n'),
  HB_SCRIPT_MALAYALAM               = HB_TAG ('M','l','y','m'),
  HB_SCRIPT_MONGOLIAN               = HB_TAG ('M','o','n','g'),
  HB_SCRIPT_MYANMAR                 = HB_TAG ('M','y','m','r'),
  HB_SCRIPT_OGHAM                   = HB_TAG ('O','g','a','m'),
  HB_SCRIPT_OLD_ITALIC              = HB_TAG ('I','t','a','l'),
  HB_SCRIPT_ORIYA                   = HB_TAG ('O','r','y','a'),
  HB_SCRIPT_RUNIC                   = HB_TAG ('R','u','n','r'),
  HB_SCRIPT_SINHALA                 = HB_TAG ('S','i','n','h'),
  HB_SCRIPT_SYRIAC                  = HB_TAG ('S','y','r','c'),
  HB_SCRIPT_TAMIL                   = HB_TAG ('T','a','m','l'),
  HB_SCRIPT_TELUGU                  = HB_TAG ('T','e','l','u'),
  HB_SCRIPT_THAANA                  = HB_TAG ('T','h','a','a'),
  HB_SCRIPT_THAI                    = HB_TAG ('T','h','a','i'),
  HB_SCRIPT_TIBETAN                 = HB_TAG ('T','i','b','t'),
  HB_SCRIPT_CANADIAN_ABORIGINAL     = HB_TAG ('C','a','n','s'),
  HB_SCRIPT_YI                      = HB_TAG ('Y','i','i','i'),
  HB_SCRIPT_TAGALOG                 = HB_TAG ('T','g','l','g'),
  HB_SCRIPT_HANUNOO                 = HB_TAG ('H','a','n','o'),
  HB_SCRIPT_BUHID                   = HB_TAG ('B','u','h','d'),
  HB_SCRIPT_TAGBANWA                = HB_TAG ('T','a','g','b'),

  /* Unicode-4.0 additions */
  HB_SCRIPT_BRAILLE                 = HB_TAG ('B','r','a','i'),
  HB_SCRIPT_CYPRIOT                 = HB_TAG ('C','p','r','t'),
  HB_SCRIPT_LIMBU                   = HB_TAG ('L','i','m','b'),
  HB_SCRIPT_OSMANYA                 = HB_TAG ('O','s','m','a'),
  HB_SCRIPT_SHAVIAN                 = HB_TAG ('S','h','a','w'),
  HB_SCRIPT_LINEAR_B                = HB_TAG ('L','i','n','b'),
  HB_SCRIPT_TAI_LE                  = HB_TAG ('T','a','l','e'),
  HB_SCRIPT_UGARITIC                = HB_TAG ('U','g','a','r'),

  /* Unicode-4.1 additions */
  HB_SCRIPT_NEW_TAI_LUE             = HB_TAG ('T','a','l','u'),
  HB_SCRIPT_BUGINESE                = HB_TAG ('B','u','g','i'),
  HB_SCRIPT_GLAGOLITIC              = HB_TAG ('G','l','a','g'),
  HB_SCRIPT_TIFINAGH                = HB_TAG ('T','f','n','g'),
  HB_SCRIPT_SYLOTI_NAGRI            = HB_TAG ('S','y','l','o'),
  HB_SCRIPT_OLD_PERSIAN             = HB_TAG ('X','p','e','o'),
  HB_SCRIPT_KHAROSHTHI              = HB_TAG ('K','h','a','r'),

  /* Unicode-5.0 additions */
  HB_SCRIPT_UNKNOWN                 = HB_TAG ('Z','z','z','z'),
  HB_SCRIPT_BALINESE                = HB_TAG ('B','a','l','i'),
  HB_SCRIPT_CUNEIFORM               = HB_TAG ('X','s','u','x'),
  HB_SCRIPT_PHOENICIAN              = HB_TAG ('P','h','n','x'),
  HB_SCRIPT_PHAGS_PA                = HB_TAG ('P','h','a','g'),
  HB_SCRIPT_NKO                     = HB_TAG ('N','k','o','o'),

  /* Unicode-5.1 additions */
  HB_SCRIPT_KAYAH_LI                = HB_TAG ('K','a','l','i'),
  HB_SCRIPT_LEPCHA                  = HB_TAG ('L','e','p','c'),
  HB_SCRIPT_REJANG                  = HB_TAG ('R','j','n','g'),
  HB_SCRIPT_SUNDANESE               = HB_TAG ('S','u','n','d'),
  HB_SCRIPT_SAURASHTRA              = HB_TAG ('S','a','u','r'),
  HB_SCRIPT_CHAM                    = HB_TAG ('C','h','a','m'),
  HB_SCRIPT_OL_CHIKI                = HB_TAG ('O','l','c','k'),
  HB_SCRIPT_VAI                     = HB_TAG ('V','a','i','i'),
  HB_SCRIPT_CARIAN                  = HB_TAG ('C','a','r','i'),
  HB_SCRIPT_LYCIAN                  = HB_TAG ('L','y','c','i'),
  HB_SCRIPT_LYDIAN                  = HB_TAG ('L','y','d','i'),

  /* Unicode-5.2 additions */
  HB_SCRIPT_AVESTAN                 = HB_TAG ('A','v','s','t'),
  HB_SCRIPT_BAMUM                   = HB_TAG ('B','a','m','u'),
  HB_SCRIPT_EGYPTIAN_HIEROGLYPHS    = HB_TAG ('E','g','y','p'),
  HB_SCRIPT_IMPERIAL_ARAMAIC        = HB_TAG ('A','r','m','i'),
  HB_SCRIPT_INSCRIPTIONAL_PAHLAVI   = HB_TAG ('P','h','l','i'),
  HB_SCRIPT_INSCRIPTIONAL_PARTHIAN  = HB_TAG ('P','r','t','i'),
  HB_SCRIPT_JAVANESE                = HB_TAG ('J','a','v','a'),
  HB_SCRIPT_KAITHI                  = HB_TAG ('K','t','h','i'),
  HB_SCRIPT_LISU                    = HB_TAG ('L','i','s','u'),
  HB_SCRIPT_MEETEI_MAYEK            = HB_TAG ('M','t','e','i'),
  HB_SCRIPT_OLD_SOUTH_ARABIAN       = HB_TAG ('S','a','r','b'),
  HB_SCRIPT_OLD_TURKIC              = HB_TAG ('O','r','k','h'),
  HB_SCRIPT_SAMARITAN               = HB_TAG ('S','a','m','r'),
  HB_SCRIPT_TAI_THAM                = HB_TAG ('L','a','n','a'),
  HB_SCRIPT_TAI_VIET                = HB_TAG ('T','a','v','t'),

  /* Unicode-6.0 additions */
  HB_SCRIPT_BATAK                   = HB_TAG ('B','a','t','k'),
  HB_SCRIPT_BRAHMI                  = HB_TAG ('B','r','a','h'),
  HB_SCRIPT_MANDAIC                 = HB_TAG ('M','a','n','d'),

  /* No script set */
  HB_SCRIPT_INVALID                 = HB_TAG_NONE
} hb_script_t;


/* Script functions */

hb_script_t
hb_script_from_iso15924_tag (hb_tag_t tag);

/* suger for tag_from_string() then script_from_iso15924_tag */
hb_script_t
hb_script_from_string (const char *s);

hb_tag_t
hb_script_to_iso15924_tag (hb_script_t script);

hb_direction_t
hb_script_get_horizontal_direction (hb_script_t script);


/* User data */

typedef struct _hb_user_data_key_t {
  char unused;
} hb_user_data_key_t;

typedef void (*hb_destroy_func_t) (void *user_data);


HB_END_DECLS

#endif /* HB_COMMON_H */
