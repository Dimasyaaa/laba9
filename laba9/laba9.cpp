#include <iostream>
#include <string>
#include <cmath>
//#include <windows.h>
#include <memory> // для ходов
#include <vector>

using namespace std;

enum class Color { WHITE, BLACK }; // цвет фигурок

class Position { // позиция фигурки
public:
    int x, y;
    Position(int x = 0, int y = 0) : x(x), y(y) {} // конструктор

    bool isValid() const {
        return x >= 0 && x < 8 && y >= 0 && y < 8; // метод проверки позиции 
    }
};

class Piece { // класс фигур
protected: // если не protected, то сломаемся
    Color color;
    Position pos;
    string name;

public:
    Piece(Color c, Position p, string n) : color(c), pos(p), name(n) {} // конструктор
    virtual ~Piece() = default; // виртуальный деструктор для норм удаления наследников.

    virtual bool canMove(Position target) const = 0; // проверка на атаку
    virtual bool canAttack(Position target) const = 0; // проверка на способность ходить)

    void setPosition(Position p) { // метод для установки новой позиции фигуры.
        pos = p;
    }

    friend ostream& operator<<(ostream& os, const Piece& piece) { // Сергей Анатольевич, ну мы это уже на изусть знаем)
        os << piece.name << " на " << (char)('A' + piece.pos.x) << (piece.pos.y + 1)
            << " (" << (piece.color == Color::WHITE ? "Белый" : "Черный") << ")";
        return os;
    }

    void print() const { // инфа о фигуре
        cout << *this;
    }
};

class Pawn : public Piece { // наследование пришло не заметно, это пешечка(маленькая, но удаленькая)
public:
    Pawn(Color c, Position p) : Piece(c, p, "Пешка") {}

    bool canMove(Position target) const override { // метод для хода пешки, далее для всех фигур аналогично 
        if (!target.isValid()) return false; // проверка на позицию

        int direction = (color == Color::WHITE) ? 1 : -1;  // Определяем направление движения в зависимости от цвета.
        bool isInitialPosition = (color == Color::WHITE && pos.y == 1) || // проверка начальной позиции
            (color == Color::BLACK && pos.y == 6);

        if (target.x == pos.x) { // // Проверяем, движется ли пешка на одну клетку вперед.
            if (target.y == pos.y + direction) return true; // еле идем, одна клетка 
            if (isInitialPosition && target.y == pos.y + 2 * direction) return true; // бежииим на 2 клетки
        }
        return false;
    }

    bool canAttack(Position target) const override { // метод для атаки пешки, далее аналогично 
        if (!target.isValid()) return false;

        int direction = (color == Color::WHITE) ? 1 : -1; 
        return abs(target.x - pos.x) == 1 && target.y == pos.y + direction; // Проверяем, может ли пешка атаковать (по диагонали). Атакуем, если целевая позиция по диагонали вперед.
    }
};

class Knight : public Piece { // поник, а не рыцарь
public:
    Knight(Color c, Position p) : Piece(c, p, "Конь") {}

    bool canMove(Position target) const override {
        if (!target.isValid()) return false;

        int dx = abs(target.x - pos.x);
        int dy = abs(target.y - pos.y);
        return (dx == 2 && dy == 1) || (dx == 1 && dy == 2);
    }

    bool canAttack(Position target) const override {
        return canMove(target);
    }
};

class Bishop : public Piece { // наш слоняра
public:
    Bishop(Color c, Position p) : Piece(c, p, "Слон") {}

    bool canMove(Position target) const override {
        if (!target.isValid()) return false;

        int dx = abs(target.x - pos.x);
        int dy = abs(target.y - pos.y);
        return dx == dy;
    }

    bool canAttack(Position target) const override {
        return canMove(target);
    }
};

class Rook : public Piece { // не Рок, а Ладья(надеюсь матвей оценит)
public:
    Rook(Color c, Position p) : Piece(c, p, "Ладья") {}

    bool canMove(Position target) const override {
        if (!target.isValid()) return false;
        return target.x == pos.x || target.y == pos.y;
    }

    bool canAttack(Position target) const override {
        return canMove(target);
    }
};

class Queen : public Piece { // Харли ...
public:
    Queen(Color c, Position p) : Piece(c, p, "Королева") {}

    bool canMove(Position target) const override {
        if (!target.isValid()) return false;

        int dx = abs(target.x - pos.x);
        int dy = abs(target.y - pos.y);
        return dx == dy || target.x == pos.x || target.y == pos.y;
    }

    bool canAttack(Position target) const override {
        return canMove(target);
    }
};

class King : public Piece { // Чуть меньше туза
public:
    King(Color c, Position p) : Piece(c, p, "Король") {}

    bool canMove(Position target) const override {
        if (!target.isValid()) return false;

        int dx = abs(target.x - pos.x);
        int dy = abs(target.y - pos.y);
        return dx <= 1 && dy <= 1;
    }

    bool canAttack(Position target) const override {
        return canMove(target);
    }
};

Position parseChessPosition(const string& input) { // ф-ия для разбора шахматной позиции из строки ввода.
    if (input.length() == 2) {
        char file = input[0]; // 1 символ обозначает столбец (файл).
        char rank = input[1]; // 2 символ обозначает ряд (ранг).

        int x = toupper(file) - 'A';  // Преобразуем букву столбца в индекс (0-7) путем вычитания 'A'.
        int y = rank - '1'; // Преобразуем цифру ряда в индекс (0-7) путем вычитания '1'.

        return Position(x, y);
    }
    return Position(-1, -1);
}

void printMenu() {
    cout << "Меню:" << endl;
    cout << "1. Создать фигуру" << endl;
    cout << "2. Переместить фигуру" << endl;
    cout << "3. Атаковать" << endl;
    cout << "4. Показать все фигуры" << endl;
    cout << "5. Выход" << endl;
    cout << "Выберите действие:";
}

unique_ptr<Piece> createPiece() { //это умный указатель из стандартной библиотеки C++, который автоматически управляет временем жизни объекта, на который он указывает. как раз для него нужен мемори)
    int choice;
    cout << "Выберите фигуру:" << endl;
    cout << "1. Пешка" << endl;
    cout << "2. Конь" << endl;
    cout << "3. Слон" << endl;
    cout << "4. Ладья" << endl;
    cout << "5. Королева" << endl;
    cout << "6. Король" << endl;
    cout << "Ваш выбор:";
    cin >> choice;

    string positionInput;
    cout << "Введите начальные координаты (например, E2):";
    cin >> positionInput;
    Position pos = parseChessPosition(positionInput);

    int color;
    cout << "Выберите цвет (0 - Белый, 1 - Черный):";
    cin >> color;
    Color pieceColor = (color == 0) ? Color::WHITE : Color::BLACK;

    // это функция, доступная в стандартной библиотеке C++ с версии C++14 и далее, которая используется для создания unique_ptr объектов.
    // Она упрощает процесс создания указателей на динамически выделенные объекты и помогает избежать ошибок, связанных с ручным управлением памятью.
    // иначе ломается 
    switch (choice) {
    case 1: return make_unique<Pawn>(pieceColor, pos);
    case 2: return make_unique<Knight>(pieceColor, pos);
    case 3: return make_unique<Bishop>(pieceColor, pos);
    case 4: return make_unique<Rook>(pieceColor, pos);
    case 5: return make_unique<Queen>(pieceColor, pos);
    case 6: return make_unique<King>(pieceColor, pos);
    default: return nullptr;
    }
}

void movePiece(Piece& piece) { // ходим
    string positionInput;
    cout << "Введите целевые координаты для перемещения (например, E4):";
    cin >> positionInput;
    Position target = parseChessPosition(positionInput);
    if (piece.canMove(target)) {
        piece.setPosition(target);
        cout << "Фигура перемещена." << endl;
    }
    else {
        cout << "Нельзя переместить в указанную позицию." << endl;
    }
}

void attackWithPiece(Piece& piece) { // убиваем
    string positionInput;
    cout << "Введите координаты для атаки (например, D5):";
    cin >> positionInput;
    Position target = parseChessPosition(positionInput);
    if (piece.canAttack(target)) {
        cout << "Атака возможна." << endl;
    }
    else {
        cout << "Атака невозможна." << endl;
    }
}

int main() {
    
    setlocale(LC_ALL, "Rus");
    //SetConsoleCP(CP_UTF8);
    //SetConsoleOutputCP(CP_UTF8);

    vector<unique_ptr<Piece>> pieces;
    int choice;

    do {
        printMenu();
        cin >> choice;

        switch (choice) {
        case 1: {
            auto piece = createPiece();
            if (piece) {
                pieces.push_back(move(piece));
                cout << "Фигура создана." << endl;
            }
            else {
                cout << "Ошибка при создании фигуры." << endl;
            }
            break;
        }
        case 2: {
            int idx;
            cout << "Выберите фигуру для перемещения (индекс):";
            cin >> idx;
            if (idx >= 0 && idx < pieces.size()) {
                movePiece(*pieces[idx]);
            }
            else {
                cout << "Некорректный индекс." << endl;
            }
            break;
        }
        case 3: {
            int idx;
            cout << "Выберите фигуру для атаки (индекс):";
            cin >> idx;
            if (idx >= 0 && idx < pieces.size()) {
                attackWithPiece(*pieces[idx]);
            }
            else {
                cout << "Некорректный индекс." << endl;
            }
            break;
        }
        case 4: {
            cout << "Список фигур:" << endl;
            for (size_t i = 0; i < pieces.size(); ++i) {
                cout << i << ": " << *pieces[i] << endl;
            }
            break;
        }
        case 5:
            cout << "Выход из программы." << endl;
            break;
        default:
            cout << "Некорректный выбор. Попробуйте еще раз." << endl;
            break;
        }
    } while (choice != 5);

    return 0;
}

// ДЛЯ ПРОВЕРКИ И НАГЛЯДНОСТИ, ВЫГЛЯДИТ НЕ ОЧ, поэтому не вывожу пользователю 
//  ===========================
//  ||    a b c d e f g h    ||
//  || 8 |_|_|_|_|_|_|_|_| 8 ||
//  || 7 |_|_|_|_|_|_|_|_| 7 ||
//  || 6 |_|_|_|_|_|_|_|_| 6 ||
//  || 5 |_|_|_|_|_|_|_|_| 5 ||
//  || 4 |_|_|_|_|_|_|_|_| 4 ||
//  || 3 |_|_|_|_|_|_|_|_| 3 ||
//  || 2 |_|_|_|_|_|_|_|_| 2 ||
//  || 1 |_|_|_|_|_|_|_|_| 1 ||
//  ||    a b c d e f g h    ||
//  ===========================

