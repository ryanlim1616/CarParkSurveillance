#pragma once
#include<stdio.h>
#include<iostream>
#include "Switches.h"

// set imshow_display to 1 to turn on all imshow, 0 to turn off.
bool imshow_display = 1;

// set resumeFromError = 1 to resume from a particular video
bool resumeFromError = 0;

//for debugging purpose - 1(true)(on)/0(false)(off)
bool debug_on = 0;


//Just need to change here to set each user's pc to be able to do what they need to do.. temporary solution
std::string user = "Ryan";
