#include <iostream>
#include <vector>
#include <cstdlib>
#include <cassert>
#include <algorithm>
#include <iterator>

template <typename T, typename Compare = std::less<T>>
class SkipList {
    static const int MAX_LEVEL = 16;

    
    struct Node {
        T value;
        std::vector<Node*> next; 
        std::vector<Node*> prev; 

        Node(T val, int level) : value(val), next(level + 1, nullptr), prev(level + 1, nullptr) {}
    };

    Node* head;          
    size_t count_elements; 
    Compare comp;

   
    int random_level() {
        int lvl = 0;
        while (lvl < MAX_LEVEL && (std::rand() % 2 == 0)) {
            lvl++;
        }
        return lvl;
    }

public:
    //  9, 10. Итераторы 
    class Iterator {
        Node* ptr;
        friend class SkipList; 

    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;

        Iterator(Node* p = nullptr) : ptr(p) {}
        T& operator*() const { return ptr->value; }
        T* operator->() const { return &ptr->value; }
        
        Iterator& operator++() { ptr = ptr->next[0]; return *this; }
        Iterator& operator--() { ptr = ptr->prev[0]; return *this; }
        
        bool operator==(const Iterator& o) const { return ptr == o.ptr; }
        bool operator!=(const Iterator& o) const { return ptr != o.ptr; }
    };

    using reverse_iterator = std::reverse_iterator<Iterator>;

    //  1. Конструктор по умолчанию 
    SkipList() : count_elements(0) {
        head = new Node(T(), MAX_LEVEL);
        for (int i = 0; i <= MAX_LEVEL; ++i) {
            head->next[i] = head;
            head->prev[i] = head;
        }
    }

    // 2. Конструктор из диапазона 
    template <typename It>
    SkipList(It first, It last) : SkipList() {
        for (; first != last; ++first) insert(*first);
    }

    // 3. Копирование 
    SkipList(const SkipList& other) : SkipList() {
        for (auto it = other.begin(); it != other.end(); ++it) insert(*it);
    }

    SkipList& operator=(const SkipList& other) {
        if (this != &other) {
            clear();
            for (auto it = other.begin(); it != other.end(); ++it) insert(*it);
        }
        return *this;
    }

    // 4. Перемещение 
    SkipList(SkipList&& other) noexcept : head(other.head), count_elements(other.count_elements) {
        other.head = new Node(T(), MAX_LEVEL);
        for (int i = 0; i <= MAX_LEVEL; ++i) {
            other.head->next[i] = other.head;
            other.head->prev[i] = other.head;
        }
        other.count_elements = 0;
    }

    SkipList& operator=(SkipList&& other) noexcept {
        if (this != &other) {
            clear();
            delete head;
            head = other.head;
            count_elements = other.count_elements;
            other.head = new Node(T(), MAX_LEVEL);
            for (int i = 0; i <= MAX_LEVEL; ++i) {
                other.head->next[i] = other.head;
                other.head->prev[i] = other.head;
            }
            other.count_elements = 0;
        }
        return *this;
    }

    // Деструктор 
    ~SkipList() {
        clear();
        delete head;
    }

    // 5, 6. empty и size
    bool empty() const { return count_elements == 0; }
    size_t size() const { return count_elements; }

    //  9, 10. begin и end 
    Iterator begin() { return Iterator(head->next[0]); }
    Iterator end() { return Iterator(head); }
    Iterator begin() const { return Iterator(head->next[0]); }
    Iterator end() const { return Iterator(head); }
    
    reverse_iterator rbegin() { return reverse_iterator(end()); }
    reverse_iterator rend() { return reverse_iterator(begin()); }

    // 7. insert 
    Iterator insert(const T& val) {
        std::vector<Node*> update(MAX_LEVEL + 1);
        Node* curr = head;

        
        for (int i = MAX_LEVEL; i >= 0; --i) {
            while (curr->next[i] != head && comp(curr->next[i]->value, val)) {
                curr = curr->next[i];
            }
            update[i] = curr;
        }

        int lvl = random_level();
        Node* new_node = new Node(val, lvl);

        for (int i = 0; i <= lvl; ++i) {
            new_node->next[i] = update[i]->next[i];
            new_node->prev[i] = update[i];
            update[i]->next[i]->prev[i] = new_node;
            update[i]->next[i] = new_node;
        }
        count_elements++;
        return Iterator(new_node);
    }

    //  8. insert из диапазона 
    template <typename It>
    void insert(It first, It last) {
        for (; first != last; ++first) insert(*first);
    }

    //  11-14. Поиск 
    Iterator lower_bound(const T& val) const {
        Node* curr = head;
        for (int i = MAX_LEVEL; i >= 0; --i) {
            while (curr->next[i] != head && comp(curr->next[i]->value, val)) {
                curr = curr->next[i];
            }
        }
        return Iterator(curr->next[0]);
    }

    Iterator upper_bound(const T& val) const {
        Node* curr = head;
        for (int i = MAX_LEVEL; i >= 0; --i) {
            while (curr->next[i] != head && !comp(val, curr->next[i]->value)) {
                curr = curr->next[i];
            }
        }
        return Iterator(curr->next[0]);
    }

    Iterator find(const T& val) const {
        Iterator it = lower_bound(val);
        if (it != end() && !comp(val, *it)) return it;
        return end();
    }

    //  12. count 
    size_t count(const T& val) const {
        size_t cnt = 0;
        for (Iterator it = lower_bound(val); it != end() && !comp(val, *it); ++it) {
            cnt++;
        }
        return cnt;
    }

    // 20. equal_range 
    std::pair<Iterator, Iterator> equal_range(const T& val) const {
        return { lower_bound(val), upper_bound(val) };
    }

    // 15. clear 
    void clear() {
        Node* curr = head->next[0];
        while (curr != head) {
            Node* next = curr->next[0];
            delete curr;
            curr = next;
        }
        for (int i = 0; i <= MAX_LEVEL; ++i) {
            head->next[i] = head;
            head->prev[i] = head;
        }
        count_elements = 0;
    }

    //  16. erase по итератору 
    Iterator erase(Iterator pos) {
        if (pos == end()) return end();
        Node* target = pos.ptr; 
        Iterator next_it = pos;
        ++next_it;


        for (size_t i = 0; i < target->next.size(); ++i) {
            target->prev[i]->next[i] = target->next[i];
            target->next[i]->prev[i] = target->prev[i];
        }
        delete target;
        count_elements--;
        return next_it;
    }

    //  17. erase диапазона
    Iterator erase(Iterator first, Iterator last) {
        while (first != last) first = erase(first);
        return last;
    }
};


void print_list(const std::string& title, SkipList<int>& sl) {
    std::cout << title << ": ";
    if (sl.empty()) {
        std::cout << "[пусто]\n";
        return;
    }
    for (auto it = sl.begin(); it != sl.end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << "(размер: " << sl.size() << ")\n";
}

int main() {
    std::srand(42);
    
    std::cout << "=== ДЕМОНСТРАЦИЯ РАБОТЫ SKIP LIST ===\n\n";

    SkipList<int> sl;

    // 1. Вставка
    std::cout << "1. Добавляем элементы: 5, 3, 8, 1, 9, 2, 7, 4, 6\n";
    for (int i : {5, 3, 8, 1, 9, 2, 7, 4, 6}) sl.insert(i);
    print_list("   Текущий список", sl);
    

    // 2. Поиск (find)
    std::cout << "2. Проверка поиска (find):\n";
    auto it5 = sl.find(5);
    if (it5 != sl.end()) std::cout << "   - Элемент 5 найден: " << *it5 << "\n";
    auto it100 = sl.find(100);
    if (it100 == sl.end()) std::cout << "   - Элемент 100 не найден (вернулся end())\n\n";

    // 3. Границы (lower_bound / upper_bound)
    std::cout << "3. Проверка границ для ключа 4:\n";
    auto lb = sl.lower_bound(4);
    auto ub = sl.upper_bound(4);
    if (lb != sl.end()) std::cout << "   - lower_bound(4) (первый >= 4): " << *lb << "\n";
    if (ub != sl.end()) std::cout << "   - upper_bound(4) (первый > 4): " << *ub << "\n\n";

    // 4. Дубликаты (count)
    std::cout << "4. Тестируем дубликаты. Добавляем число 7 ещё 3 раза.\n";
    sl.insert(7); sl.insert(7); sl.insert(7);
    print_list("   Текущий список", sl);
    std::cout << "   - Метод count(7) возвращает: " << sl.count(7) << "\n\n";

    // 5. Удаление (erase)
    std::cout << "5. Удаляем первый найденный элемент 7 через erase(iterator).\n";
    sl.erase(sl.find(7));
    print_list("   Список после удаления", sl);
    std::cout << "   - Теперь count(7) возвращает: " << sl.count(7) << "\n\n";

    // 6. Обратный обход (reverse iterator)
    std::cout << "6. Обход списка в обратном порядке (rbegin -> rend):\n   ";
    for (auto rit = sl.rbegin(); rit != sl.rend(); ++rit) {
        std::cout << *rit << " ";
    }
    std::cout << "\n\n";

    // 7. Очистка
    std::cout << "7. Очищаем список (clear).\n";
    sl.clear();
    print_list("   Текущий список", sl);

    std::cout << "\nУспех\n";
    return 0;
}