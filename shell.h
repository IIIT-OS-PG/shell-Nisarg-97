#include <string>
#include <iostream>
#include <unistd.h>
#include <vector>
#include <cstring>
#include <sys/types.h>
#include <sys/wait.h>
#include <sstream>
#include <fstream>
#include <fcntl.h>
using namespace std;


class command
{
    public :
    string command_name;
    vector<string> parameters;
    
    int fds[2];
};


class commands
{
    public :
vector<command> v_commands;
int command_count;
};
