/*
    File:    test-regrule.cpp
    Created: 31 July 2018 at 11:00 Moscow time
    Author:  Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/

#include <cstddef>
#include <cstdio>
#include <string>
#include <memory>
#include "../include/get_processed_text.h"
#include "../include/location.h"
#include "../include/errors_and_tries.h"
#include "../include/char_trie.h"
#include "../include/trie_for_set.h"
#include "../include/main_scaner.h"
#include "../include/expr_scaner.h"
#include "../include/scope.h"
// // // // // // // // // // // // // #include "../include/ast.h"
#include "../include/expr_parser.h"
// // // // // // // // // // // // // #include "../include/print_ast.h"
#include "../include/char_conv.h"
#include "../include/regrule.h"
#include "../include/print_regrule.h"
// // // // // // // // // // // // // #include "../include/regular_definition_section.h"
// // // // // // // // // // // // // #include "../include/print_regdef.h"

struct Act_info{
    std::u32string name_;
    std::u32string body_;
};

static const Act_info added_acts[] = {
    {U"write",                      U"buffer += ch;"                                          },
    {U"add_dec_digit_to_char_code", U"char_code = char_code * 10 + digit2int(ch);"            },
    {U"add_hex_digit_to_char_code", U"char_code = char_code << 4 + digit2int(ch);"            },
    {U"add_bin_digit_to_char_code", U"char_code = char_code << 1 + digit2int(ch);"            },
    {U"add_oct_digit_to_char_code", U"char_code = char_code << 3 + digit2int(ch);"            },
    {U"write_by_code",              U"buffer += char_code;"                                   },
    {U"add_dec_digit",              U"token.int_value = token.int_value * 10 + digit2int(ch);"},
    {U"add_hex_digit",              U"token.int_value = token.int_value << 4 + digit2int(ch);"},
    {U"add_bin_digit",              U"token.int_value = token.int_value << 1 + digit2int(ch);"},
    {U"add_oct_digit",              U"token.int_value = token.int_value << 3 + digit2int(ch);"}
};

void add_action(Errors_and_tries&       etr,
                std::shared_ptr<Scope>& scope,
                const std::u32string&   name,
                const std::u32string&   body)
{
    Id_attributes iattr;
    iattr.kind_             = 1u << static_cast<uint8_t>(Id_kind::Action_name);
    size_t idx              = etr.ids_trie -> insert(name);
    size_t body_idx         = etr.strs_trie-> insert(body);
    iattr.act_string_       = body_idx;
    scope->idsc_[idx]       = iattr;

    Str_attributes sattr;
    sattr.kind_             = 1u << static_cast<uint16_t>(Str_kind::Action_definition);
    sattr.code_             = 0;
    scope->strsc_[body_idx] = sattr;

    auto name_in_utf8       = u32string_to_utf8(name);
    printf("Index of action with name %s is %zu.\n", name_in_utf8.c_str(), idx);
}

enum Myauka_exit_codes{
    Success, No_args, File_processing_error, Syntax_error
};

static const char* usage_str = "Usage: %s file\n";

int main(int argc, char* argv[])
{
    if(1 == argc){
        printf(usage_str, argv[0]);
        return No_args;
    }

    std::u32string    text    = get_processed_text(argv[1]);
    if(!text.length()){
        return File_processing_error;
    }

    char32_t*        p        = const_cast<char32_t*>(text.c_str());
    auto             loc      = std::make_shared<Location>(p);
    Errors_and_tries et;
    et.ec                     = std::make_shared<Error_count>();
    et.ids_trie               = std::make_shared<Char_trie>();
    et.strs_trie              = std::make_shared<Char_trie>();
    auto             set_trie = std::make_shared<Trie_for_set_of_char32>();
    auto             esc      = std::make_shared<Expr_scaner>(loc, et, set_trie);
    auto             msc      = std::make_shared<Main_scaner>(loc, et);
    auto             scope    = std::make_shared<Scope>();

    for(const auto& ai : added_acts){
        add_action(et, scope, ai.name_, ai.body_);
    }

    auto             ep       = std::make_shared<Expr_parser>(esc, et, scope);
    auto             regrulep = std::make_shared<Regrule>(ep, msc, et, scope);
    auto             rule     = regrulep->compile();
    size_t           nerrors  = et.ec->get_number_of_errors();
    if(nerrors){
        printf("Total number of errors: %zu.\n",nerrors);
        return Syntax_error;
    }
    print_regrule(rule, et.ids_trie);
// // // // // // // // // // // // //     auto             regdefp  = std::make_shared<regdef_section::Regdef_section>(scope,
// // // // // // // // // // // // //                                                                                  et,
// // // // // // // // // // // // //                                                                                  msc,
// // // // // // // // // // // // //                                                                                  ep);
// // // // // // // // // // // // //
// // // // // // // // // // // // //     std::map<size_t, ast::Regexp_ast> m;
// // // // // // // // // // // // //
// // // // // // // // // // // // //     constexpr Main_lexem_code begin_code = Main_lexem_code::Kw_strings;
// // // // // // // // // // // // //
// // // // // // // // // // // // // //     auto             regdef   = regdefp->compile(begin_code, m);
// // // // // // // // // // // // // //     print_regdef(regdef, m, et);
    return Success;
}