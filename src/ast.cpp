/*
    File:    ast.cpp
    Created: 13 April 2018 at 18:46 Moscow time
    Author:  Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/

#include "../include/ast.h"

namespace ast{
    void Binary_op::apply_action(size_t act_idx)
    {
        for(auto& child : children_){
            child->apply_action(act_idx);
        }
    }

    void Unary_op::apply_action(size_t act_idx)
    {
        child_->apply_action(act_idx);
    }

    void Regexp_name_leaf::apply_action(size_t act_idx)
    {
        action_idx_ = act_idx;
    }

    void Character_leaf::apply_action(size_t act_idx)
    {
        action_idx_ = act_idx;
    }

    void Char_class_leaf::apply_action(size_t act_idx)
    {
        action_idx_ = act_idx;
    }

    void Char_class_compl_leaf::apply_action(size_t act_idx)
    {
        action_idx_ = act_idx;
    }

    class Deleter : public Visitor{
    public:
        Deleter()               = default;
        Deleter(const Deleter&) = default;
        virtual ~Deleter()      = default;

        void visit(ast::Binary_op&             ref) override;
        void visit(ast::Unary_op&              ref) override;
        void visit(ast::Regexp_name_leaf&      ref) override;
        void visit(ast::Character_leaf&        ref) override;
        void visit(ast::Char_class_leaf&       ref) override;
        void visit(ast::Char_class_compl_leaf& ref) override;
    };

    void Deleter::visit(ast::Binary_op& ref){
        for(auto& child : ref.children_){
            if(child){
                child->accept(*this);
                child = nullptr;
            }
        }
    }

    void Deleter::visit(ast::Unary_op& ref){
        auto& child =  ref.child_;
        if(child){
            child->accept(*this);
            child = nullptr;
        }
    }

    void Deleter::visit(ast::Regexp_name_leaf& ref){}

    void Deleter::visit(ast::Character_leaf& ref){}

    void Deleter::visit(ast::Char_class_leaf& ref){}

    void Deleter::visit(ast::Char_class_compl_leaf& ref){}

    Regexp_ast::~Regexp_ast()
    {
        if(root_){
            Deleter d;
            root_->accept(d);
        }
    }
};