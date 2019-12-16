# Embedded-OS
My solutions for Embedded Operating Systems Assignments.

## Wave
Code to build a waveform file when the user press or release a key.
It prints a 1 to the file when a key is pressed and a zero when no key is pressed.

## FindTask
Find Task is a` Linux Loadable Module` that can find the process id of any given process.
It takes the Process Name as an argument and if the process is running it prints its process id.
If the process is not running it returns `not found`.

## FindTaskTimer
This assignment is the second version of FindTask that uses the timer queue.
It checks the list of process running on a periodic base until the target process is found.
The idea is to place a call back function in the timer queue and call it once the timer expires.
This process is repeated until the target process is found.

## Morse
A user space application that displays a message using Morse Code.
It runs in different architectures, when using x86_64 it prints the message to the terminal, and when running on a BeagleBone Black it blinks LED0.

## Testchar
This project shows the basics of a `character device driver` and how a user space application can interface with it.
