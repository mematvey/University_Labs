#include <sstream>
#include <string>
#include <cmath>
#include <limits>

enum Color {
    RED,
    BLACK
};

struct Node {
    std::string film_name;
    double sum_ratings;
    double avg_rating;
    int count_ratings;

    Color color;
    Node *left, *right, *parent;

    Node(const std::string &name, double rating)
        : film_name(name), sum_ratings(rating), avg_rating(rating), count_ratings(1),
        color(RED), left(nullptr), right(nullptr), parent(nullptr) {
    }

    /**
     * Обновить значение рейтиинга.
     * 
     * @note При добавлении новой оценки к фильму:
     * 
        - Обновите сумму оценок и увеличьте количество оценок.

        - Пересчитайте среднюю оценку.
        Если текущая средняя оценка равна `avg`,
        количество оценок равно `n`,
        а новая оценка равна `r`,
        то новое значение средней оценки вычисляется по формуле:  
            
        `новое среднее = ((avg × n) + r) / (n + 1)`
     
     */
    void updateRating(double newRating) {
        count_ratings += 1;
        sum_ratings += newRating;
        avg_rating = sum_ratings / count_ratings;
    }
};


class RBTree {
public:
    Node *root;

    RBTree() : root(nullptr) {}

    /**
     * Левое вращение вокруг узла x.
     * 
     * @param x нода, вокруг которой будет выполняться вращение
     */
    void leftRotate(Node *x) {
        Node* child = x->right;
        // Если у child есть левый потомок, то мы должны соединить его с y
        x->right = child->left;
        if (x->right != nullptr) {
            x->right->parent = x;
        }
        child->parent = x->parent;
        if (x->parent == nullptr) {
            root = child;
        } else if (x == x->parent->left) {
            x->parent->left = child;
        } else {
            x->parent->right = child;
        }
        child->left = x;
        x->parent = child;
    }

    /**
     * Правок вращение вокруг узла x.
     * 
     * @param x нода, вокруг которой будет выполняться вращение
     */
    void rightRotate(Node *y) {
        Node* child = y->left;
        // Если у child есть правый потомок, то мы должны соединить его с y
        y->left = child->right;
        if (y->left != nullptr) {
            y->left->parent = y;
        }
        child->parent = y->parent;
        if (y->parent == nullptr) {
            root = child;
        } else if (y == y->parent->right) {
            y->parent->right = child;
        } else {
            y->parent->left = child;
        }
        child->right = y;
        y->parent = child;
    }

    /**
     * Вставить/обновить фильм и рейтинг
     * 
     * @param film_name название фильма
     * @param rating рейтинг
     */
    void insert(const std::string &film_name, double rating) {
        Node *newNode = new Node(film_name, rating);
        Node *current = root;
        Node *parent = nullptr;

        while (current != nullptr) {
            parent = current;
            if (current->film_name < film_name) {
                current = current->right;
            } else if (current->film_name > film_name) {
                current = current->left;
            } else {
                current->updateRating(rating);
                // Удалим, так как больше не используем
                delete newNode;
                return;
            }
        }
        
        newNode->parent = parent;
        if (parent == nullptr) {
            root = newNode;
        } else if (parent->film_name < newNode->film_name) {
            parent->right = newNode;
        } else {
            parent->left = newNode;
        }

        fixInsert(newNode);
    }

    /**
     * Восстановить свойства красно-черного дерева после вставки.
     * 
     * @param node вставленный узел, который нужно исправить
     */
    void fixInsert(Node *node) {
        if (node == root) {
            node->color = BLACK;
            return;
        }

        Node *parent = nullptr;
        Node *grand = nullptr;
        while (node != root && node->color == RED && node->parent->color == RED) {
            parent = node->parent;
            grand = parent->parent;
            // Если родитель - левый ребенок дедушки, то дядя - правый
            if (parent == grand->left) {
                Node *uncle = grand->right;
                // Если дядя - красный, то просто перекрашиваем узлы 
                // и делаем node = grand, чтобы продолжить балансировку
                if (uncle != nullptr && uncle->color == RED) {
                    parent->color = BLACK;
                    uncle->color = BLACK;
                    grand->color = RED;
                    node = grand;
                } else {
                    // Если node - правый потомок родителя, то ситуация LR - 
                    // подъем ребенка на место родителя и LL
                    if (node == parent->right) {
                        leftRotate(parent);
                        parent = node;
                        node = parent; 
                    }
                    // Этот шаг нужен в любом случае - LL вращение
                    rightRotate(grand);
                    std::swap(parent->color, grand->color);
                    node = parent;
                }
            } else {
                // Дядя - левый
                Node *uncle = grand->left;
                if (uncle != nullptr && uncle->color == RED) {
                    parent->color = BLACK;
                    uncle->color = BLACK;
                    grand->color = RED;
                    node = grand;
                } else {
                    // Если node - левый потомок родителя, то ситуация RL
                    if (node == parent->left) {
                        rightRotate(parent);
                        parent = node;
                        node = parent;
                    }
                    // RR вращение
                    leftRotate(grand);
                    std::swap(parent->color, grand->color);
                    node = parent;
                }
            }
        }
        root->color = BLACK;
    }

    /**
     * Найти фильм по названию
     * 
     * @param film_name название фильма
     * 
     * @return указатель на ноду, содержащий фильм. nullptr если не найдено.
     */
    Node *search(const std::string &film_name) {
        Node *current = root;
        while (current != nullptr) {
            if (current->film_name < film_name) {
                current = current->right;
            } else if (current->film_name > film_name) {
                current = current->left;
            } else {
                return current;
            }
        }
        return nullptr;
    }

    /**
     * Обход по порядку для поиска узла со средним рейтингом, наиболее близким к target_rating.
     * 
     * @param node
     * @param target_rating таргетный рейтинг
     * 
     * @param bestMatch наиболее подходящий узел, найденный на данный момент. 
     * @param bestDiff наименьшая разница между целевым и фактическим рейтингами, найденными на данный момент.
     */
    void inOrderRecommend(Node *node, double target_rating, Node *&bestMatch, double &bestDiff) { 
        // Рекурсивная реализация прямого обхода дерева для поиска хорошего фильма
        if (node == nullptr) {
            return;
        }
        double currDiff = std::abs(node->avg_rating - target_rating);
        // Функция работает с ссылками, поэтому меняются переменные из функции recommend
        if (currDiff < bestDiff) {
            bestDiff = currDiff;
            bestMatch = node;
        }
        inOrderRecommend(node->left, target_rating, bestMatch, bestDiff);
        inOrderRecommend(node->right, target_rating, bestMatch, bestDiff);
    }

    /**
     * Получить рекомендацию — фильм со средним рейтингом, наиболее близким к target_rating.
     * 
     * @param target_rating таргет тейтинг
     * 
     * @return нода на лучшее совпадение. если не найдено - nullptr
     */
    Node *recommend(double target_rating) {
        if (root == nullptr) {
            return nullptr;
        }
        double bestDiff = std::numeric_limits<double>::max();
        Node *bestMatch = nullptr;
        inOrderRecommend(root, target_rating, bestMatch, bestDiff);
        return bestMatch;
    }
};
