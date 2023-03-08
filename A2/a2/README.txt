
OVERALL DESIGN:

    My program first executes the command "uname -a" to find either "Darwin" or "Linux" to test what operating system the program is being run on.
    Next it executes the command "nproc --all" if on linux and "sysctl -a" if on mac to get the core count. From here it will create x amount of workers.
    When I first designed this program I didn't take the Round Robin into consideration and ended up creating a system where each worker has an ID from 1-8 and it searches for the first
    available worker and gives the task to it. I did it this way as now when the worker reports its done with a task I have a waiting list of commands that still need to be run so I can give
    the worker a task as soon as its finished, if that waiting list is empty the worker runs in idle until it needs to be run. I ended up not changing my program to Round Robin as I figured
    it ran better this way because the server only gives idle workers tasks.
    So when a task comes in to the server. The server searches for the first available worker. It will then send that worker a task. The server then puts the worker on a list indicating
    that the worker is running a task. If a new task comes into the server while all workers are busy then the task gets put in a queue. When the worker is finished its task it returns a 
    statement indicating it has finished to the server, if there is a task in the waiting queue the server will immediatley give that worker the first task in the queue and puts that worker back
    on the list indicating it is working, if there is no task in the waiting list the worker is removed from the list that indicates its working and the worker goes into idle mode.




TEST CASES:

    Test Case 1
        The first test case is used to test two common commands two different ways. The first execution runs "echo hello world" by placing this command into the server-inputfifo directly.
        This will be outputed in /tmp/worker-gnodwell.1.log and to know if it ran correctly there should be one line with the command run and a second line with the output of that command 
        "hello world"
        The second execution runs "ls -l" by using the mgSubmit program. The output of this command should also be found in /tmp/worker-gnodwell.1.log under the first command in the same format.


    Test Case 2
        The second test case is used to test the task scheduling section of this program by telling the server to run timedCountdown 10 24 times. This is done so many times so that as long as
        the server has less than 24 workers it should make use of the task scheduling.
        To know if this is run correctly each log file in the /tmp/ should contain multiple countdowns. Ideally if this is run on 8 cores each log file should have 3 countdowns and if run on a 16 
        core system each log file should have 2 countdowns.

