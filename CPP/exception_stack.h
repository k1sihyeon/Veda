#pragma once

class MyStack {
   private:
    int data[100];
    int tos;

   public:
    MyStack() {
        tos = -1;
    }
    void push(int n) throw(char *);
    int pop() throw(char *);
};