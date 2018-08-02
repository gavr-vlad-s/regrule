/*
    File:    print_ast.h
    Created: 19 April 2018 at 09:08 Moscow time
    Author:  Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/

#ifndef PRINT_AST_H
#define PRINT_AST_H
#include <string>
#include <memory>
#include "../include/ast.h"
std::string ast2string(const ast::Regexp_ast& tree);
#endif