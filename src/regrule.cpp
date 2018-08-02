/*
    File:    regrule.cpp
    Created: 25 July 2018 at 11:37 Moscow time
    Author:  Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/

#include "../include/regrule.h"
#include "../include/main_lexem_info.h"
#include "../include/belongs.h"
#include "../include/idx_to_string.h"

/*
 * Each rule of a regular definition has a form
 *      name -> {regular expression}
 * Hence, a rule can be written as the following regexp:
 *      abc  (1)
 * where
 *      a    identifier which is a rule name
 *      b    -> (i.e., arrow)
 *      c    {regular expression}
 * Minimal DFA for the regexp (1) has the following transition table:
 *
 * |-------|---|---|---|--------------|
 * | State | a | b | c | Remark       |
 * |-------|---|---|---|--------------|
 * |   A   | B |   |   | Start state. |
 * |-------|---|---|---|--------------|
 * |   B   |   | C |   |              |
 * |-------|---|---|---|--------------|
 * |   C   |   |   | D |              |
 * |-------|---|---|---|--------------|
 * |   D   |   |   |   | Final state. |
 * |-------|---|---|---|--------------|
 *
 * But for ease of writing, we need to introduce more meaningful names for states of
 * a finite automaton. The following table shows the matching state names from the
 * previous table and meaningful names. Meaningful names are collected in the enumeration
 * State.
 *
 * |-------|-------------------|
 * | State |  Meaningful name  |
 * |-------|-------------------|
 * |   A   | Start             |
 * |-------|-------------------|
 * |   B   | Rule_name         |
 * |-------|-------------------|
 * |   C   | Arrow             |
 * |-------|-------------------|
 * |   D   | Body              |
 * |-------|-------------------|
 */

struct Regrule::Impl{
public:
    Impl()            = default;
    Impl(const Impl&) = default;
    ~Impl()           = default;

    Impl(const std::shared_ptr<Expr_parser>& ep,
         const std::shared_ptr<Main_scaner>& msc,
         const Errors_and_tries&             et,
         const std::shared_ptr<Scope>&       scope) :
        ep_(ep), msc_(msc), et_(et), scope_(scope) {}

    Rule_info compile();
private:
    std::shared_ptr<Expr_parser> ep_;
    std::shared_ptr<Main_scaner> msc_;
    Errors_and_tries             et_;
    std::shared_ptr<Scope>       scope_;

    Main_lexem_code              lc_;
    Main_lexem_info              li_;

    Rule_info                    current_rule_;

    enum class State{
        Start, Rule_name, Arrow, Body
    };

    typedef bool (Regrule::Impl::*Proc)();

    static Proc                  procs_[];

    bool start_proc(); bool rule_name_proc();
    bool arrow_proc(); bool body_proc();

    void automaton();

    State                        state_;

    void check_rule_name(size_t name_idx);
};

Regrule::~Regrule() = default;

Regrule::Regrule(const std::shared_ptr<Expr_parser>& ep,
                 const std::shared_ptr<Main_scaner>& msc,
                 const Errors_and_tries&             et,
                 const std::shared_ptr<Scope>&       scope) :
    impl_(std::make_shared<Impl>(ep, msc, et, scope)) {}

Regrule::Regrule() : impl_(std::make_shared<Impl>()) {}

Rule_info Regrule::compile()
{
    auto result = impl_->compile();
    return result;
}

Rule_info Regrule::Impl::compile()
{
    automaton();
    return current_rule_;
}

Regrule::Impl::Proc Regrule::Impl::procs_[] = {
    &Regrule::Impl::start_proc, &Regrule::Impl::rule_name_proc,
    &Regrule::Impl::arrow_proc, &Regrule::Impl::body_proc
};

enum class Msg_name{
    Expected_rule_name,            Expected_arrow,
    Expected_opened_curly_bracket, Already_defined_rule_name,
    Unexpected_end_of_text
};

static const char* messages[] = {
    "Error at line %zu: expected a rule name.\n",
    "Error at line %zu: expected an arrow.\n",
    "Error at line %zu: expected an opened curly bracket.\n",
    "Error at line %zu: the rule name %s is already defined.\n",
    "Error at line %zu: unexpected end of text.\n"
};

static inline uint64_t check_id_attribute(Id_kind ikind, Id_attributes iattr)
{
    return belongs(static_cast<uint8_t>(ikind), iattr.kind_);
}

void Regrule::Impl::check_rule_name(size_t name_idx)
{
    auto&  id_scope        = scope_->idsc_;
    auto   it              =  id_scope.find(name_idx);
    if(it == id_scope.end()){
        Id_attributes added_id_attr;
        added_id_attr.kind_ = 1u << static_cast<uint8_t>(Id_kind::Regexp_name);
        id_scope[name_idx]  = added_id_attr;
        return;
    }
    auto existing_id_attr = it->second;
    if(check_id_attribute(Id_kind::Regexp_name, it->second)){
        auto str = idx_to_string(et_.ids_trie, name_idx);
        printf(messages[static_cast<unsigned>(Msg_name::Already_defined_rule_name)],
               msc_->lexem_begin_line_number(),
               str.c_str());
        et_.ec->increment_number_of_errors();
        return;
    }
    existing_id_attr.kind_ |= 1u << static_cast<uint8_t>(Id_kind::Regexp_name);
    id_scope[name_idx]     =  existing_id_attr;
}

bool Regrule::Impl::start_proc()
{
    bool t = true;
    if(lc_ == Main_lexem_code::Id){
        current_rule_.name_ = li_.ident_index;
        state_              = State::Rule_name;
        // Check the name of the rule to see if it is already defined.
        check_rule_name(li_.ident_index);
        return t;
    }
    printf(messages[static_cast<unsigned>(Msg_name::Expected_rule_name)],
           msc_->lexem_begin_line_number());
    et_.ec->increment_number_of_errors();
    switch(lc_){
        case Main_lexem_code::Arrow:
            state_ = State::Arrow;
            break;
        case Main_lexem_code::Opened_fig_brack:
            state_ = State::Body;
            msc_->back();
            break;
        default:
            t      = false;
    }
    return t;
}

bool Regrule::Impl::rule_name_proc()
{
    bool t = true;
    if(lc_ == Main_lexem_code::Arrow){
        state_ = State::Arrow;
        return t;
    }
    printf(messages[static_cast<unsigned>(Msg_name::Expected_arrow)],
           msc_->lexem_begin_line_number());
    et_.ec->increment_number_of_errors();
    switch(lc_){
        case Main_lexem_code::Id:
            current_rule_.name_ = li_.ident_index;
            state_              = State::Rule_name;
            // Check the name of the rule to see if it is already defined.
            check_rule_name(li_.ident_index);
            break;
        case Main_lexem_code::Opened_fig_brack:
            state_ = State::Body;
            msc_->back();
            break;
        default:
            t                   = false;
    }
    return t;
}

bool Regrule::Impl::arrow_proc()
{
    bool t = true;
    if(lc_ == Main_lexem_code::Opened_fig_brack){
        state_ = State::Body;
        msc_->back();
        return t;
    }
    printf(messages[static_cast<unsigned>(Msg_name::Expected_opened_curly_bracket)],
           msc_->lexem_begin_line_number());
    et_.ec->increment_number_of_errors();
    switch(lc_){
        case Main_lexem_code::Id:
            current_rule_.name_ = li_.ident_index;
            state_              = State::Rule_name;
            // Check the name of the rule to see if it is already defined.
            check_rule_name(li_.ident_index);
            break;
        case Main_lexem_code::Arrow:
            state_ = State::Arrow;
            break;
        default:
            t                   = false;
    }
    return t;
}

bool Regrule::Impl::body_proc()
{
    bool t;
    msc_->back();
    // We get the result of parsing the regular expression, which is
    // the body of the rule, and return it.
    auto regexp         = ep_->compile();
    auto regexp_root    = regexp.get_root();
    current_rule_.body_ = std::make_shared<ast::Regexp_ast>(regexp_root);
    t = false;
    return t;
}

void Regrule::Impl::automaton()
{
    current_rule_.name_ = 0;
    current_rule_.body_ = nullptr;
    // Do something.
    state_ = State::Start;
    while((lc_ = (li_ = msc_->current_lexem()).code) != Main_lexem_code::None){
        bool t = (this->*procs_[static_cast<unsigned>(state_)])();
        if(!t){
            break;
        }
    }
    if(state_ != State::Body){
        printf(messages[static_cast<unsigned>(Msg_name::Unexpected_end_of_text)],
               msc_->lexem_begin_line_number());
        et_.ec->increment_number_of_errors();
        msc_->back();
    }
}