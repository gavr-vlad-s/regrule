/*
    File:    expr_parser.h
    Created: 12 April 2018 at 09:15 Moscow time
    Author:  Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/

#ifndef EXPR_PARSER_H
#define EXPR_PARSER_H

#include <memory>
#include "../include/ast.h"
#include "../include/expr_scaner.h"
#include "../include/scope.h"
#include "../include/errors_and_tries.h"

class Expr_parser{
public:
    Expr_parser();
    ~Expr_parser();
    Expr_parser(const Expr_parser&) = default;
    Expr_parser(const Expr_scaner_ptr&        esc,
                const Errors_and_tries&       et,
                const std::shared_ptr<Scope>& scope);

    ast::Regexp_ast compile();
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
#endif