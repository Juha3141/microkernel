#include "interpreter.hpp"

int main(int argc , char **argv) {
    if(argc != 3) {
        std::cout << "usage : config2hpp [input config file] [output hpp file]\n";
        return -1;
    }
    std::ifstream config_file;
    std::ofstream hpp_output;
    config_file.open(argv[1]);
    hpp_output.open(argv[2] , std::ios::trunc);
    if(!config_file.is_open()) {
        std::cout << "cannot open file" << argv[1] << "\n";
        return -1;
    }

    config_file.seekg(0 , std::ios::end);
    int file_size = config_file.tellg();
    config_file.seekg(0 , std::ios::beg);

    // header
    std::string macro_name = convert_to_header_macro_name(argv[2]);
    hpp_output << "#ifndef " << macro_name << "\n";
    hpp_output << "#define " << macro_name << "\n\n";
    
    int line_number = 0;
    while(!config_file.eof()) {
        std::string line;
        parsed_data_t data;
        std::getline(config_file , line);
        // std::cout << "-------- line " << line_number << " --------\n";

        // remove all the comments
        std::string trimmed_line = trim_line(remove_comment(line));
        // std::cout << trimmed_line << " \n";
        if(skip_line(trimmed_line) == true) continue;
        if(interpret_line(trimmed_line , data) == false) {
            std::cout << "line " << line_number << " : regex not matched!\n";
            break;
        }
        
        set_global_group(data);

        std::string macro = convert_to_macro(data);
        // std::cout << "C++ macro : " << macro << "\n";
        if(macro.length() > 0) {
            hpp_output << macro << "\n";
        }
        line_number++;
    } 
    hpp_output << "\n#endif";

    config_file.close();
    hpp_output.close();
}