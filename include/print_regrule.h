/*
    File:    print_regrule.h
    Created: 29 July 2018 at 18:35 Moscow time
    Author:  Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/

#ifndef PRINT_REGRULE_H
#define PRINT_REGRULE_H
#include <memory>
#include "../include/regrule.h"
#include "../include/char_trie.h"
void print_regrule(const Rule_info& ri, const std::shared_ptr<Char_trie>& t);
#endif