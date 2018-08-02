/*
    File:    print_ast.cpp
    Created: 19 April 2018 at 09:34 Moscow time
    Author:  Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/

// #define DEBUG
// #ifdef DEBUG
// #   include <cstdio>
// #endif
#include "../include/print_ast.h"
#include "../include/print_char32.h"
class To_string_visitor : public ast::Visitor{
public:
    To_string_visitor()                         = default;
    To_string_visitor(const To_string_visitor&) = default;
    virtual ~To_string_visitor()                = default;

//     std::string to_string(const std::shared_ptr<ast::Ast_elem>& root);

    std::string to_string(const ast::Regexp_ast& tree);

    virtual void visit(ast::Binary_op&             ref) override;
    virtual void visit(ast::Unary_op&              ref) override;
    virtual void visit(ast::Regexp_name_leaf&      ref) override;
    virtual void visit(ast::Character_leaf&        ref) override;
    virtual void visit(ast::Char_class_leaf&       ref) override;
    virtual void visit(ast::Char_class_compl_leaf& ref) override;

private:
    size_t      indent_     = 0;
    std::string str_repres_;
};

std::string ast2string(const ast::Regexp_ast& tree)
{
    std::string       result;
    To_string_visitor v;
//     result = v.to_string(tree.get_root());
    result = v.to_string(tree);
// #ifdef DEBUG
//     printf("final str_repres_: %s\n", result.c_str());
// #endif
    return result;
}

std::string To_string_visitor::to_string(const ast::Regexp_ast& tree)
{
    indent_ = 0;
    str_repres_.clear();
    tree.traverse(*this);
    return str_repres_;
}

// std::string To_string_visitor::to_string(const std::shared_ptr<ast::Ast_elem>& root)
// {
//     indent_ = 0;
//     str_repres_.clear();
//     root->accept(*this);
//     return str_repres_;
// }

static constexpr size_t indent_increment = 4;

static const std::string binary_op_strs[] = {
    "{Or\n", "{Concat\n"
};

static const std::string unary_op_strs[] = {
    "{Kleene\n", "{Positive\n", "{Optional\n"
};

void To_string_visitor::visit(ast::Binary_op& ref)
{
    auto indent    =  std::string(indent_, ' ');
    str_repres_ += indent + binary_op_strs[static_cast<unsigned>(ref.kind_)];
    indent_     += indent_increment;
    for(auto& child : ref.children_){
        if(child){
            child->accept(*this);
        }else{
            str_repres_ += std::string(indent_, ' ') + "{nullptr}\n";
        }
    }
    indent_     -= indent_increment;
    str_repres_ += indent + "}\n";
// #ifdef DEBUG
//     printf("str_repres_: %s\n", str_repres_.c_str());
// #endif
}

void To_string_visitor::visit(ast::Unary_op& ref)
{
    auto ind    =  std::string(indent_, ' ');
    str_repres_ += ind + unary_op_strs[static_cast<unsigned>(ref.kind_)];
    indent_     += indent_increment;
    auto& child =  ref.child_;
    if(child){
        child->accept(*this);
    }else{
        str_repres_ += std::string(indent_, ' ') + "{nullptr}\n";
    }
    indent_     -= indent_increment;
    str_repres_ += ind + "}\n";
}

void To_string_visitor::visit(ast::Regexp_name_leaf& ref)
{
    str_repres_ += std::string(indent_, ' ')              +
                   "{REgexp_name "                        +
                   std::to_string(ref.regexp_name_index_) +
                   "[action_idx_ : "                      +
                   std::to_string(ref.action_idx_)       +
                   "]}\n";
}

void To_string_visitor::visit(ast::Character_leaf& ref)
{
    str_repres_ += std::string(indent_, ' ')     +
                "{Char "                         +
                show_char32(ref.c_)              +
                "[action_idx_ : "                +
                std::to_string(ref.action_idx_) +
                "]}\n";
}

void To_string_visitor::visit(ast::Char_class_leaf& ref)
{
    str_repres_ += std::string(indent_, ' ')              +
                   "{Char_class "                         +
                   std::to_string(ref.set_of_char_index_) +
                   "[action_idx_ : "                      +
                   std::to_string(ref.action_idx_)       +
                   "]}\n";
}

void To_string_visitor::visit(ast::Char_class_compl_leaf& ref)
{
    str_repres_ += std::string(indent_, ' ')              +
                   "{Char_class_complement "              +
                   std::to_string(ref.set_of_char_index_) +
                   "[action_idx_ : "                      +
                   std::to_string(ref.action_idx_)       +
                   "]}\n";
}