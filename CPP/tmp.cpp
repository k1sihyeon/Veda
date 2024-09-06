
//선언부
class Exp {
    private:
        int base;
        int exp;
    public:
        Exp(int base = 1, int exp = 1);
        int getValue();
        int getBase();
        int getExp();
        bool equals(Exp b);
};

//구현부
Exp::Exp(int base = 1, int exp = 1) {
    this->base = base;
    this->exp = exp;
}

int Exp::getValue() {
    int val = base;
    for (int i = 0; i < exp; i++)
        base *= base;
    
    return val;
}

int Exp::getBase() {
    return base;
}

int Exp::getExp() {
    return exp;
}

bool Exp::equals(Exp b) {
    return this->getValue() == b.getValue();
}
