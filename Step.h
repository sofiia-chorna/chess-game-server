#pragma once

#include <vector>
#include <string>
#include <iostream>

struct Coord
{
    x : int;
    y : int;
};

class Step
{
private:
    std::string name;
    Coord coord;
};
