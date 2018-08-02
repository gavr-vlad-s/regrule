/*
    File:    main_scaner.h
    Created: 14 December 2015 at 15:25 Moscow time
    Author:  Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/

#ifndef MAIN_SCANER_H
#define MAIN_SCANER_H

#include <string>
#include "../include/abstract_scaner.h"
#include "../include/error_count.h"
#include "../include/main_lexem_info.h"

class Main_scaner : public Abstract_scaner<Main_lexem_info>{
public:
    Main_scaner()                        = default;
    Main_scaner(const Location_ptr& location, const Errors_and_tries& et) :
        Abstract_scaner<Main_lexem_info>(location, et) {};
    Main_scaner(const Main_scaner& orig) = default;
    virtual ~Main_scaner()               = default;
    virtual Main_lexem_info current_lexem();
private:
    enum Automaton_name{
        A_start,   A_unknown,   A_id,
        A_keyword, A_delimiter, A_string
    };
    Automaton_name automaton; /* current automaton */

    typedef bool (Main_scaner::*Automaton_proc)();
    /* This is the type of pointer to the member function that implements
     * the state machine that processes the lexeme. This function must
     * return true if the lexeme is not yet parsed, and false otherwise. */

    typedef void (Main_scaner::*Final_proc)();
    /* And this is the type of the pointer to the member function that
     * performs the necessary actions in the event of an unexpected end
     * of the lexeme. */

    static Automaton_proc procs[];
    static Final_proc     finals[];
    /* lexeme processing functions: */
    bool start_proc();     bool unknown_proc();
    bool id_proc();        bool keyword_proc();
    bool delimiter_proc(); bool string_proc();
    /*functions for performing actions in case of an unexpected end of the token: */
    void none_final_proc();      void unknown_final_proc();
    void id_final_proc();        void keyword_final_proc();
    void delimiter_final_proc(); void string_final_proc();
    /* A function that corrects a lexeme code, most likely, is a keyword, and
     *outputs the necessary diagnostics. */
    void correct_keyword();

    void correct_delim();
};
#endif