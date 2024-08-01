#pragma once

class Adder {
   private:
    int op1, op2;

   public:
    Adder(int a, int b);
    int process();
};