/*
    File:    print_regrule.cpp
    Created: 29 July 2018 at 18:37 Moscow time
    Author:  Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/

#include <cstddef>
#include "../include/print_regrule.h"
#include "../include/idx_to_string.h"
#include "../include/print_ast.h"
void print_regrule(const Rule_info& ri, const std::shared_ptr<Char_trie>& t)
{
    auto rname = idx_to_string(t, ri.name_);
    auto rbody = ast2string(ri.body_);
    printf("rule with name %s [%zu]:\n %s\n", rname.c_str(), ri.name_, rbody.c_str());
}