# Big-Integer
В данном репозитории находится реализация класса для представления длинного знакового числа.

## Реализация
Класс называется `big_integer`, а код находится `big_integer.h` и `big_integer.cpp`.

Реализованы:
- Конструктор по умолчанию, инициализирующий число нулём.
- Конструктор копирования.
- Конструкторы от числовых типов.
- Explicit конструктор от `std::string`.
- Оператор присваивания.
- Операторы сравнения.
- Арифметические операции: сложение, вычитание, умножение, деление, унарный минус и плюс.
- Инкреметы и декременты.
- Битовые операции: и, или, исключающее или, не (аналогично битовым операциям для `int`)
- Битовые сдвиги.
- Внешняя функция `std::string to_string(big_integer const&)`.

Реализация удовлетворяет следующим требованиям:
- Умножение и деление работают не хуже, чем за O(nm).
- Остальные операции производятся с максимально возможной асимптотической эффективностью.
- Помимо асимптотики, уделено внимание оптимизации количества аллокаций и общего времени работы.
- Разряды числа представляются как минимум 32-битными числами, все биты в их представлении используются.
- В качество челленджа во время написания не использовались сторонние библиотеки.
- `big_integer` умеет создаваться от числовых типов, сохраняя значение. Если переменная числового типа имела значение `x`, значит `big_integer` после создания представляет собой число `x`.
- Класс был прокрыт тестами для проверки корректности его работы и того, что исполняемые им операции входят в приемлимые рамки по времени. (Время прохождения написанных тестов не превышает 1 секунды.)

Написание данного класса вдохновлено книгой ["Modern Computer Arithmetic"](https://members.loria.fr/PZimmermann/mca/mca-0.5.pdf) и статьёй ["Multiple-Length Division Revisited: A Tour of the Minefield"](https://surface.syr.edu/cgi/viewcontent.cgi?article=1162&context=eecs_techreports) 

Так, в реализации деления был использован алгоритм Svoboda's Division, позволяющий достаточно быстро находить остаток от деления, предварительно сведя числа к их нормализованному виду. Это позволяет впоследствии также быстро найти частное, если его вычисление требуется требуется.

В репозитории **исключительно для тестирования** используется реализация длинных чисел с использованием библиотеки `GNU Multi-Precision`.

## Сборка и тестирование

Для сборки кода и запуска тестов можно воспользоваться IDE (например, CLion имеет интеграцию с googletests).
