/*
    File:    main_scaner_keyword_table.h
    Created: 23 March 2018 at 15:27 Moscow time
    Author:  Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/

#ifndef MAIN_SCANER_KEYWORD_TABLE_H
#define MAIN_SCANER_KEYWORD_TABLE_H
#include "../include/elem.h"
#include "../include/main_lexem_info.h"
/* For the keyword processing automaton, the member state of the class Main_scaner
 * is the index of the element in the transition table, denoted below as
 * a_keyword_jump_table. */
extern const Elem<Main_lexem_code> a_keyword_jump_table[];
#endif