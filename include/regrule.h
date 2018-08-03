/*
    File:    regrule.h
    Created: 25 July 2018 at 10:42 Moscow time
    Author:  Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/

#ifndef REGRULE_H
#define REGRULE_H
#include <memory>
#include <cstddef>
#include "../include/ast.h"
#include "../include/expr_parser.h"
#include "../include/scope.h"
#include "../include/errors_and_tries.h"
#include "../include/main_scaner.h"

struct Rule_info{
    size_t          name_;
    ast::Regexp_ast body_;
};

class Regrule{
public:
    Regrule();
    Regrule(const Regrule&)            = default;
    Regrule& operator=(const Regrule&) = default;
    ~Regrule();

    Regrule(const std::shared_ptr<Expr_parser>& ep,
            const std::shared_ptr<Main_scaner>& msc,
            const Errors_and_tries&             et,
            const std::shared_ptr<Scope>&       scope);

    Rule_info compile();
private:
    struct Impl;
    std::shared_ptr<Impl> impl_;
};
#endif