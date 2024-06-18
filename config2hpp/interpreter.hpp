#ifndef _INTERPRETER_H_
#define _INTERPRETER_H_

#include <iostream>
#include <fstream>
#include <string>

#include <regex>

#define VARIABLE_TYPE_NORMAL     1
#define VARIABLE_TYPE_NORMAL_RAW 2
#define VARIABLE_TYPE_GROUP      3
#define VARIABLE_TYPE_GROUP_END  4
#define VARIABLE_TYPE_WARNING    5

typedef struct {
    int type;

    std::string name;
    std::string value;
}parsed_data_t;

std::string convert_to_header_macro_name(std::string file_name);

std::string remove_comment(std::string line);
std::string trim_line(std::string line);
bool skip_line(std::string line);
bool interpret_line(std::string line , parsed_data_t &data);

void set_global_group(const parsed_data_t &data);
std::string convert_to_macro(const parsed_data_t &data);


#endif