/*
    File:    main_lexem_info.h
    Created: 22 March 2018 at 18:55 Moscow time
    Author:  Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/

#ifndef MAIN_LEXEM_INFO_H
#define MAIN_LEXEM_INFO_H
#include <cstddef>
enum class Main_lexem_code : uint16_t {
    None,                  Unknown,               Id,
    Kw_action,             Kw_class_members,      Kw_codes,
    Kw_codes_type,         Kw_comments,           Kw_delimiters,
    Kw_ident_name,         Kw_idents,             Kw_keywords,
    Kw_multilined,         Kw_nested,             Kw_newline_is_lexem,
    Kw_numbers,            Kw_scaner_name,        Kw_single_lined,
    Kw_strings,            Kw_token_fields,       Kw_header_additions,
    Kw_impl_additions,     Kw_lexem_info_name,    Comma,
    Colon,                 Opened_fig_brack,      Closed_fig_brack,
    String,                Arrow,                 M_Kw_action,
    M_Kw_class_members,    M_Kw_codes,            M_Kw_codes_type,
    M_Kw_comments,         M_Kw_delimiters,       M_Kw_ident_name,
    M_Kw_idents,           M_Kw_keywords,         M_Kw_multilined,
    M_Kw_nested,           M_Kw_newline_is_lexem, M_Kw_numbers,
    M_Kw_scaner_name,      M_Kw_single_lined,     M_Kw_strings,
    M_Kw_token_fields,     M_Kw_header_additions, M_Kw_impl_additions,
    M_Kw_lexem_info_name,  M_arrow
};

struct Main_lexem_info{
    Main_lexem_code code;
    union{
        size_t ident_index;
        size_t string_index;
    };
};
#endif