My Shell (mysh: Linux Shell)

main() -
    We determine whether or not the mode is interactive or batch using isatty() and retrieve the file descriptor of the correct file we want to read commands from.
    For interactive mode, this is where we write our welcome and end of input messages.
    We pass these file descriptors into the inputLoop() function.

inputLoop() -
    Keeps reading file for interactive mode or reads file once for batch mode and calls checkForPipeline().
    Wildcard expansion occurs here and is pushed into our arraylist of words that make up our entire command.

checkForPipeline() -
    Checks if one of the arguments that was pushed into the arraylist is a "|" and forks a child process accordingly.
    Sets a pipeExists flag.

checkForRedirection() -
    Checks if one of the arguments that was pushed into the arraylist is a "<" or ">" and forks child processes accordingly.

checkFirstArg() -
    Takes in the user command and handles the operation of the command inputted by the user. If the user's command is any of the four built-in commands, the appropriate functions are called. If the user's command is a pathname starting with a '/' or a barename, it executes the appropriate command if it exists. If the command is neither a pathname nor a built-in command, the function uses the access() function to find the command if it exists in any of the given three paths and executes it if it meets the criteria. 

For built-in commands: cd, pwd, which, exit

Fork() was not used for these.

cd(): takes in the argument inputted by the user and changes directory into it. 

which(): uses the access() function to check if the user inputted argument exists in the given paths.

pwd(): gets the current working directory based off of the user's cd command

exit(): takes in the arraylist of arguments if any and prints those arguments, before exitting out of the shell. 

To successfully use the then and else commands in our shell, we make use of a global variable that is set to 0 if a command is successful and 1 if the command fails. To determine whether or not to use the appropriate then/else command, a conditional is used to check if the exitstatus of the previously used command was successful or not. 

Test Files:

myscript.sh - Exits with exit command.

test.sh - Exits with exit command and additional arguments.

testing.sh - Exits because EOF is reached.

All of them contain different sequences of commands that may contain pipelines, redirection, and wildcards and start with a built-in, bare name, pathname, or conditional. We test "then" and "else" commands following a failed and a successful command.

Start Commands:

Interactive Mode-
        ./mysh
        echo hello | ./mysh /dev/tty
        These show a welcome message ("Welcome to my shell!"), the prompt "mysh> " so the user can write a new command and press enter, and the end message ("Exitng my shell.").
        The commands we inputted to test were the same as the one we hardcoded into the .sh files.

Batch Mode-
        ./mysh myscript.sh
        cat test.sh | ./mysh
        ./mysh < test.sh
        These use a specific .sh file and output the corresponding output based on the commands within that file.

Limitations:
    When the shell encounters redirection and pipe commands, the shell has a memory leak caused potentially by the initialization of the arraylists we use to perform the left and right processes of the pipe. 
    We have a memory leak when we do wildcard expansion using glob() despite using globfree() for every glob() call.
