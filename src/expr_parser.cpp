/*
    File:    expr_parser.cpp
    Created: 12 April 2018 at 09:27 Moscow time
    Author:  Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/

#include <cstdint>
#include "../include/expr_parser.h"
#include "../include/expr_lexem_info.h"
#include "../include/belongs.h"
#include "../include/idx_to_string.h"

/* Grammar rules for regexps:
 *
 * S -> pTq
 * T -> E(bE)*
 * E -> F+
 * F -> Gc?
 * G -> Ha?
 * H -> d | lTr
 *
 * In this grammar, a means $action_name; b means the operator |; c means unary
 * operators ? * +; d means a character, a character class, a character class
 * complement, or %regexp_name; p means { (opening curly bracket); q means }
 * (closing curly bracket); l means ( (opening round bracket); r means ) (closing
 * round bracket).
 *
 * To parse regexps, we will use a recursive descent. That is, for any non-terminal
 * we will write a corresponding function. Namely, for the non-terminal S we will
 * write the function proc_S, for the non-terminal T we will write the function proc_T,
 * for the non-terminal E we will write the function proc_E, for the non-terminal F we
 * will write the function proc_F, for the non-terminal G we will write the function
 * proc_G, for the non-terminal H we will write the function proc_H.
 *
 * Each of these functions is implemented as a deterministic finite automaton, which
 * is constructed by a regexp in the right-hand side of a rule.
 *
 * Firstly, the function proc_S is an implementation of a minimal non-deterministic
 * finite automaton for the regexp pTq. The transition table of this automaton is
 *
 * |-------------------|------------------|-------------------|---|--------------|
 * |      State        |         p        |         q         | T |    Remark    |
 * |-------------------|------------------|-------------------|---|--------------|
 * | Start             | Open_fig_bracket |                   |   | Start state. |
 * |-------------------|------------------|-------------------|---|--------------|
 * | Open_fig_bracket  |                  |                   | T |              |
 * |-------------------|------------------|-------------------|---|--------------|
 * | T                 |                  | Close_fig_bracket |   |              |
 * |-------------------|------------------|-------------------|---|--------------|
 * | Close_fig_bracket |                  |                   |   | Final state. |
 * |-------------------|------------------|-------------------|---|--------------|
 *
 * Secondly, the function proc_T is an implementation of a minimal non-deterministic
 * finite automaton for the regexp E(bE)*. The transition table of this automaton is
 *
 * |-------|-------|---|--------------|
 * | State |   b   | E |    Remark    |
 * |-------|-------|---|--------------|
 * | Start |       | E | Start state. |
 * |-------|-------|---|--------------|
 * | E     | Start |   | Final state. |
 * |-------|-------|---|--------------|
 *
 * Thirdly, the function proc_E is an implementation of a minimal non-deterministic
 * finite automaton for the regexp F+. The transition table of this automaton is
 *
 * |-------|---|--------------|
 * | State | F |    Remark    |
 * |-------|---|--------------|
 * | Start | F | Start state. |
 * |-------|---|--------------|
 * | F     | F | Final state. |
 * |-------|---|--------------|
 *
 * Fourthly, the function proc_F is an implementation of a minimal non-deterministic
 * finite automaton for the regexp Gc?. The transition table of this automaton is
 *
 * |-------|-------|---|--------------|
 * | State |   c   | G |    Remark    |
 * |-------|-------|---|--------------|
 * | Start |       | G | Start state. |
 * |-------|-------|---|--------------|
 * | G     | Unary |   | Final state. |
 * |-------|-------|---|--------------|
 * | Unary |       |   | Final state. |
 * |-------|-------|---|--------------|
 *
 * Fifthly, the function proc_G is an implementation of a minimal non-deterministic
 * finite automaton for the regexp Ha?. The transition table of this automaton is
 *
 * |-------|-------|---|--------------|
 * | State |   a   | H |    Remark    |
 * |-------|-------|---|--------------|
 * | Start |       | H | Start state. |
 * |-------|-------|---|--------------|
 * | H     | Act   |   | Final state. |
 * |-------|-------|---|--------------|
 * | Act   |       |   | Final state. |
 * |-------|-------|---|--------------|
 *
 * Sixthly, the function proc_H is an implementation of a minimal non-deterministic
 * finite automaton for the regexp d | lTr. The transition table of this automaton is
 *
 * |-------|---|---|---|---|--------------|
 * | State | d | l | r | T |    Remark    |
 * |-------|---|---|---|---|--------------|
 * | Start | D | L |   |   | Start state. |
 * |-------|---|---|---|---|--------------|
 * | D     |   |   |   |   | Final state. |
 * |-------|---|---|---|---|--------------|
 * | L     |   |   |   | T |              |
 * |-------|---|---|---|---|--------------|
 * | T     |   |   | R |   |              |
 * |-------|---|---|---|---|--------------|
 * | R     |   |   |   |   | Final state. |
 * |-------|---|---|---|---|--------------|
 *
 */

enum class Terminal{
    End_of_text, Term_a,  Term_b,
    Term_c,      Term_d,  Term_p,
    Term_q,      Term_LP, Term_RP
};

enum class H_State{
    Start, D, L, T, R
};

struct Expr_parser::Impl{
public:
    Impl()            = default;
    Impl(const Impl&) = default;
    ~Impl()           = default;

    Impl(const Expr_scaner_ptr&        esc,
         const Errors_and_tries&       et,
         const std::shared_ptr<Scope>& scope) :
        esc_(esc), et_(et), scope_(scope) {}

    ast::Regexp_ast compile();
private:
    std::shared_ptr<Expr_scaner>   esc_;
    Errors_and_tries               et_;
    std::shared_ptr<Scope>         scope_;

    std::shared_ptr<ast::Ast_elem> proc_S();
    std::shared_ptr<ast::Ast_elem> proc_T();
    std::shared_ptr<ast::Ast_elem> proc_E();
    std::shared_ptr<ast::Ast_elem> proc_F();
    std::shared_ptr<ast::Ast_elem> proc_G();
    std::shared_ptr<ast::Ast_elem> proc_H();

//     Expr_lexem_info li_;
//     Terminal        t_;

    bool proc_H_state_Start(std::shared_ptr<ast::Ast_elem>& e,
                            H_State&                        state,
                            Expr_lexem_info                 li,
                            Terminal                        t);
};

Expr_parser::~Expr_parser() = default;

Expr_parser::Expr_parser(const Expr_scaner_ptr&        esc,
                         const Errors_and_tries&       et,
                         const std::shared_ptr<Scope>& scope) :
    impl_(std::make_unique<Impl>(esc, et, scope)) {}

Expr_parser::Expr_parser() : impl_(std::make_unique<Impl>()) {}

ast::Regexp_ast Expr_parser::compile()
{
    return impl_->compile();
}

ast::Regexp_ast Expr_parser::Impl::compile()
{
    return ast::Regexp_ast{proc_S()};
}

static const Terminal lexem2terminal_map[] = {
    Terminal::End_of_text, Terminal::End_of_text, Terminal::Term_a,
    Terminal::Term_d,      Terminal::Term_LP,     Terminal::Term_RP,
    Terminal::Term_b,      Terminal::Term_c,      Terminal::Term_c,
    Terminal::Term_c,      Terminal::Term_d,      Terminal::Term_p,
    Terminal::Term_q,      Terminal::Term_d,      Terminal::Term_d
};

static Terminal lexem2terminal(const Expr_lexem_info& l)
{
    return lexem2terminal_map[static_cast<uint16_t>(l.code)];
}

static const char* opening_curly_brace_is_expected =
    "Error at line %zu: an opening curly brace is expected.\n";

static const char* closing_curly_brace_is_expected =
    "Error at line %zu: an closing curly brace is expected.\n";

static const char* undefined_action                =
    "Error at line %zu: the action %s is not defined.\n";

static const char* not_action_name                 =
    "Error at line %zu: the identifier %s is not an action name.\n";

static const char* unexpected_end_of_regexp        =
    "Error at line %zu: an unexpected end of regexp.\n";

static const char* unexpected_action               =
    "Error at line %zu: an unexpected action name.\n";

static const char* unexpected_or                   =
    "Error at line %zu: unexpected operator |.\n";

static const char* unexpected_postfix_operator     =
    "Error at line %zu: unexpected postfix operator.\n";

static const char* unexpected_p                    =
    "Error at line %zu: unexpected begin of regexp.\n";

static const char* unexpected_q                    =
    "Error at line %zu: unexpected end of regexp.\n";

static const char* unexpected_rp                   =
    "Error at line %zu: unexpected closing round bracket.\n";

static const char* expected_rp                     =
    "Error at line %zu: expected closing round bracket.\n";


std::shared_ptr<ast::Ast_elem> Expr_parser::Impl::proc_S()
{
    std::shared_ptr<ast::Ast_elem> result;
    enum class State{
        Start, Open_fig_bracket, T, Close_fig_bracket
    };

    State state = State::Start;
    for(;;){
        Expr_lexem_info li = esc_->current_lexem();
        Terminal        t  = lexem2terminal(li);
        switch(state){
            case State::Start:
                if(t != Terminal::Term_p){
                    printf(opening_curly_brace_is_expected,
                           esc_->lexem_begin_line_number());
                    et_.ec->increment_number_of_errors();
                    esc_->back();
                    return result;
                }
                state = State::Open_fig_bracket;
                break;
            case State::Open_fig_bracket:
                esc_->back();
                result = proc_T();
                if(!result){
                    return result;
                }
                state  = State::T;
                break;
            case State::T:
                if(t != Terminal::Term_q){
                    esc_->back();
                    printf(closing_curly_brace_is_expected,
                           esc_->lexem_begin_line_number());
                    et_.ec->increment_number_of_errors();
                    return result;
                }
                state = State::Close_fig_bracket;
                break;
            default:
                esc_->back();
                return result;
        }
    }
    return result;
}

using Or_args = std::list<std::shared_ptr<ast::Ast_elem>>;

static std::shared_ptr<ast::Ast_elem> build_or_node(const Or_args& children,
                                                    size_t         num_of_children)
{
    switch(num_of_children){
        case 0:
            return nullptr;
        case 1:
            return children.front();
        default:
            return std::make_shared<ast::Binary_op>(ast::Binary_op_kind::Or,
                                                    children);
    }
}

std::shared_ptr<ast::Ast_elem> Expr_parser::Impl::proc_T()
{
    std::shared_ptr<ast::Ast_elem> node;

    enum class State{
        Start, E
    };

    State                                     state           = State::Start;
    std::list<std::shared_ptr<ast::Ast_elem>> children;
    size_t                                    num_of_children = 0;
    for(;;){
        Expr_lexem_info li = esc_->current_lexem();
        Terminal        t  = lexem2terminal(li);
        switch(state){
            case State::Start:
                {
                    esc_->back();
                    auto p = proc_E();
                    if(!p){
                        return node;
                    }
                    children.push_back(p);
                    state = State::E;
                    num_of_children++;
                }
                break;
            case State::E:
                if(t == Terminal::Term_b){
                    state = State::Start;
                }else{
                    esc_->back();
                    return build_or_node(children, num_of_children);
                }
                break;
        }
    }
    return node;
}

using Concat_args = std::list<std::shared_ptr<ast::Ast_elem>>;

static std::shared_ptr<ast::Ast_elem> build_concat_node(const Concat_args& children,
                                                        size_t             num_of_children)
{
    switch(num_of_children){
        case 0:
            return nullptr;
        case 1:
            return children.front();
        default:
            return std::make_shared<ast::Binary_op>(ast::Binary_op_kind::Concat,
                                                    children);
    }
}

std::shared_ptr<ast::Ast_elem> Expr_parser::Impl::proc_E()
{
    std::shared_ptr<ast::Ast_elem> node;

    enum class State{
        Start, F
    };

    State                                     state           = State::Start;
    std::list<std::shared_ptr<ast::Ast_elem>> children;
    size_t                                    num_of_children = 0;
    for(;;){
        Expr_lexem_info li = esc_->current_lexem();
        Terminal        t  = lexem2terminal(li);
        esc_->back();
        switch(state){
            case State::Start:
                {
                    auto p = proc_F();
                    if(!p){
                        return node;
                    }
                    children.push_back(p);
                    state = State::F;
                    num_of_children++;
                }
                break;
            case State::F:
                if((t == Terminal::Term_d) || (t == Terminal::Term_LP)){
                    auto p = proc_F();
                    if(!p){
                        return build_concat_node(children, num_of_children);
                    }
                    children.push_back(p);
                    num_of_children++;
                }else{
                    return build_concat_node(children, num_of_children);
                }
                break;
        }
    }
    return node;
}

static inline ast::Unary_op_kind lexem2unary_kind(const Expr_lexem_info& li)
{
    uint16_t shift  = static_cast<uint16_t>(Expr_lexem_code::Kleene_closure);
    return static_cast<ast::Unary_op_kind>(static_cast<uint16_t>(li.code) - shift);
}

std::shared_ptr<ast::Ast_elem> Expr_parser::Impl::proc_F()
{
    std::shared_ptr<ast::Ast_elem> node;

    enum class State{
        Start, G, Unary
    };

    State state = State::Start;
    for(;;){
        Expr_lexem_info li = esc_->current_lexem();
        Terminal        t  = lexem2terminal(li);
        switch(state){
            case State::Start:
                esc_->back();
                node  = proc_G();
                if(!node){
                    return node;
                }
                state = State::G;
                break;
            case State::G:
                if(t != Terminal::Term_c){
                    esc_->back();
                    return node;
                }
                state = State::Unary;
                {
                    auto p = std::make_shared<ast::Unary_op>(lexem2unary_kind(li), node);
                    node   = p;
                }
                break;
            case State::Unary:
                esc_->back();
                return node;
        }
    }
    return node;
}

static inline uint64_t check_id_attribute(Id_kind ikind, Id_attributes iattr)
{
    return belongs(static_cast<uint8_t>(ikind), iattr.kind_);
}

std::shared_ptr<ast::Ast_elem> Expr_parser::Impl::proc_G()
{
    std::shared_ptr<ast::Ast_elem> node;

    enum class State{
        Start, H, Act
    };

    State state = State::Start;
    for(;;){
        Expr_lexem_info li = esc_->current_lexem();
        Terminal        t  = lexem2terminal(li);
        switch(state){
            case State::Start:
                esc_->back();
                node  = proc_H();
                if(!node){
                    return node;
                }
                state = State::H;
                break;
            case State::H:
                if(t == Terminal::Term_a){
                    state           = State::Act;
                    auto&  id_scope = scope_->idsc_;
                    size_t act_idx  = li.action_name_index;
                    auto   it        = id_scope.find(act_idx);
                    if(it == id_scope.end()){
                        auto str = idx_to_string(et_.ids_trie, act_idx);
                        printf(undefined_action,
                               esc_->lexem_begin_line_number(),
                               str.c_str());
                        et_.ec->increment_number_of_errors();
                        return nullptr;
                    }
                    if(!check_id_attribute(Id_kind::Action_name, it->second))
                    {
                        auto str = idx_to_string(et_.ids_trie, act_idx);
                        printf(not_action_name,
                               esc_->lexem_begin_line_number(),
                               str.c_str());
                        et_.ec->increment_number_of_errors();
                        return nullptr;
                    }
                    node->apply_action(act_idx);
                }else{
                    esc_->back();
                    return node;
                }
                break;
            case State::Act:
                esc_->back();
                return node;
        }
    }
    return node;
}

static std::shared_ptr<ast::Leaf> lexem2leaf(const Expr_lexem_info& li)
{
    std::shared_ptr<ast::Leaf> result;
    switch(li.code){
        case Expr_lexem_code::Regexp_name:
            result = std::make_shared<ast::Regexp_name_leaf>(li.regexp_name_index);
            break;
        case Expr_lexem_code::Character:
            result = std::make_shared<ast::Character_leaf>(li.c);
            break;
        case Expr_lexem_code::Class_complement:
            result = std::make_shared<ast::Char_class_compl_leaf>(li.set_of_char_index);
            break;
        case Expr_lexem_code::Character_class:
            result = std::make_shared<ast::Char_class_leaf>(li.set_of_char_index);
            break;
        default:
            ;
    }
    return result;
}

bool Expr_parser::Impl::proc_H_state_Start(std::shared_ptr<ast::Ast_elem>& node,
                                           H_State&                        state,
                                           Expr_lexem_info                 li,
                                           Terminal                        t)
{
    bool there_are_errors = true;
    switch(t){
        case Terminal::Term_d:
            node              = lexem2leaf(li);
            state             = H_State::D;
            there_are_errors  = false;
            break;
        case Terminal::Term_LP:
            state             = H_State::L;
            there_are_errors  = false;
            break;
        case Terminal::End_of_text:
            esc_->back();
            printf(unexpected_end_of_regexp, esc_->lexem_begin_line_number());
            et_.ec->increment_number_of_errors();
            break;
        case Terminal::Term_a:
            printf(unexpected_action, esc_->lexem_begin_line_number());
            et_.ec->increment_number_of_errors();
            break;
        case Terminal::Term_b:
            printf(unexpected_or, esc_->lexem_begin_line_number());
            et_.ec->increment_number_of_errors();
            break;
        case Terminal::Term_c:
            printf(unexpected_postfix_operator, esc_->lexem_begin_line_number());
            et_.ec->increment_number_of_errors();
            break;
        case Terminal::Term_p:
            printf(unexpected_p, esc_->lexem_begin_line_number());
            et_.ec->increment_number_of_errors();
            break;
        case Terminal::Term_q:
            printf(unexpected_q, esc_->lexem_begin_line_number());
            et_.ec->increment_number_of_errors();
            break;
        case Terminal::Term_RP:
            printf(unexpected_rp, esc_->lexem_begin_line_number());
            et_.ec->increment_number_of_errors();
            break;
    }
    return there_are_errors;
}

std::shared_ptr<ast::Ast_elem> Expr_parser::Impl::proc_H()
{
    std::shared_ptr<ast::Ast_elem> node;

    H_State state = H_State::Start;
    for(;;){
        Expr_lexem_info li = esc_->current_lexem();
        Terminal        t  = lexem2terminal(li);
        bool there_are_errors;
        switch(state){
            case H_State::Start:
                there_are_errors = proc_H_state_Start(node, state, li, t);
                if(there_are_errors){
                    return node;
                }
                break;
            case H_State::D:
                esc_->back();
                return node;
            case H_State::L:
                esc_->back();
                node  = proc_T();
                if(!node){
                    return node;
                }
                state = H_State::T;
                break;
            case H_State::T:
                if(t != Terminal::Term_RP){
                    esc_->back();
                    printf(expected_rp, esc_->lexem_begin_line_number());
                    et_.ec->increment_number_of_errors();
                    return nullptr;
                }
                state = H_State::R;
                break;
            case H_State::R:
                esc_->back();
                return node;
        }
    }
    return node;
}