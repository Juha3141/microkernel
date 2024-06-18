#include "interpreter.hpp"

std::string convert_to_header_macro_name(std::string file_name) {
    std::transform(file_name.begin() , file_name.end() , file_name.begin() , ::toupper);
    int index = file_name.find("." , 0);
    
    return "_CONFIG_"+file_name.substr(0 , index)+"_";
}

std::string remove_comment(std::string line) {
    if(line.length() == 0) return line;
    int comment_index = line.find_first_of('#');
    if(comment_index == std::string::npos) return line;
    return line.substr(0 , comment_index);
}

std::string trim_line(std::string line) {
    int front_space_idx = -1;
    int rear_space_idx = -1;
    for(front_space_idx = 0;; front_space_idx++) {
        if(line[front_space_idx] != ' ') break;
    }
    for(rear_space_idx = line.length()-1; rear_space_idx >= 0; rear_space_idx--) {
        if(line[rear_space_idx] != ' ') break;
    }
    return line.substr(front_space_idx , rear_space_idx-front_space_idx+1);
}

bool skip_line(std::string line) {
    if(line.length() == 0) return true;
    for(int i = 0; line[i] != 0x00; i++) {
        if(line[i] != ' ') return false;
    }
    return true;
}

#define REGEX_DEFINITIONS \
    std::regex *syntax_regex[] = {\
        new std::regex("\\[([\\w]+)\\][ ]*\\=[ ]*([\\w]+)[ ]*\\{[ ]*") , /* Grouping */\
        new std::regex("([\\w]+)[ ]*\\=[ ]*(.+)[ ]*") , /* Normal */\
        new std::regex("\\@([\\w]+)[ ]*\\=[ ]*(.+)[ ]*") , /* Raw Normal */\
        new std::regex("\\}[ ]*WARNING[ ]*\\=[ ]*(.+)") , /* Warning */\
        new std::regex("\\}") , /* End */\
    };

bool interpret_line(std::string one_line , parsed_data_t &data) {
    REGEX_DEFINITIONS
    int match_result = -1;
    for(int i = 0; i < sizeof(syntax_regex)/sizeof(std::regex *); i++) {
        if(std::regex_match(one_line , *syntax_regex[i]) == true) {
            match_result = i;
            break;
        }
    }
    if(match_result == -1) return false;
    std::smatch match;

    if(std::regex_search(one_line , match , *syntax_regex[match_result]) == false) return false;
    /*
    for(int i = 0; i < match.size(); i++) {
        std::cout << "match : " << match[i].str() << "\n";
    }
    */
    switch(match_result) {
        case 0:
            data.type = VARIABLE_TYPE_GROUP;
            data.name = match[1].str();
            data.value = match[2].str();
            break;
        case 1:
            data.type = VARIABLE_TYPE_NORMAL;
            data.name = match[1].str();
            data.value = match[2].str();
            break;
        case 2:
            data.type = VARIABLE_TYPE_NORMAL_RAW;
            data.name = match[1].str();
            data.value = match[2].str();
            break;
        case 3:
            data.type = VARIABLE_TYPE_WARNING;
            data.name = "WARNING";
            data.value = match[1].str();
            break;
        case 4: 
            data.type = VARIABLE_TYPE_GROUP_END;
            data.name = "";
            data.value = "";
            break;
    }
    return true;
}

static std::string global_group = "";
static std::string global_group_value = "";

void set_global_group(const parsed_data_t &data) {
    if(data.type == VARIABLE_TYPE_GROUP_END||data.type == VARIABLE_TYPE_WARNING) {
        global_group = "";
        global_group_value = "";
        return; 
    }
    if(data.type == VARIABLE_TYPE_GROUP) {
        global_group = data.name;
        global_group_value = data.value;
        return;
    }
}

std::string convert_to_macro(const parsed_data_t &data) {
    // std::cout << "type : " << data.type << "\n";
    // std::cout << "current group : " << global_group << "(" << global_group_value << ")\n";
    std::string macro = "";
    switch(data.type) {
        case VARIABLE_TYPE_GROUP:
            macro += "\n/****************** CONFIG_"+global_group+" ******************/\n";
            if(data.value.compare("disabled") == 0) macro += "// ";
            if(data.value.compare("disabled_full") == 0) macro += "// ";
            macro += ("#define CONFIG_USE_"+data.name);
            break;
        case VARIABLE_TYPE_NORMAL:
            if(global_group_value.compare("disabled_full") == 0) macro = "// ";
            if(global_group != "") macro += ("#define CONFIG_"+global_group+"_"+data.name+" "+data.value);
            else                   macro += ("#define CONFIG_"+data.name+" "+data.value);
            break;
        case VARIABLE_TYPE_NORMAL_RAW:
            if(global_group_value.compare("disabled_full") == 0) macro = "// ";
            macro += ("#define CONFIG_"+data.name+" "+data.value);
            break;
        case VARIABLE_TYPE_WARNING:
            macro += ("#define CONFIG_WARNING_NO_"+global_group+" WARNING("+data.value+");");
            break;
            
    }

    return macro;
}