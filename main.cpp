// Libraries need to be included
#include <algorithm>
#include <iostream>
#include <fstream>      
#include <sstream>      
#include <vector> 
#include <string>     
#include <cstring>  
#include <unordered_map>
#include <unistd.h>     
#include <sys/wait.h>   
#include <sys/stat.h>  
#include <sys/types.h>  
#include <cstdlib>      

using namespace std;

// Global variables
vector<string> builtins = {"cd", "exit", "history", "help", "echo", "alias", "enable", "let","logout", "read", "printf", "type"};
vector<string> custom_commands = {"byte-mkdir", "byte-calc", "byte-bookmark", "byte-lastdir"};
vector<string> command_history;
unordered_map<string, string> aliases;
string last_working_dir="";
unordered_map<string, string> bookmarks;
const int HISTORY_LIMIT = 50;
const string ALIAS_FILE = ".byte_aliases";

//Input Func -> Reads Input
string read_input() {
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    string input;
    cout << "\033[1;32mBYTE@\033[1;34m" << cwd << "\033[0m >> ";
    getline(cin, input);
    if (!input.empty()) {
        command_history.push_back(input);
    }
    if (command_history.size() > HISTORY_LIMIT)
        command_history.erase(command_history.begin());

    return input;
}

vector<char*> convert_to_cstr_args(vector<string>& tokens) {
    vector<char*> args;
    for (auto& token : tokens)
        args.push_back(&token[0]); 
    args.push_back(nullptr); 
    return args;
}

vector<string> tokenize(const string& input) {
    vector<string> tokens;
    istringstream iss(input);
    string token;
    bool inQuotes = false;
    char quoteChar = '\0';
    string current;

    for (char c : input) {
        if ((c == '\'' || c == '"')) {
            if (!inQuotes) {
                inQuotes = true;
                quoteChar = c;
            } else if (quoteChar == c) {
                inQuotes = false;
            }
            current += c;
        } else if (isspace(c) && !inQuotes) {
            if (!current.empty()) {
                tokens.push_back(current);
                current.clear();
            }
        } else {
            current += c;
        }
    }

    if (!current.empty()) {
        tokens.push_back(current);
    }

    return tokens;
}

//Evaluation func -> To evaluate basic functions for simple calculations.
int eval_simple_expression(const string& expr) {
    istringstream iss(expr);
    int result, num;
    char op = '+';

    iss >> result;  // read first number

    while (iss >> op >> num) {
        switch (op) {
            case '+': result += num; break;
            case '-': result -= num; break;
            case '*': result *= num; break;
            case '/': 
                if (num == 0) throw runtime_error("division by zero");
                result /= num;
                break;
            default:
                throw runtime_error("unsupported operator");
        }
    }

    return result;
}

// Built-in Funcs
int shell_cd(char **args) {
    if (args[1] == NULL) {
        cerr << "ByteShell: expected argument to \"cd\"\n";
    } else {
        if (chdir(args[1]) != 0) {
            perror("ByteShell");
        }
    }
    return 1;
}   

void shell_help() {
    cout << "\n===================== BYTE SHELL HELP =====================\n";
    cout << "Welcome to ByteShell - your customizable command line tool\n\n";
    
    cout << "\nBuilt-in Commands:\n";
    cout << "  cd <path>             : Change current directory\n";
    cout << "  exit                  : Exit ByteShell\n";
    cout << "  history               : Show command history\n";
    cout << "  help                  : Display this help message\n";
    cout << "  echo <text>           : Print text to the screen\n";
    cout << "  printf <format> <val> : Print formatted output\n";
    cout << "  read <var>            : Read input from user\n";
    cout << "  logout                : Exit ByteShell (alias for exit)\n";
    cout << "  let <expr>            : Evaluate arithmetic expressions\n";
    cout << "  alias name='cmd'      : Create shortcut for a command\n";
    cout << "  enable <builtin>      : Enable/disable builtins\n";
    cout << "  type <command>        : Show if command is built-in or external\n";

    cout << "\nCustom ByteShell Commands:\n";
    cout << "  byte-mkdir <path>     : Create folder at any path without leaving current dir\n";
    cout << "  byte-calc <expr>      : Evaluate arithmetic expression (e.g., 4+5*3)\n";
    cout << "  byte-lastdir          : Jump back to your last working directory\n";
    cout << "  byte-book <name> <path> : Bookmark a directory with a name\n";
    cout << "  byte-goto <name>      : Navigate to a bookmarked path\n";
    cout << "  byte-bookmarks        : List all saved bookmarks\n";
    cout << "\n===========================================================\n\n";
}

void shell_echo(const vector<string>& tokens) {
    for (size_t i = 1; i < tokens.size(); ++i)
        cout << tokens[i] << " ";
    cout << endl;
}

void shell_let(const vector<string>& tokens) {
    if (tokens.size() < 2) {
        cerr << "let: missing expression\n";
        return;
    }

    string full_expr;
    for (size_t i = 1; i < tokens.size(); ++i)
        full_expr += tokens[i];

    // Handle optional variable assignment: e.g., x=2+3
    size_t eq_pos = full_expr.find('=');
    string expr;
    if (eq_pos != string::npos) {
        expr = full_expr.substr(eq_pos + 1);  // get part after '='
    } else {
        expr = full_expr;
    }

    try {
        int result = eval_simple_expression(expr);
        cout << result << endl;
    } catch (...) {
        cerr << "let: error evaluating expression\n";
    }
}


void shell_logout() {
    cout << "Logging out of ByteShell...\n";
    exit(0);
}

void shell_read() {
    string var;
    cout << "Enter input: ";
    getline(cin, var);
    cout << "You entered: " << var << endl;
}

void shell_printf(const vector<string>& tokens) {
    for (size_t i = 1; i < tokens.size(); ++i)
        cout << tokens[i] << " ";
    cout << endl;
}

void shell_type(const string& cmd) {
    if (find(builtins.begin(), builtins.end(), cmd) != builtins.end()) {
        cout << cmd << " is a shell builtin\n";
    } else if (find(custom_commands.begin(), custom_commands.end(), cmd) != custom_commands.end()) {
        cout << cmd << " is a ByteShell custom command\n";
    } else {
        cout << cmd << " is an external command or not found\n";
    }
}

void load_history() {
    ifstream in(".byte_history");
    string line;
    while (getline(in, line))
        command_history.push_back(line);
}

void load_last_working_dir() {
    ifstream in(".byte_lastdir");
    getline(in, last_working_dir);
}

void save_history() {
    ofstream out(".byte_history");
    for (auto& cmd : command_history)
        out << cmd << "\n";
}

void save_last_working_dir() {
    ofstream out(".byte_lastdir");
    out << last_working_dir;
}

void save_bookmarks_to_file() {
    ofstream out(".byte_bookmarks");
    for (auto& [name, path] : bookmarks) {
        out << name << " " << path << "\n";
    }
}

void load_bookmarks_from_file() {
    ifstream in(".byte_bookmarks");
    string name, path;
    while (in >> name >> path) {
        bookmarks[name] = path;
    }
}

void save_aliases_to_file() {
    ofstream out(ALIAS_FILE);
    for (const auto& pair : aliases) {
        out << pair.first << "=" << pair.second << '\n';
    }
}

void load_aliases_from_file() {
    ifstream in(ALIAS_FILE);
    string line;
    while (getline(in, line)) {
        size_t eq = line.find('=');
        if (eq != string::npos) {
            string key = line.substr(0, eq);
            string value = line.substr(eq + 1);
            aliases[key] = value;
        }
    }
}


// Execute Func
void execute_command(vector<string>& tokens) {
    if (tokens.empty()) return;

    if (tokens[0] == "exit") {
        cout << "Exiting Byteshell...\n";
        save_history();
        save_bookmarks_to_file();
        save_last_working_dir();
        exit(0);
    }

    if (tokens[0] == "cd") {
        vector<char*> args = convert_to_cstr_args(tokens);
        shell_cd(args.data());  // call your cd function
        return;
    }

    if (tokens[0] == "history") {
        for (int i = 0; i < command_history.size(); ++i) {
            cout << i + 1 << "  " << command_history[i] << endl;
        }
        return;
    }

    if (tokens[0] == "help") {
        shell_help();
        return;
    }

    if (tokens[0] == "echo") {
        shell_echo(tokens);
        return;
    }

    if (tokens[0] == "let") {
        shell_let(tokens);
        return;
    }

    if (tokens[0] == "logout") {
        shell_logout();
        return;
    }

    if (tokens[0] == "read") {
        shell_read();
        return;
    }

    if (tokens[0] == "printf") {
        shell_printf(tokens);
        return;
    }

    if (tokens[0] == "type") {
        if (tokens.size() >= 2)
            shell_type(tokens[1]);
        else
            cerr << "type: missing argument\n";
        return;
    }

    if (tokens[0] == "alias") {
        if (tokens.size() == 1) {
            for (const auto& pair : aliases) {
                cout << "alias " << pair.first << "='" << pair.second << "'\n";
            }
        } else {
            for (size_t i = 1; i < tokens.size(); ++i) {
                string alias_def = tokens[i];
                size_t eq = alias_def.find('=');
                if (eq != string::npos) {
                    string name = alias_def.substr(0, eq);
                    string value = alias_def.substr(eq + 1);
    
                    // Remove quotes if present
                    if (!value.empty() && value.front() == '\'') value = value.substr(1);
                    if (!value.empty() && value.back() == '\'') value.pop_back();
    
                    aliases[name] = value;
                    save_aliases_to_file();
                } else {
                    cerr << "Invalid alias format. Use alias name='value'\n";
                }
            }
        }
        return;
    }  

    if (tokens[0] == "byte-mkdir") {
        if (tokens.size() < 2) {
            cout << "Usage: byte-mkdir <full_path>\n";
        } else {
            if (mkdir(tokens[1].c_str(), 0755) == 0) {
                cout << "Directory created at " << tokens[1] << endl;
            } else {
                perror("mkdir failed");
            }
        }
        return;
    }
    
    if(tokens[0] == "byte-bookmark") {
        if (tokens.size() < 3) {
            cout << "Usage: byte-bookmark add <name>\n       byte-bookmark go <name>\n";
        } else if (tokens[1] == "add") {
            char cwd[1024];
            getcwd(cwd, sizeof(cwd));
            bookmarks[tokens[2]] = string(cwd);
            save_bookmarks_to_file();
            cout << "Added bookmark " << tokens[2] << " for " << cwd << endl;
        } else if (tokens[1] == "go") {
            if (bookmarks.find(tokens[2]) != bookmarks.end()) {
                if (chdir(bookmarks[tokens[2]].c_str()) != 0)
                    perror("byte-bookmark go");
            } else {
                cout << "Bookmark not found.\n";
            }
        }
        return;
    }
    
    if(tokens[0] == "byte-calc") {
        if (tokens.size() < 2) {
            cout << "Usage: byte-calc <expression>\n";
            return;
        }
    
        string expr;
        for (int i = 1; i < tokens.size(); ++i)
            expr += tokens[i];
    
        try {
            int result = eval_simple_expression(expr);
            cout << "Result: " << result << endl;
        } catch (...) {
            cout << "Error evaluating expression.\n";
        }
        return;
    }

    if (tokens[0] == "byte-lastdir") {
        load_last_working_dir();
        if (!last_working_dir.empty()) {
            if (access(last_working_dir.c_str(), F_OK) == 0) {
                if (chdir(last_working_dir.c_str()) != 0) {
                    perror("chdir failed");
                }
            } else {
                cout << "Saved last working directory no longer exists.\n";
            }
        } else {
            cout << "No last working directory saved.\n";
        }
        return;
    }
    
    if (tokens[0] != "cd" && tokens[0] != "exit" && tokens[0] != "history") {
        char cwd[1024];
        getcwd(cwd, sizeof(cwd));
        last_working_dir = string(cwd);
        save_last_working_dir();
    }

    pid_t pid = fork();
    if (pid == 0) {
        vector<char*> args = convert_to_cstr_args(tokens);
        if (execvp(args[0], args.data()) == -1) {
            perror("Byteshell error");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("Fork failed");
    } else {
        int status;
        waitpid(pid, &status, 0);
    }
}

// Main Func
int main() {
    load_history();
    load_bookmarks_from_file();
    load_last_working_dir();
    load_aliases_from_file();

    while (true) {
        string input = read_input();
        vector<string> tokens = tokenize(input);

        // 🔁 Alias Expansion Logic
        if (!tokens.empty() && aliases.count(tokens[0])) {
            string alias_expansion = aliases[tokens[0]];
            vector<string> alias_tokens;
            istringstream iss(alias_expansion);
            string t;
            while (iss >> t) alias_tokens.push_back(t);
        
            // Replace the alias in tokens
            tokens.erase(tokens.begin()); // remove alias keyword
            tokens.insert(tokens.begin(), alias_tokens.begin(), alias_tokens.end());
        }       

        execute_command(tokens);
    }
    return 0;
}

