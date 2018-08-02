/*
    File:    print_char32.cpp
    Created: 19 January 2017 at 10:40 Moscow time
    Author:  Гаврилов Владимир Сергеевич
    E-mails: vladimir.s.gavrilov@gmail.com
             gavrilov.vladimir.s@mail.ru
             gavvs1977@yandex.ru
*/

#include <string>
#include <map>
#include "../include/print_char32.h"
#include "../include/char_conv.h"

static const std::map<char32_t, std::string> esc_char_strings = {
    {U'\'',   R"~(U'\'')~"},   {U'\"', R"~(U'\"')~"},     {U'\?', R"~(U'\?')~"},
    {U'\\',   R"~(U'\\')~"},   {U'\0', R"~(U'\0')~"},     {U'\a', R"~(U'\a')~"},
    {U'\b',   R"~(U'\b')~"},   {U'\f', R"~(U'\f')~"},     {U'\n', R"~(U'\n')~"},
    {U'\r',   R"~(U'\r')~"},   {U'\t', R"~(U'\t')~"},     {U'\v', R"~(U'\v')~"},
    {U'\x01', R"~(U'\x01')~"}, {U'\x02', R"~(U'\x02')~"}, {U'\x03', R"~(U'\x03')~"},
    {U'\x04', R"~(U'\x04')~"}, {U'\x05', R"~(U'\x05')~"}, {U'\x06', R"~(U'\x06')~"},
    {U'\x0e', R"~(U'\x0e')~"}, {U'\x0f', R"~(U'\x0f')~"}, {U'\x10', R"~(U'\x10')~"},
    {U'\x11', R"~(U'\x11')~"}, {U'\x12', R"~(U'\x12')~"}, {U'\x13', R"~(U'\x13')~"},
    {U'\x14', R"~(U'\x14')~"}, {U'\x15', R"~(U'\x15')~"}, {U'\x16', R"~(U'\x16')~"},
    {U'\x17', R"~(U'\x17')~"}, {U'\x18', R"~(U'\x18')~"}, {U'\x19', R"~(U'\x19')~"},
    {U'\x1a', R"~(U'\x1a')~"}, {U'\x1b', R"~(U'\x1b')~"}, {U'\x1c', R"~(U'\x1c')~"},
    {U'\x1d', R"~(U'\x1d')~"}, {U'\x1e', R"~(U'\x1e')~"}, {U'\x1f', R"~(U'\x1f')~"}
};

std::string show_char32(const char32_t c)
{
    std::string result;
    auto it = esc_char_strings.find(c);
    if(it != esc_char_strings.end()){
        result = it->second;
    }else{
        result = "U\'" + char32_to_utf8(c) + '\'';
    }
    return result;
}

void print_char32(const char32_t c)
{
    std::string s = show_char32(c);
    printf("%s", s.c_str());
}