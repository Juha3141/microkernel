#include "interpreter.hpp"

std::string convert_to_header_macro_name(std::string file_name) {
    std::transform(file_name.begin() , file_name.end() , file_name.begin() , ::toupper);
    int start_index = file_name.find_last_of("/");
    std::string without_path = file_name.substr(start_index+1);
    
    int index = without_path.find("." , 0);
    
    return "_CONFIG_"+without_path.substr(0 , index)+"_";
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
    std::string syntax_regex[] = {\
        "\\[([\\w]+)\\][ ]*\\=[ ]*([\\w]+)[ ]*\\{[ ]*" , /* Grouping */\
        "\\[([\\w]+)\\][ ]*\\=[ ]*([\\w]+)[ ]*" , /* Grouping with no contents*/\
        "([\\w]+)[ ]*\\=[ ]*(.+)[ ]*" , /* Normal */\
        "\\@([\\w]+)[ ]*\\=[ ]*(.+)[ ]*" , /* Raw Normal */\
        "\\@\\@([\\w]+)[ ]*\\=[ ]*(.+)[ ]*" , /* Raw Normal Complete */\
        "\\}[ ]*WARNING[ ]*\\=[ ]*(.+)" , /* Warning */\
        "\\}" , /* End */\
        "\\[([\\w]+)\\][ ]*\\=[ ]*([\\w]+)[ ]*\\{[ ]*\\}[ ]*WARNING[ ]*\\=[ ]*(.+)" , /* Grouping with warning*/\
        "\\[([\\w]+)\\][ ]*\\=[ ]*\\{[ ]*" , /* Grouping without CONFIG_USE header(pure namespace) */\
    };

int interpret_line(std::string one_line , std::vector<parsed_data_t>&parsed_data_list) {
    REGEX_DEFINITIONS
    int match_result = -1;
    for(int i = 0; i < sizeof(syntax_regex)/sizeof(std::regex *); i++) {
        if(std::regex_match(one_line , std::regex(syntax_regex[i])) == true) {
            match_result = i;
            break;
        }
    }
    if(match_result == -1) return 0;
    std::smatch match;

    if(std::regex_search(one_line , match , std::regex(syntax_regex[match_result])) == false) return false;
    /*
    for(int i = 0; i < match.size(); i++) {
        std::cout << "match : " << match[i].str() << "\n";
    }
    */
    switch(match_result) {
        case 0:
            parsed_data_list.push_back({
                .type = VARIABLE_TYPE_GROUP , 
                .name = match[1].str() , 
                .value = match[2].str()
            });
            break;
        case 1:
            parsed_data_list.push_back({
                .type = VARIABLE_TYPE_GROUP , 
                .name = match[1].str() , 
                .value = match[2].str() , 
            });
            break;
        case 2:
            parsed_data_list.push_back({
                .type = VARIABLE_TYPE_NORMAL , 
                .name = match[1].str() , 
                .value = match[2].str() , 
            });
            break;
        case 3:
            parsed_data_list.push_back({
                .type = VARIABLE_TYPE_NORMAL_RAW , 
                .name = match[1].str() , 
                .value = match[2].str() , 
            });
            break;
        case 4:
            parsed_data_list.push_back({
                .type = VARIABLE_TYPE_NORMAL_COMPLETE_RAW , 
                .name = match[1].str() , 
                .value = match[2].str() , 
            });
            break;
        case 5:
            parsed_data_list.push_back({
                .type = VARIABLE_TYPE_WARNING , 
                .name = "WARNING" , 
                .value = match[1].str() , 
            });
            break;
        case 6: 
            parsed_data_list.push_back({
                .type = VARIABLE_TYPE_GROUP_END , 
                .name = "" , 
                .value = "" , 
            });
            break;
        case 7: 
            parsed_data_list.push_back({
                .type = VARIABLE_TYPE_GROUP , 
                .name = match[1].str() , 
                .value = match[2].str() , 
            });
            parsed_data_list.push_back({
                .type = VARIABLE_TYPE_WARNING , 
                .name = "WARNING" , 
                .value = match[3].str() , 
            });
            break;
        case 8: 
            parsed_data_list.push_back({
                .type = VARIABLE_TYPE_GROUP , 
                .name = match[1].str() , 
                .value = "enabled_no_use" , 
            });
            break;
    }
    return parsed_data_list.size();
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

using ull = unsigned long long;

void translate_metric(std::string &value) {
    std::pair<std::regex,ull> metric_regex[] = {
        std::make_pair(std::regex("[ ]*([0-9]+)KB?[ ]*") , ((ull)1<<10)) , /* KB */
        std::make_pair(std::regex("[ ]*([0-9]+)MB?[ ]*") , ((ull)1<<20)) , /* MB */
        std::make_pair(std::regex("[ ]*([0-9]+)GB?[ ]*") , ((ull)1<<30)) , /* GB */
        std::make_pair(std::regex("[ ]*([0-9]+)TB?[ ]*") , ((ull)1<<40)) , /* TB */
        std::make_pair(std::regex("[ ]*([0-9]+)PB?[ ]*") , ((ull)1<<50)) , /* PB */
        std::make_pair(std::regex("[ ]*([0-9]+)EB?[ ]*") , ((ull)1<<60)) , /* EB */
    };
    bool found = false;
    int match_id = -1;
    for(int i = 0; i < sizeof(metric_regex)/sizeof(std::pair<std::regex,ull>); i++) {
        if(std::regex_match(value , metric_regex[i].first) == true) {
            match_id = i;
            break;
        }
    }
    if(match_id == -1) return;

    std::smatch match;
    if(std::regex_search(value , match , metric_regex[match_id].first) == false) return;

    value = std::to_string(std::stoull(match[1].str())*metric_regex[match_id].second);
}

void translate_metric_in_list_items(std::string &value) {
    if(value[0] != '{') return;

    int val_len = value.size();
    value = value.substr(1 , val_len-2);

    std::string new_str = "{";
    size_t prev_off = 0;
    size_t sub_off = 0;
    while((sub_off = value.find("," , sub_off)) != std::string::npos) {
        std::string item = trim_line(value.substr(prev_off , sub_off-prev_off));
        translate_metric(item);
        new_str += item+",";
        
        sub_off++;
        prev_off = sub_off;
    }
    std::string last_item = trim_line(value.substr(prev_off , val_len-2-prev_off));
    translate_metric(last_item);
    new_str += last_item+"}";
    
    value = new_str;
}

std::string convert_to_macro(parsed_data_t data) {
    // std::cout << "type : " << data.type << "\n";
    // std::cout << "current group : " << global_group << "(" << global_group_value << ")\n";
    std::string macro = "";

    translate_metric(data.value);
    translate_metric_in_list_items(data.value);
    switch(data.type) {
        case VARIABLE_TYPE_GROUP:
            macro += "\n/****************** CONFIG_"+data.name+" ******************/\n";
            if(data.value == "disabled")      macro += "// ";
            if(data.value == "disabled_full") macro += "// ";
            if(data.value == "enabled")       macro += ("#define CONFIG_USE_"+data.name);
            break;
        case VARIABLE_TYPE_NORMAL: 
            if(global_group_value == "disabled_full") macro = "// ";
            if(global_group != "") macro += ("#define CONFIG_"+global_group+"_"+data.name+" "+data.value);
            else                   macro += ("#define CONFIG_"+data.name+" "+data.value);
            break;
        case VARIABLE_TYPE_NORMAL_RAW:
            if(global_group_value == "disabled_full") macro = "// ";
            macro += ("#define CONFIG_"+data.name+" "+data.value);
            break;
        case VARIABLE_TYPE_NORMAL_COMPLETE_RAW:
            if(global_group_value == "disabled_full") macro = "// ";
            macro += ("#define "+data.name+" "+data.value);
            break;
        case VARIABLE_TYPE_WARNING:
            macro += ("#define CONFIG_WARNING_NO_"+global_group+" WARNING("+data.value+");");
            break;
            
    }

    return macro;
}
