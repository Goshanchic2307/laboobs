#include <iostream>
#include <random>
#include <algorithm>


class Brosok {
public:
    virtual unsigned roll() = 0;      
};


class Dice : public Brosok {
public:
    Dice(unsigned max, unsigned seed)
        : max(max), dstr(1, max), reng(seed) {}

    unsigned roll() override {
        return dstr(reng);
    }

private:
    unsigned max;
    std::uniform_int_distribution<unsigned> dstr;
    std::default_random_engine reng;
};


class ThreeDicePool : public Brosok {
public:
    ThreeDicePool(Brosok& a, Brosok& b, Brosok& c)
        : a(a), b(b), c(c) {}

    unsigned roll() override {
        return a.roll() + b.roll() + c.roll();
    }

private:
    Brosok& a;
    Brosok& b;
    Brosok& c;
};

class PenaltyDice : virtual public Brosok {
public:
    PenaltyDice(Brosok& x) 
    : x(x) {}
    unsigned roll() override {
        unsigned r1 = x.roll();
        unsigned r2 = x.roll();
        return std::min(r1, r2);  
    }
private:
    Brosok& x;
};

class BonusDice : virtual public Brosok {
public:
    BonusDice(Brosok& x) : x(x) {}
    unsigned roll() override {
        unsigned r1 = x.roll();
        unsigned r2 = x.roll();
        return std::max(r1, r2);   
    }
private:
    Brosok& x;
};


class DoubleDice : public PenaltyDice, public BonusDice {
public:
    DoubleDice(Dice& d) : PenaltyDice(d), BonusDice(d) { }

    unsigned roll() override {
        return PenaltyDice::roll(); 
    }

    unsigned roll_penalty() { return PenaltyDice::roll(); }
    unsigned roll_bonus()   { return BonusDice::roll();   }
};

double expected_value(Brosok& r, unsigned number_of_rolls = 1) {
    unsigned long long accum = 0;
    for (unsigned cnt = 0; cnt < number_of_rolls; ++cnt) {
        accum += r.roll();
    }
    return double(accum) / double(number_of_rolls);
}

double value_probability(unsigned value, Brosok& d, unsigned number_of_rolls = 1) {
    unsigned count = 0;
    for (unsigned i = 0; i < number_of_rolls; ++i) {
        if (d.roll() == value) {
            ++count;
        }
    }
    return double(count) / double(number_of_rolls);
}

void print_histogram(Brosok& d, unsigned min_val, unsigned max_val,
                     unsigned number_of_rolls = 100000) {
    for (unsigned v = min_val; v <= max_val; ++v) {
        double p = value_probability(v, d, number_of_rolls);
        std::cout << v << "\t";
        int bar_len = int(p * 200); 
        for (int i = 0; i < bar_len; ++i) std::cout << '#';
        std::cout << " " << p << "\n";
    }
}


int main() {
    const unsigned N = 100000;  

    Dice d6(6, 5);
    std::cout << "E(Dice(6))       = " << expected_value(d6, N)<< "\n";

    Dice d100(100, 7);
    std::cout << "E(Dice(100))     = " << expected_value(d100, N)<< "\n";

    Dice a(6, 1), b(6, 2), c(6, 3);
    ThreeDicePool tdp(a, b, c);
    std::cout << "E(ThreeDicePool) = " << expected_value(tdp, N)<< "\n";


    PenaltyDice pd100(d100);
    BonusDice   bd100(d100);

    std::cout << "E(PenaltyDice(100)) = " << expected_value(pd100, N)<< "\n";
    std::cout << "E(Dice(100))        = " << expected_value(d100,  N)<< "\n";
    std::cout << "E(BonusDice(100))   = " << expected_value(bd100, N)<< "\n";
    
  
    PenaltyDice ptdp(tdp);
    BonusDice   btdp(tdp);

    std::cout << "E(PenaltyDice(3d6)) = " << expected_value(ptdp, N)<< "\n";
    std::cout << "E(ThreeDicePool)    = " << expected_value(tdp,  N)<< "\n";
    std::cout << "E(BonusDice(3d6))   = " << expected_value(btdp, N)<< "\n";


    std::cout << "\n--- Dice(100): обычный бросок ---\n";
    print_histogram(d100, 1, 100, N);

    std::cout << "\n--- Dice(100): PenaltyDice ---\n";
    print_histogram(pd100, 1, 100, N);

    std::cout << "\n--- Dice(100): BonusDice ---\n";
    print_histogram(bd100, 1, 100, N);


    std::cout << "\n--- ThreeDicePool: обычный бросок ---\n";
    print_histogram(tdp, 3, 18, N);

    std::cout << "\n--- ThreeDicePool:PenaltyDice ---\n";
    print_histogram(ptdp, 3, 18, N);

    std::cout << "\n--- ThreeDicePool: BonusDice ---\n";
    print_histogram(btdp, 3, 18, N);

    
    Dice d100_dd(100, 123);          
    DoubleDice dd(d100_dd);

    
    std::cout << "E[DoubleDice как штраф]        = "
              << expected_value(dd, N) << "\n";

    
    std::cout << "E[DoubleDice как преимущество] = "
              << expected_value(static_cast<BonusDice&>(dd), N)
              << "\n";

    

   
    std::cout << "\n--- DoubleDice: гистограмма как штраф ---\n";
    print_histogram(dd, 1, 100, N);
    

    
    std::cout << "\n--- DoubleDice: гистограмма как преимущество ---\n";
    print_histogram(static_cast<BonusDice&>(dd), 1, 100, N);


    return 0;
}