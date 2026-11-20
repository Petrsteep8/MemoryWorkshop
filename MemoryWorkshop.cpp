#include <array>
#include <cstddef>
#include <iostream>
#include <string>

struct Question {
    std::string stage;
    std::string condition;
    std::string code;
    std::array<std::string, 3> options;
    int correct;
    std::string explanation;
};

const std::array<Question, 10> QUESTIONS = { {
    {
        "Опасная копия",
        "Выполняется RawArray b = a. Что делает неявный конструктор копирования с полем data?",
        R"(struct RawArray {
    int* data = new int[3]{10, 20, 30};
    ~RawArray() { delete[] data; }
};
RawArray a;
RawArray b = a;
b.data[0] = 99;)",
        {
            "Создаёт новый буфер и копирует в него элементы.",
            "Копирует значение указателя: оба объекта ссылаются на один буфер.",
            "Не компилируется, потому что объявлен деструктор."
        },
        2,
        "Неявный конструктор копирования выполняет поэлементное копирование полей, поэтому копируется только значение указателя; буфер не дублируется, и оба объекта получают один адрес."
    },
    {
        "Опасная копия",
        "Сразу после записи b.data[0] = 99, до разрушения объектов, какое значение хранится в a.data[0]?",
        R"(struct RawArray {
    int* data = new int[3]{10, 20, 30};
    ~RawArray() { delete[] data; }
};
RawArray a;
RawArray b = a;
b.data[0] = 99;)",
        {
            "99, потому что оба указателя ведут к одному элементу.",
            "10, потому что b всегда имеет независимый буфер.",
            "Такая запись не компилируется."
        },
        1,
        "После поверхностного копирования a.data и b.data указывают на один буфер, поэтому запись через b.data[0] меняет тот же элемент, который виден через a.data[0]."
    },
    {
        "Опасная копия",
        "Что неправильно при уничтожении a и b, если их указатели всё ещё содержат один адрес?",
        R"(struct RawArray {
    int* data = new int[3]{10, 20, 30};
    ~RawArray() { delete[] data; }
};
RawArray a;
RawArray b = a;
b.data[0] = 99;)",
        {
            "Ничего: delete[] автоматически считает владельцев.",
            "Второй delete[] обязан бросить std::bad_alloc.",
            "Один буфер освобождается повторно, возникает undefined behavior."
        },
        3,
        "Оба деструктора вызовут delete[] для одного и того же адреса; повторное освобождение — undefined behavior, а delete[] не отслеживает число владельцев."
    },
    {
        "Правильное владение",
        "В этом операторе копируются int. Какое утверждение верно?",
        R"(IntArray& operator=(const IntArray& other) {
    int* fresh = other.size ? new int[other.size] : nullptr;
    for (std::size_t i = 0; i < other.size; ++i)
        fresh[i] = other.data[i];
    delete[] data;
    data = fresh;
    size = other.size;
    return *this;
})",
        {
            "До new надо выполнить delete[] data, иначе старый буфер утечёт.",
            "Если new бросит, объект сохранится; a = a здесь также безопасно.",
            "Любое a = a вызывает double free, если нет отдельного if."
        },
        2,
        "Новый буфер сначала готовится, поэтому при исключении new старый объект не меняется; при самоприсваивании fresh получает копию до delete[] data, после чего старый буфер освобождается один раз."
    },
    {
        "Правильное владение",
        "Какой подход к специальным членам класса корректен?",
        "",
        {
            "Согласовать деструктор, copy и при необходимости move; при владении через RAII-поля предпочесть правило нуля.",
            "Деструктор автоматически запрещает copy и реализует move.",
            "Правило нуля запрещает классу владеть std::vector."
        },
        1,
        "Правило трёх/пяти требует согласованно определять деструктор, копирование и перемещение; если владение инкапсулировано в RAII-полях, лучше следовать правилу нуля."
    },
    {
        "Передача ресурсов",
        "Что делает само выражение std::move(a)?",
        "",
        {
            "Немедленно обнуляет все поля a.",
            "Безусловно вызывает перемещающий конструктор.",
            "Даёт xvalue; изменение ресурса выполняет выбранная затем операция."
        },
        3,
        "std::move — это приведение к xvalue; он сам не изменяет объект и не гарантирует вызов move-конструктора. Ресурс переносит та операция, для которой затем выбрана перегрузка."
    },
    {
        "Передача ресурсов",
        "Предположим, data - единолично владеющий int*, size - размер. Какой план подходит для operator=(IntArray&& other) с контрактом «self-move сохраняет значение»?",
        "",
        {
            "Если this == &other, вернуть *this. Иначе освободить свой буфер, принять поля other, обнулить other и вернуть *this.",
            "Скопировать поля other, оставив источник без изменений, чтобы оба объекта владели буфером.",
            "Вызвать delete[] other.data, затем скопировать этот адрес в data."
        },
        1,
        "Нужно отдельно обработать this == &other, иначе освободить свой буфер, забрать поля other и привести источник к пустому состоянию; это сохраняет значение при self-move."
    },
    {
        "Передача ресурсов",
        "Контракт учебного IntArray: после перемещения в другой объект источник имеет size == 0 и data == nullptr. Какие действия допустимы?",
        "",
        {
            "Объект уже уничтожен; использовать его нельзя никак.",
            "Прочитать size, проверить data, присвоить новое значение или уничтожить объект; a[0] читать нельзя.",
            "Прочитать a[0]: прежний элемент обязательно сохранился в источнике."
        },
        2,
        "После перемещения объект остаётся живым, но пустым по контракту IntArray: можно читать size, проверять data, присваивать заново или уничтожать; обращаться к a[0] нельзя, потому что буфера нет."
    },
    {
        "Операторы под контролем",
        "a и b независимо владеют буферами {10, 20, 30}. Их адреса различны. operator== сравнивает размер и элементы. Чему равно a == b?",
        "",
        {
            "false, потому что адреса различаются.",
            "Выражение нельзя вычислить для двух объектов.",
            "true, потому что размеры и значения элементов совпадают."
        },
        3,
        "operator== в этом примере сравнивает логическое содержимое: размер и элементы. Разные адреса буферов не важны, поэтому a == b истинно."
    },
    {
        "Операторы под контролем",
        "Есть две перегрузки ниже. a - обычный массив, c - const-массив, оба размера 3. Какое утверждение верно?",
        R"(int& operator[](std::size_t i) { return data[i]; }
const int& operator[](std::size_t i) const { return data[i]; })",
        {
            "c[0] = 5 допустимо, потому что метод const.",
            "a[0] = 5 и int x = c[0] допустимы; c[0] = 5 не компилируется.",
            "a[10] = 5 обязательно бросит исключение."
        },
        2,
        "Для неконстантного a выбирается int&, поэтому a[0] = 5 допустимо. Для const c выбирается const int&, поэтому c[0] можно читать, но присваивание c[0] = 5 не компилируется."
    }
} };

std::string trimSpacesTabs(const std::string& s) {
    const std::string ws = " \t";
    std::size_t begin = s.find_first_not_of(ws);
    if (begin == std::string::npos) return "";
    std::size_t end = s.find_last_not_of(ws);
    return s.substr(begin, end - begin + 1);
}

bool readChoice(int min, int max, int& choice, const std::string& prompt) {
    std::string line;
    while (true) {
        std::cout << prompt;
        if (!std::getline(std::cin, line)) return false;

        std::string t = trimSpacesTabs(line);
        if (t.size() == 1 && t[0] >= '0' && t[0] <= '9') {
            int value = t[0] - '0';
            if (value >= min && value <= max) {
                choice = value;
                return true;
            }
        }

        std::cout << "Некорректный ввод. Введите одно число: "
            << min << ".." << max << ".\n";
    }
}

bool waitEnter() {
    std::string line;
    while (true) {
        std::cout << "Нажмите Enter, чтобы продолжить...";
        if (!std::getline(std::cin, line)) return false;
        if (trimSpacesTabs(line).empty()) return true;
        std::cout << "Для продолжения нужна пустая строка.\n";
    }
}

void showRules() {
    std::cout << "\nПравила и баллы\n";
    std::cout << "В игре 10 вопросов. В каждом выберите один вариант: 1, 2 или 3.\n";
    std::cout << "Правильный ответ: +10 баллов, неправильный: 0 баллов.\n";
    std::cout << "Максимум: 100 баллов. Новый запуск игры сбрасывает счёт.\n";
    std::cout << "Оценки:\n";
    std::cout << "90-100: Отличное понимание темы\n";
    std::cout << "70-89:  Уверенное понимание материала\n";
    std::cout << "50-69:  Базовые знания, рекомендуется повторение\n";
    std::cout << "0-49:   Стоит повторить материал\n";
}

void showMenu() {
    std::cout << "\nМастерская памяти\n";
    std::cout << "1. Начать новую игру\n";
    std::cout << "2. Показать правила и систему баллов\n";
    std::cout << "0. Выход\n";
}

void showQuestion(const Question& q, int index) {
    std::cout << "\nЭтап: " << q.stage << "\n";
    std::cout << "Вопрос " << (index + 1) << "/10\n";
    std::cout << q.condition << "\n";
    if (!q.code.empty()) {
        std::cout << "Фрагмент кода:\n" << q.code << "\n";
    }
    for (int i = 0; i < 3; ++i) {
        std::cout << (i + 1) << ") " << q.options[i] << "\n";
    }
}

bool askQuestion(const Question& q, int index, int& correctCount) {
    showQuestion(q, index);

    int answer = 0;
    if (!readChoice(1, 3, answer, "Ваш ответ: ")) return false;

    if (answer == q.correct) {
        std::cout << "Верно\n";
        ++correctCount;
    }
    else {
        std::cout << "Неверно\n";
    }

    std::cout << "Правильный ответ: " << q.correct << ") "
        << q.options[q.correct - 1] << "\n";
    std::cout << "Пояснение: " << q.explanation << "\n";

    return waitEnter();
}

void showResult(int correctCount) {
    int score = correctCount * 10;
    std::string grade;

    if (score >= 90) {
        grade = "Отличное понимание темы";
    }
    else if (score >= 70) {
        grade = "Уверенное понимание материала";
    }
    else if (score >= 50) {
        grade = "Базовые знания, рекомендуется повторение";
    }
    else {
        grade = "Стоит повторить материал";
    }

    std::cout << "\n Результат\n";
    std::cout << "Правильно: " << correctCount << " из 10\n";
    std::cout << "Баллы: " << score << " из 100\n";
    std::cout << "Оценка: " << grade << "\n";
}

bool playGame() {
    int correctCount = 0;

    for (int i = 0; i < static_cast<int>(QUESTIONS.size()); ++i) {
        if (!askQuestion(QUESTIONS[i], i, correctCount)) {
            return false;
        }
    }

    showResult(correctCount);
    return true;
}

int main() {
    setlocale(LC_ALL, "");
    while (true) {
        showMenu();

        int choice = 0;
        if (!readChoice(0, 2, choice, "Выберите пункт: ")) {
            std::cout << "\nВвод завершён.\n";
            return 0;
        }

        if (choice == 0) {
            std::cout << "До свидания!\n";
            return 0;
        }

        if (choice == 2) {
            showRules();
            continue;
        }

        if (choice == 1) {
            if (!playGame()) {
                std::cout << "\nИгра прервана: достигнут конец ввода. "
                    "Итоговая оценка не выставляется.\n";
                return 0;
            }
        }
    }
}