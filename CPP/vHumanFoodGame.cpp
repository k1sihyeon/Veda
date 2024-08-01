#include <bits/stdc++.h>
using namespace std;

#define W 10
#define H 20

int dx[] = {0, 1, 0, -1};
int dy[] = {1, 0, -1, 0};


class GameObject {
   protected:
    int dist;
    int x;
    int y;

   public:
    GameObject(int startX, int startY, int dist) {
        this->x = startX;
        this->y = startY;
        this->dist = dist;
    }

    virtual ~GameObject() {};

    virtual void move() = 0;
    virtual char getShape() = 0;

    int getX() {
        return x;
    }

    int getY() {
        return y;
    }

    bool collide(GameObject* p) {
        if (this->x == p->getX() && this->y == p->getY()) {
            return true;
        }
        else
            return false;
    }
};

class Human : public GameObject {
   public:
    void move() override {
        char c;
        cout << "wasd >> ";
        cin >> c;

        int nx, ny;

        if (c == 'd') {
            nx = this->dist * dx[0] + getX();
            ny = this->dist * dy[0] + getY();
        }
        else if (c == 's') {
            nx = this->dist * dx[1] + getX();
            ny = this->dist * dy[1] + getY();
        }
        else if (c == 'a') {
            nx = this->dist * dx[2] + getX();
            ny = this->dist * dy[2] + getY();
        }
        else if (c == 'w') {
            nx = this->dist * dx[3] + getX();
            ny = this->dist * dy[3] + getY();
        }
        else {
            // input error
        }

        if ((nx >= 0) && (ny >= 0) && (nx < W) && (ny < H)) {
            this->x = nx;
            this->y = ny;
        }
    }

    char getShape() override {
        return 'H';
    }

    Human(int x, int y, int dist) : GameObject(x, y, dist) {
    }

    ~Human() {
    }
};

class Monster : public GameObject {
   public:
    void move() override {
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> dis(0, 3);
        int rnum = dis(gen);

        int nx = this->dist * dx[rnum] + getX();
        int ny = this->dist * dy[rnum] + getY();

        if ((nx >= 0) && (ny >= 0) && (nx < W) && (ny < H)) {
            this->x = nx;
            this->y = ny;
        }
    }

    char getShape() override {
        return 'M';
    }

    Monster(int x, int y, int dist) : GameObject(x, y, dist) {
    }

    ~Monster() {
    }
};

class Food : public GameObject {
   private:
    int count = 0;
   
   public:
    void move() override {
        int cnt = count % 5;
        if (cnt >= 0 && cnt <= 2) {
            // 제자리
        }
        else if (cnt == 3 || cnt == 4) {
            random_device rd;
            mt19937 gen(rd());
            uniform_int_distribution<> dis(0, 3);
            int rnum = dis(gen);

            int nx = this->dist * dx[rnum] + getX();
            int ny = this->dist * dy[rnum] + getY();

            if ((nx >= 0) && (ny >= 0) && (nx < W) && (ny < H)) {
                this->x = nx;
                this->y = ny;
            }
        }
        else {
            // err
        }

        count++;
    }

    char getShape() override {
        return '@';
    }

    Food(int x, int y, int dist) : GameObject(x, y, dist) {
    }

    ~Food() {
    }
};

class Game {
   private:
    Human* h;
    Monster* m;
    Food* f;

    char board[W][H];

   public:
    void draw() {
        for(int i = 0; i < W; i++) {
            for (int j = 0; j < H; j++) {
                cout << board[i][j];
            }
            cout << "\n";
        }
    }

    void setPos() {
        memset(board, '-', sizeof(board));

        board[m->getX()][m->getY()] = m->getShape();
        board[f->getX()][f->getY()] = f->getShape();
        board[h->getX()][h->getY()] = h->getShape();
    }

    bool run() {

        cout << "** Human의 Food 먹기 게임을 시작합니다. **" << "\n";
        setPos();
        draw();

        while (true) {
            m->move();
            f->move();
            h->move();

            setPos();

            draw();

            if (h->collide(f)) {
                cout << "Human is winner" << endl;
                return false;
            }

            if (h->collide(m) || m->collide(f)) {
                cout << "Game Lose !" << endl;
                return false;
            }
        }
    }

    Game(Human* h, Monster* m, Food* f) {
        this->h = h;
        this->m = m;
        this->f = f;
    }
};

int main(void) {
    Human h(0, 0, 1);
    Food f(8, 18, 1);
    Monster m(5, 9, 2);

    Game g(&h, &m, &f);
    
    if (!g.run())
        return 0;

    return 0;
}