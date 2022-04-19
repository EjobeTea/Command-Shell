# Command-Shell

Command shell accepts commands, forks a child process, and executes based on those commands. The shell will keep running until the user specifies to "quit" or "exit" the shell. 
</br>
### Run in terminal to compile and execute:
gcc msh.c -o msh —std=c99  
./msh 
</br>
</br>
Commands      | Explanation |  Example
:-------------: | :------------:|:--------:
ls | Lists directories & files in current working directory | ls -l
cd | Changes current directory. | cd Documents
history | Lists last 15 previous commands | history
!n| Where n is number 0 - 14, it will execute the nth command given within given history | !2
pidhistory | Gives a history of last 15 pids (process identifiers) | pidhistory
quit or exit | Exits the terminal. | exit

Note: This shell does support flags with commands, such as -a or -l for example.
