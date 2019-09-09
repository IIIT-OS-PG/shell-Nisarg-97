

#include "shell.h"

void close_pipes(int (*pipes)[2], int count){
    for (int i = 0; i < count; i++)
            {
                close(pipes[i][1]);
                close(pipes[i][0]);
            }
}
command parse_with_space(string S1,command instuct){

stringstream STR2(S1);
string S2;


    while (STR2.peek() == ' ')
                STR2.get();

            while(getline(STR2, S2, ' ')){
                
            instuct.parameters.push_back(S2);
        }
        instuct.command_name = instuct.parameters[0];
        return instuct;
}

commands parse_with_pipe(string input){
    
    commands command_list;
    command instuct;
    stringstream STR1(input);
    string S1;
            
    while (getline(STR1, S1, '|'))
    {
        instuct.parameters.clear();
        instuct=parse_with_space(S1,instuct);
        command_list.v_commands.push_back(instuct);
    }
    return command_list;
}

commands handle_redirection(commands commands,string f_name,string operators){

               const char *p = f_name.c_str();
                if (operators == ">")
                    commands.v_commands[0].fds[1]=open(p, O_WRONLY | O_CREAT | O_TRUNC, 0777);
                if (operators == ">>")
                    commands.v_commands[0].fds[1]=open(p, O_WRONLY | O_CREAT | O_APPEND, 0777);
                
                return commands;
}

int exe_single_command(commands commands){
    

        commands.v_commands[0].fds[1] = 1;
        for (int i = 1; i<commands.v_commands[0].parameters.size(); i++){
            if (commands.v_commands[0].parameters[i] == ">") {
                string f_name = commands.v_commands[0].parameters[i + 1];
                string operators = commands.v_commands[0].parameters[i];
                commands.v_commands[0].parameters.erase(commands.v_commands[0].parameters.begin() + i + 1);
                commands.v_commands[0].parameters.erase(commands.v_commands[0].parameters.begin() + i);
                
                commands= handle_redirection (commands , f_name, operators);
            }

            else if (commands.v_commands[0].parameters[i] == ">>") {
                string f_name = commands.v_commands[0].parameters[i + 1];
                string operators = commands.v_commands[0].parameters[i];
                commands.v_commands[0].parameters.erase(commands.v_commands[0].parameters.begin()+i + 1);
                commands.v_commands[0].parameters.erase(commands.v_commands[0].parameters.begin()+ i);
                
                 handle_redirection (commands , f_name, operators);
            }
        }

        if (strcmp(commands.v_commands[0].parameters[0].c_str(),"cd")==0)
        {
            int ret;
            const char * p= commands.v_commands[0].parameters[1].c_str(); 
            ret = chdir(p);
		        if (ret != 0) {
		        	fprintf(stderr, "error: unable to change dir\n");
			        return -1;
                }
                return 0;
        }

        
        char *argument_v[1000];
        int j;
        for (j = 0; j < commands.v_commands[0].parameters.size(); j++)
        {
            argument_v[j] = new char[commands.v_commands[0].parameters[j].size() + 1];
            strcpy(argument_v[j], commands.v_commands[0].parameters[j].c_str());
        }
        argument_v[j] = (char *)NULL;

        
        pid_t pid = fork();
        if (pid == 0)
        {
            if (commands.v_commands[0].fds[1] != 1)
            {
                dup2(commands.v_commands[0].fds[1], 1);
                close(commands.v_commands[0].fds[1]);
            }
            if (execvp(argument_v[0], (char *const *)argument_v) < 0)
            {
                perror("error");
                return -1;
            }
            exit(0);
        }
        wait(NULL);
}


int exe_commands(commands commands)
{
    

    
    if (commands.v_commands.size() == 0)
        return 0;

    
    if (commands.v_commands.size() == 1)
    {
        return exe_single_command(commands);
        
    }
    else
    { 
        int count = commands.v_commands.size();
        commands.command_count=count;
        int(*pipes)[2] = new int[commands.v_commands.size()- 1][2];

       
        commands.v_commands[0].fds[0] = 0; 
        for (int i = 0; i < commands.command_count - 1; i++)
        {

            if (pipe(pipes[i]) == -1)
            {
                perror("pipe");
            }
            commands.v_commands[i].fds[1] = pipes[i][1];
            commands.v_commands[i + 1].fds[0] = pipes[i][0];
        }
        commands.v_commands[commands.command_count - 1].fds[1] = 1; 
        

        for (int i = 0; i < commands.command_count; i++)
        {
            
            char *argument_v[1024];
            int j;
            for (j = 0; j < commands.v_commands[i].parameters.size(); j++)
            {
                argument_v[j] = new char[commands.v_commands[i].parameters[j].size() + 1];
                strcpy(argument_v[j], commands.v_commands[i].parameters[j].c_str());
            }

            argument_v[j] = (char *)NULL;

            pid_t pid = fork();

            if (pid == 0)
            { 
                
                if (commands.v_commands[i].fds[0] != 0)
                {
                    dup2(commands.v_commands[i].fds[0], 0);
                    close(commands.v_commands[i].fds[0]);
                }
                if (commands.v_commands[i].fds[1] != 1)
                {
                    dup2(commands.v_commands[i].fds[1], 1);
                    close(commands.v_commands[i].fds[1]);
                }
                close_pipes(pipes,commands.command_count-1);
                execvp(argument_v[0], (char *const *)argument_v);
                perror("error");
                for (int i = 0; i < commands.v_commands[i].parameters.size(); i++)
                    free(argument_v[i]);

                abort();
            }
        }
        close_pipes(pipes,commands.command_count-1);
        wait(NULL);
        free(pipes);
        commands.command_count=0;
    }
    return 0;
}


int main()
{

    int store = 0;
    
    while (1)
    {   
        string input;    
        printf("$ ");
        getline(cin, input);
       
        commands commands = parse_with_pipe(input);

        store = exe_commands(commands);
        if (input == "exit")
            break;
    }

    return 0;
}
