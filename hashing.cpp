#include "MurmurHash3.cpp"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include <functional>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <cstdint>
/**
* Определение типов:
*   - FEATURE: вектор double для хранения признаков.
*   - LABEL: целочисленный тип для метки (0 или 1).
*   - целочисленная константа HASH_DIM (размерность хешированного пространства ).
*/
using FEATURE = std::vector<double>;
using LABEL = short int;
constexpr short int HASH_DIM = 1000; 

struct Sample {
    FEATURE features;
    LABEL label;
};



/**
 * Перевести текст в фичи
 * 
 * Каждый токен к нижнему регистру -> хеш от строки -> feats[index] += 1, где index = хеш % HASH_DIM
 * Затем выполнить L2-нормализацию feats.
 * 
 * @param text 
 * @return vec<double> feats
 * 
 * @solve  
 */
std::vector<double> text_to_features(const std::string& text) {
    std::vector<double> feats(HASH_DIM, 0);

    std::string lower_text;
    // Увеличим строку для того, чтобы transform мог работать
    lower_text.resize(text.size());
    std::transform(text.begin(), text.end(), lower_text.begin(), 
        [](unsigned char c){
            return std::tolower(c);
    });

    // Захешируем
    std::istringstream iss(lower_text);
    std::string token;
    while (iss >> token){
        uint32_t hash = 0;
        MurmurHash3(token.c_str(), token.size(), 42, &hash);
        feats[hash % HASH_DIM] += 1;
    }

    // Нормализуем вектор
    double norm = 0;
    for (double value : feats){
        norm += std::pow(value, 2);
    }
    norm = std::sqrt(norm);
    if (norm != 0){
        for (double &value : feats){
            value /= norm;
        }
    }

    return feats;
}


// Можно использовать внешние источники, но обязательно укажите ссылку на них.
// Если источник = генеративные модели / не будет источников - будет больно на защите.
class LogisticRegression {
public:
    size_t dim = HASH_DIM;
    double lr = 0.2;
    double reg_lambda = 0.01;
    double class_weight_0 = 1.0;  // вес для класса 0 (ham)
    double class_weight_1 = 1.0;  // вес для класса 1 (spam)
    LogisticRegression()
    : weights(dim, 0.0),
      learning_rate(lr),
      lambda(reg_lambda),
      epochs(500) {}

    /**
     * Обучаем логрег.
     * 
     * @param vec<Sample> trainData - наши фичи : labels
     * @param vec<Sample> validData - на чем считаем метрики
     * 
     * тут набросок, можете предложить свой вариант реализации. 
     * например - вывод лосса на обучении, или каджые M эпох уменьшает lr
     * 
     * при указании источника спрашивать как работает не будем.
     * 
     * функция должна обновить weights
     * @solve
     */
    void train(const std::vector<Sample>& trainData, const std::vector<Sample>& validData) { 
        // Обучение модели с помощью градиентного спуска и L2 регуляризации
        double best_accuracy = 0;
        int no_improvement_count = 0;
        const int early_stopping_threshold = 5;
        reg_lambda = 0.03;

        for (size_t epoch = 0; epoch < epochs; epoch++){
            for (size_t i = 0; i < trainData.size(); i++){
                const Sample& X_i = trainData[i];
                double error = X_i.label - sigmoid(dot_product(X_i.features, weights));
                double class_weight = (X_i.label == 1) ? class_weight_1 * 3 : class_weight_0;
                error *= class_weight;

                for (size_t j=0; j<weights.size(); j++){
                    double gradient = X_i.features[j] * error;
                    weights[j] += lr * gradient - (reg_lambda * weights[j]);
                }
            }

            // Проведем валидацию
            std::vector<double> metrics = evaluate(validData);
            double accuracy = (metrics[0] + metrics[1]) / (metrics[0] +  metrics[1] + metrics[2] +  metrics[3]);
            if (accuracy > best_accuracy) {
                best_accuracy = accuracy;
                no_improvement_count = 0;
            } else {
                no_improvement_count++;
                if (no_improvement_count == early_stopping_threshold) {
                    break;
                }
            }
        }
    }

    /**
     * Предсказываем класс для новых фичей
     * 
     * должна вернуть vec<double> metrics = {0, 0, 0, 0}; // TP, TN, FP, FN 
     * @param data - vec<Sample> данные для валидации
     * @solve
     */
    std::vector<double> evaluate(const std::vector<Sample>& data) const {
        std::vector<double> metrics = {0,0,0,0};
        for (Sample object : data){
            int predicted_label = predict(object.features);
            if (object.label == 1 && predicted_label == 1){
                metrics[0] += 1;
            } else if (object.label == 0 && predicted_label == 0){
                metrics[1] += 1;
            } else if (object.label == 0 && predicted_label == 1){
                metrics[2] += 1;
            } else if (object.label == 1 && predicted_label == 0){
                metrics[3] += 1;
            }
        }
        return metrics;
    }

    int predict(const FEATURE& feats) const {
        // Заменил порог на 0.53. При нем достигаются лучшие метрики
        return (sigmoid(dot_product(weights, feats)) >= 0.53) ? 1 : 0;
    }

private:
    std::vector<double> weights;
    double learning_rate;
    double lambda;
    int epochs;

    static double dot_product(const FEATURE& w, const FEATURE& x) {
        double res = 0.0;
        for (size_t i = 0; i < w.size(); ++i) {
            res += w[i] * x[i];
        }
        return res;
    }
    // можете использовать другую активацию
    static double sigmoid(double z) {
        return 1.0 / (1.0 + exp(-z));
    }
};

/**
 * Загрузить данные из csv
 * csv формата class,text
 * 
 * затем загруженные данные нужно пропустить через text_to_features
 * 
 * @param filename путь до файла, указывать как "entrypoiny/FILENAME"
 * @param vec<Sample> data - куда положить загруженные данные
 * @return bool если загрузка успешна
 * 
 * @solve  
 */
bool read_csv(const std::string& filename, std::vector<Sample>& data) { 
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        size_t commaPos = line.find(',');
        if (commaPos != std::string::npos){
            Sample str;
            str.label = (line.substr(0, commaPos) == "ham") ? 0 : 1;
            str.features = text_to_features(line.substr(commaPos+1));
            data.push_back(str);
        } else {
            continue;
        }
    }

    return true;
 }

