#include "interpreter.hpp"

int main(int argc , char **argv) {
    if(argc != 5) {
        std::cout << "usage : config2hpp [input config file] [pre-header file] [output hpp file] [header guard name]\n";
        std::cout << "   + specify [pre-header file] as \"none\" if there's no pre-header file\n";
        return -1;
    }
    std::ifstream config_file;
    bool no_pre_header_file = false;
    std::ifstream pre_header_file;
    std::ofstream hpp_output;
    config_file.open(argv[1]);
    if(std::string(argv[2]) == "none") {
        no_pre_header_file = true;
    }
    pre_header_file.open(argv[2]);
    hpp_output.open(argv[3] , std::ios::trunc);
    if(!config_file.is_open()) {
        std::cout << "cannot open file " << argv[1] << "\n";
        return -1;
    }
    if(!no_pre_header_file && !pre_header_file.is_open()) {
        std::cout << "cannot open file " << argv[2] << "\n";
        return -1;
    }
    if(!hpp_output.is_open()) {
        std::cout << "cannot open file " << argv[3] << "\n";
        return -1;
    }

    config_file.seekg(0 , std::ios::end);
    int file_size = config_file.tellg();
    config_file.seekg(0 , std::ios::beg);

    // header
    std::string macro_name = argv[4];
    hpp_output << "#ifndef " << macro_name << "\n";
    hpp_output << "#define " << macro_name << "\n\n";
    if(!no_pre_header_file) {
        hpp_output << pre_header_file.rdbuf() << "\n";
    }
    
    int line_number = 0;
    while(!config_file.fail()) {
        std::string line;
        std::vector<parsed_data_t>data_list;
        std::getline(config_file , line);
        // std::cout << "-------- line " << line_number << " --------\n";

        // remove all the comments
        std::string trimmed_line = trim_line(remove_comment(line));
        // std::cout << trimmed_line << " \n";
        if(skip_line(trimmed_line) == true) continue;
        if(interpret_line(trimmed_line , data_list) == false) {
            std::cout << "line " << line_number << " : regex not matched!\n";
            break;
        }

        std::string macro;
        for(parsed_data_t &d : data_list) {
            macro += convert_to_macro(d)+'\n';
            set_global_group(d);
        }
        // std::cout << "C++ macro : " << macro << "\n";
        if(macro.length() > 0) {
            hpp_output << macro;
        }
        line_number++;
    } 
    hpp_output << "\n#endif";

    config_file.close();
    hpp_output.close();
}
