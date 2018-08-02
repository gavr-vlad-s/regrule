/*
    File:    ast.h
    Created: 11 April 2018 at 07:48 Moscow time
    Author:  Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/

#ifndef AST_H
#define AST_H
#include <memory>
#include <cstddef>
#include <list>
namespace ast{

    struct Binary_op;
    struct Unary_op;
    struct Leaf;
    struct Regexp_name_leaf;
    struct Character_leaf;
    struct Char_class_leaf;
    struct Char_class_compl_leaf;

    class Visitor{
    public:
        Visitor()                                      = default;
        Visitor(const Visitor&)                        = default;
        virtual ~Visitor()                             = default;

        virtual void visit(Binary_op&             ref) = 0;
        virtual void visit(Unary_op&              ref) = 0;
        virtual void visit(Regexp_name_leaf&      ref) = 0;
        virtual void visit(Character_leaf&        ref) = 0;
        virtual void visit(Char_class_leaf&       ref) = 0;
        virtual void visit(Char_class_compl_leaf& ref) = 0;
    };

    struct Ast_elem{
        Ast_elem()                                = default;
        Ast_elem(const Ast_elem&)                 = default;
        virtual ~Ast_elem()                       = default;

        size_t action_idx_                        = 0;

        virtual void apply_action(size_t act_idx) = 0;

        virtual void accept(Visitor& v)           = 0;
    };

    enum class Binary_op_kind{
        Or, Concat
    };

    enum class Unary_op_kind{
        Kleene, Positive, Optional
    };

    struct Binary_op : public Ast_elem{
        Binary_op()                             = default;
        Binary_op(const Binary_op&)             = default;
        virtual ~Binary_op()                    = default;

        Binary_op(Binary_op_kind kind, std::list<std::shared_ptr<Ast_elem>> children) :
            kind_(kind), children_(children) {}

        Binary_op_kind                       kind_     = Binary_op_kind::Or;
        std::list<std::shared_ptr<Ast_elem>> children_;

        void apply_action(size_t act_idx) override;

        void accept(Visitor& v) override
        {
            v.visit(*this);
        }
    };

    struct Unary_op : public Ast_elem{
        Unary_op()                       = default;
        Unary_op(const Unary_op&)        = default;
        virtual ~Unary_op()              = default;

        Unary_op(Unary_op_kind kind, const std::shared_ptr<Ast_elem>& child) :
            kind_(kind), child_(child) {}

        Unary_op_kind             kind_  = Unary_op_kind::Kleene;
        std::shared_ptr<Ast_elem> child_ = nullptr;

        void apply_action(size_t act_idx) override;

        void accept(Visitor& v) override
        {
            v.visit(*this);
        }
    };

    struct Leaf : public Ast_elem{
        Leaf()            = default;
        Leaf(const Leaf&) = default;
        virtual ~Leaf()   = default;

        void apply_action(size_t act_idx) override = 0;

        void accept(Visitor& v) override           = 0;
    };

    struct Regexp_name_leaf : public Leaf{
        Regexp_name_leaf()                        = default;
        Regexp_name_leaf(const Regexp_name_leaf&) = default;
        virtual ~Regexp_name_leaf()               = default;

        Regexp_name_leaf(size_t regexp_name_index) :
            regexp_name_index_(regexp_name_index){}

        size_t   regexp_name_index_               = 0;

        void apply_action(size_t act_idx) override;

        void accept(Visitor& v) override
        {
            v.visit(*this);
        }
    };

    struct Character_leaf : public Leaf{
        Character_leaf()                      = default;
        Character_leaf(const Character_leaf&) = default;
        virtual ~Character_leaf()             = default;

        Character_leaf(char32_t c) : c_(c){}

        char32_t  c_                          = 0;

        void apply_action(size_t act_idx) override;

        void accept(Visitor& v) override
        {
            v.visit(*this);
        }
    };

    struct Char_class_leaf : public Leaf{
        Char_class_leaf()                       = default;
        Char_class_leaf(const Char_class_leaf&) = default;
        virtual ~Char_class_leaf()              = default;

        Char_class_leaf(size_t set_of_char_index) :
            set_of_char_index_(set_of_char_index){}

        size_t   set_of_char_index_             = 0;

        void apply_action(size_t act_idx) override;

        void accept(Visitor& v) override
        {
            v.visit(*this);
        }
    };

    struct Char_class_compl_leaf : public Leaf{
        Char_class_compl_leaf()                             = default;
        Char_class_compl_leaf(const Char_class_compl_leaf&) = default;
        virtual ~Char_class_compl_leaf()                    = default;

        Char_class_compl_leaf(size_t set_of_char_index) :
            set_of_char_index_(set_of_char_index){}

        size_t   set_of_char_index_                         = 0;

        void apply_action(size_t act_idx) override;

        void accept(Visitor& v) override
        {
            v.visit(*this);
        }
    };

    using Regexp = std::list<std::shared_ptr<Ast_elem>>;

    class Regexp_ast{
    public:
        Regexp_ast()                  = default;
        Regexp_ast(const Regexp_ast&) = default;
        ~Regexp_ast();

        Regexp_ast(const std::shared_ptr<Ast_elem>& root) : root_(root) {}

        void traverse(Visitor& v) const{
            if(root_){
                root_->accept(v);
            }
        }

        void traverse(Visitor& v){
            if(root_){
                root_->accept(v);
            }
        }

        std::shared_ptr<Ast_elem> get_root() const{
            return root_;
        }
    private:
        std::shared_ptr<Ast_elem> root_;
    };
};
#endif