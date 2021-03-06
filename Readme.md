# Car2Car Simulator, using Lamport’s Mutual Exclusion Algorithm for Distributed Systems
By Shaomin (Samuel) Zhang, Mar 2015

## Brief
This is a programming project in my Distributed Systems course. The purpose of this project is to better understand the algorithms for Distributed Mutual Exclusion. Lamport’s algorithm for mutual exclusion is chosen for my program. This program consists of two parts. The “Car” runs at backend and its modules include several socket message channels, Lamport’s logical clock, Lamport’s Mutual Exclusion algorithm and its modified version for the second part of this question, and car’s basic functionality movement. The other part is a GUI application based on a 3rdparty GUI system called FTK, which is an open source cross-platform GUI tool that I was once familiar. These two parts of programs are actually independent but highly integrated together and can run on Ubuntu Linux distribution. 

## Problem
![](https://raw.githubusercontent.com/smicn/Car2Car/master/docs/question.png)

## Design
### Lamport’s Algorithm for Mutual Exclusion
<br>
Ghosh, Sukumar (2014-07-14). Distributed Systems: An Algorithmic Approach, Second Edition (Chapman & Hall/CRC Computer and Information Science Series) (Page 130-131). Chapman and Hall/CRC. Kindle Edition.<br>
7.2.1 Lamport’s Solution<br>
	The first published solution to this problem is due to Lamport. It works on a completely connected network and assumes that interprocess communication channels are FIFO.<br>
	Each process maintains its own private request-queue Q. The algorithm is described by the following five rules:<br>
*	LA1: To request entry into its CS, a process sends a time-stamped request to every other process in the system and also enters the request in its local Q.<br>
*	LA2: When a process receives a request, it places it in its Q. If the process is not in its CS, then it sends a time-stamped ack to the sender. Otherwise, it defers the sending of the ack until its exit from the CS.<br>
*	LA3: A process enters its CS, when (1) its request is ordered ahead of all other requests (i.e., the time stamp of its own request is less than the time stamps of all other requests) in its local Q and (2) it has received the acks from every other process in response to its current request.<br>
*	LA4: To exit from the CS, a process (1) deletes the request from its local queue and (2) sends a time-stamped release message to all the other processes.<br>
*	LA5: When a process receives a release message, it removes the corresponding request from its local queue.<br>
<br>

### Design: Essential Factors for the Car

1. A car owns these properties: <br>
	a. Lamport Logical Clock, which is to generate and maintain stable timestamps. <br>
	b. State Machine with 3 states:  Out-of-CS, Waiting-for-Entering-CS and In-CS. <br>
	c. Network connections with every other car, which is able to both receive and send messages based on UDP sockets. The message at least contains 3 parameters: sender’s ID, message (Request, Ack or Release), timestamp. <br>
	d. At least 3 queue-like buffers to maintain: requests from others, acks feedback from others for my own request and the requests that I received in CS and should be acked after I leave CS. <br>
2. A car has other properties not related with critical section issue: <br>
	a. Basic items from the requirements, like color, number, ID, (x, y)-coordinates, a timer-driven movement within a given map. <br>
	b. Heartbeat report to a GUI observer, an independent GUI process who is only responsible for showing but not work as an coordinator or any kind of message mediator, so that the whole scenario can be seen by us. <br>











### Supplementary Design for Part (B)

Slightly we need to change the answer to such a question: “should the car in CS give an ACK to other requests?” For the previous version, the answer of course is absolutely No (Fig-A).  For part-B of this question, now we just change it to “If the car who sends request is moving in the same direction with me, I, the car in CS, will give it an ACK”, anything else keeps the same (Fig-B).

![](https://raw.githubusercontent.com/smicn/Car2Car/master/docs/partA.png)  
Fig-A At most one car is on the bridge

![](https://raw.githubusercontent.com/smicn/Car2Car/master/docs/partB.png) 
<br>Fig-B Multiple cars can be on the bridge as long as they move in the same direction

## How to compile and run this program
* Download this project to your Linux machine (You'd better use Ubuntu), 'cd' into this directory and sh ./build.sh If everything is normal, you should see a GUI window appears. 
* What to do next? Press button 'Start(A)' and see the cars run in circles and try to cross the bridge occasionally and press button 'Stop'. Then try another button 'Start(B)' to see part (b) for another situation.

## Snapshot
![](https://raw.githubusercontent.com/smicn/Car2Car/master/docs/snapshot.jpg)
<brbr>
and yes, this is an animation and the cars can move:
![](https://github.com/smicn/Car2Car/blob/master/docs/ezgif.com-optimize.gif)
<br>

## More explanations about the source code
The source code for the ‘Car’ is under the sub-directory ‘car’ and source code for user interface is under the sub-directory ‘gui’. These two parts of programs are totally independent. The GUI application receives heartbeat report messages from cars and displays the information on screen.<br><br>
Some extra code is for debugging and testing. They are also submitted because they are meaningful to show the independency of the car program and its GUI application. Another application called ‘simulator’ is actually using 4 threads and one mutex to perfectly represent the same scenario of part (a), one car is allowed to cross the bridge, which can be used to compare with the distributed version. Another ‘test’ application is to run the cars without GUI but only command line prints, which was used for debugging and now is a key proof for the independency of car program. <br><br>
FTK is a 3rdparty GUI system. For it is cross-platform and on Linux it is based on X-window and you can see my ‘-lX11’ in the Makefile under gui subdirectory. Even I was familiar with this GUI system, it still cost me much time to port it on my work station and develop my own car GUI application based on it. Oh, if you see some displaying flaws on GUI widgets, they are the problems embedded in the GUI itself. <br><br>
Why displaying ‘Welcome to Lamar Car2Car’ on the title of GUI application? It is for fun and a sincere greeting to the real Car2Car research program from the European Union: www.car-2-car.org <br><br>
This program can only be executed on Ubuntu rather than other Linux machines, unfortunately. Mostly it is because the dependencies of the GUI system are slightly but terribly different on Linux distributions. It now has to run on Ubuntu is because my work station happens to be Ubuntu 12.04 64-bit LTS. The dependencies have to be checked and the Makefile should be slightly modified if you try to run on CentOS and other machines.<br>

