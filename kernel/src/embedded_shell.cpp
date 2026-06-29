#include <kernel/embedded_shell.hpp>

static void read_input(InputReader& kbd_reader , String &str) {
    str.reserve(128);
    while(1) {
        input_event event;
        if(!kbd_reader.read(event)) continue;
        // ignore any other input types for now
        if(event.type != INPUT_TYPE_KEYDOWN) continue;
        
        byte data = event.data & 0xff;
        if(data == '\n') {
            debug::out::printf("%c" , data);
            break;
        }
        else if(data == '\t') {
            /* Action reserved for later */
        }
        else if(data == '\b') {
            if(!str.backspace()) continue;
            // Remove one character from the screen
            debug::out::printf("\b \b");
        }
        else {
            str += data;
            debug::out::printf("%c" , data);
        }
    }
    // remove the unnecessary spaces at the rear of the string
    for(int i = str.size()-1; i >= 0; i--) {
        if(str[i] == ' ') str[i] = '\0';
        else break;
    }
}

static bool parse_arguments(const String& full_str , LinkedList<String*>&ll) {
    bool success = true;
    max_t prev_idx = 0;
    for(max_t i = 0; full_str[i] != 0; i++) {
        if(full_str[i] == ' ') {
            if(i == prev_idx) { prev_idx = i+1; continue; }

            String *new_str = new String(full_str.substr(prev_idx , i-prev_idx));
            ll.add_rear(new_str);
            prev_idx = i+1;
        }
    }
    ll.add_rear(new String(full_str.substr(prev_idx)));
    return success;
}

static void release_str_ll(LinkedList<String*>&ll) {
    auto *ptr = ll.get_start_node();
    while(ptr != nullptr) {
        delete ptr->object;

        ptr = ptr->next;
    }
}

typedef int (*shell_command_func_t)(file_t* &current_dir , int argc , char **argv);

struct shell_commands_t {
    const char *cmd;
    const char *help;

    shell_command_func_t func;    
};

shell_commands_t commands_list[] = {
    {"help"   , "show commands" , eshell::cmd::help} , 
    {"cd" , "enter a directory" , eshell::cmd::cd} , 
    {"ls" , "list files in the current directory" , eshell::cmd::ls} , 
    {"read"   , "read a file" , eshell::cmd::read} , 
    {"clear"  , "clears the screen" , eshell::cmd::clear} , 
    {"echo"   , "echo" , eshell::cmd::echo} , 
    {"mem"    , "show mem usage" , eshell::cmd::mem} , 
};

void eshell::start() {
    debug::out::printf("Entering the Embedded Shell\n");
    debug::out::printf(DEBUG_INFO , "  _______ _            __  __ _                _                        _   _____           _           _   \n");
    debug::out::printf(DEBUG_INFO , " |__   __| |          |  \\/  (_)              | |                      | | |  __ \\         (_)         | |  \n");
    debug::out::printf(DEBUG_INFO , "    | |  | |__   ___  | \\  / |_  ___ _ __ ___ | | _____ _ __ _ __   ___| | | |__) | __ ___  _  ___  ___| |_ \n");
    debug::out::printf(DEBUG_INFO , "    | |  | '_ \\ / _ \\ | |\\/| | |/ __| '__/ _ \\| |/ / _ \\ '__| '_ \\ / _ \\ | |  ___/ '__/ _ \\| |/ _ \\/ __| __|\n");
    debug::out::printf(DEBUG_INFO , "    | |  | | | |  __/ | |  | | | (__| | | (_) |   <  __/ |  | | | |  __/ | | |   | | | (_) | |  __/ (__| |_ \n");
    debug::out::printf(DEBUG_INFO , "    |_|  |_| |_|\\___| |_|  |_|_|\\___|_|  \\___/|_|\\_\\___|_|  |_| |_|\\___|_| |_|   |_|  \\___/| |\\___|\\___|\\__|\n");
    debug::out::printf(DEBUG_INFO , "                                                                                          _/ |              \n");
    debug::out::printf(DEBUG_INFO , "                                                                                         |__/               \n");
    InputReader kbd_reader;
    kbd_reader.open("keyboard");
    String current_dir_name("@");
    file_t *current_dir_f = vfs::open({current_dir_name.c_str() , nullptr} , FILE_OPEN_READONLY);
    eshell::cmd::help(current_dir_f , 1 , nullptr);
    
    while(1) {
        String str;
        vfs::get_full_filename(current_dir_f , current_dir_name);
        debug::out::printf(DEBUG_INFO , "eshell: %s > " , current_dir_name.c_str());
        read_input(kbd_reader , str);

        LinkedList<String*>parsed_arguments;
        parse_arguments(str , parsed_arguments);

        if(parsed_arguments.size() == 0) continue;

        auto *ptr = parsed_arguments.get_start_node();

        // construct argv for commands
        char **argv = (char **)memory::pmem_alloc(parsed_arguments.size()*sizeof(char *));
        max_t i = 0;
        while(ptr != nullptr) {
            argv[i++] = (char *)ptr->object->c_str();
            ptr = ptr->next;
        }
        /**** Handle the command ****/
        int ret = 0x7fffffff;
        for(int i = 0; i < sizeof(commands_list)/sizeof(shell_commands_t); i++) {
            if(strcmp(commands_list[i].cmd , argv[0]) == 0) {
                ret = commands_list[i].func(current_dir_f , parsed_arguments.size() , argv);
                break;
            }
        }
        if(ret == 0x7fffffff) {
            debug::out::printf("Command named \"%s\" does not exist\n" , argv[0]);
        }

        memory::pmem_free(argv);
        release_str_ll(parsed_arguments);
    }
}

int eshell::cmd::help(file_t* &current_dir , int argc , char **argv) {
    int cmd_max_char = 0;
    const int cmd_count = sizeof(commands_list)/sizeof(shell_commands_t);
    for(int i = 0; i < cmd_count; i++) {
        cmd_max_char = max(cmd_max_char , strlen(commands_list[i].cmd));
    }

    for(int i = 0; i < cmd_count; i++) {
        int space_count = cmd_max_char-strlen(commands_list[i].cmd);
        debug::out::printf("%s" , commands_list[i].cmd);
        for(int i = 0; i < space_count; i++) {
            debug::out::printf(" ");
        }
        debug::out::printf("  %s\n" , commands_list[i].help);
    }
    return 0;
}

int eshell::cmd::cd(file_t* &current_dir , int argc , char **argv) {
    if(argc > 2) debug::out::printf("cd : too many arguments!\n");
    if(argc == 1) {
        String full_name;
        vfs::get_full_filename(current_dir , full_name);
        return 0;
    }

    file_t *new_dir = vfs::open({argv[1] , current_dir} , FILE_OPEN_READONLY);
    if(new_dir == nullptr) {
        new_dir = vfs::open({argv[1] , nullptr} , FILE_OPEN_READONLY);
    }
    debug::out::printf("new_dir : 0x%llx\n" , new_dir);
    if(new_dir == nullptr) {
        debug::out::printf("\"%s\" : No such directory\n" , argv[1]);
        return -1;
    }
    if(new_dir->info->file_type != FILE_TYPE_DIRECTORY) {
        debug::out::printf("\"%s\" is not a directory\n" , argv[1]);
        return -1;
    }
    vfs::close(current_dir);
    current_dir = new_dir;
    return 0;
}

int eshell::cmd::ls(file_t* &current_dir , int argc , char **argv) {
    int file_count = vfs::read_directory(current_dir);
    debug::out::printf("%d files\n" , file_count);
    max_t max_file_name_len = 0;
    
    auto ptr = current_dir->info->file_list->get_start_node();
    while(ptr != nullptr) {
        max_file_name_len = max(strlen(ptr->object->name) , max_file_name_len);
        ptr = ptr->next;
    }

    ptr = current_dir->info->file_list->get_start_node();
    while(ptr != nullptr) {
        max_t f_len = strlen(ptr->object->name);
        debug::out::printf("%s" , ptr->object->name);
        for(int i = 0; i < max_file_name_len-f_len+3; i++) { debug::out::printf(" "); }

        if(ptr->object->info->file_type == FILE_TYPE_DIRECTORY) {
            debug::out::printf("DIR\n");
        }
        else if(ptr->object->info->file_type == FILE_TYPE_DEVICE_FILE) {
            debug::out::printf("DEV\n");
        }
        else {
            debug::out::printf("%lld(%d)\n" , ptr->object->info->file_size , ptr->object->info->file_type);
        }
        ptr = ptr->next;
    }
    return 0;
}

int eshell::cmd::read(file_t* &current_dir , int argc , char **argv) {
    if(argc != 2) debug::out::printf("Usage : read [file name]\n");

    file_t *file = vfs::open({argv[1] , current_dir} , FILE_OPEN_READONLY);
    if(file == nullptr) {
        debug::out::printf("Error : file \"%s\" not found\n" , argv[1]);
        return -1;
    }

    max_t file_size = file->info->file_size;
    debug::out::printf("file size : %lld\n" , file_size);
    debug::out::printf("--------------------------------\n");
    char buffer[516];
    for(max_t offset = 0; offset <= file_size; offset += 512) {
        vfs::read(file , 512 , buffer);
        debug::out::printf("%s\n" , buffer);
    }
    debug::out::printf("--------------------------------\n");
    memory::pmem_free(buffer);
    vfs::close(file);
    return 0;
}

int eshell::cmd::clear(file_t* &current_dir , int argc , char **argv) {
    debug::out::clear_screen();
    return 0;
}

int eshell::cmd::echo(file_t* &current_dir , int argc , char **argv) {
    for(int i = 1; i < argc; i++) {
        debug::out::printf("%s" , argv[i]);
        if(i != argc-1) debug::out::printf(" ");
    }
    debug::out::printf("\n");
    return 0;
}

int eshell::cmd::mem(file_t* &current_dir , int argc , char **argv) {
    max_t usage = memory::pmem_usage();
    debug::out::printf("total : %lldMB\n" , memory::pmem_total_size()/1024/1024);
    debug::out::printf("usage : %lld.%d%dkB\n" , usage/1024 , (usage*10/1024)%10 , (usage*100/1024)%10);
    return 0;
}