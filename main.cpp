#include <iostream>
#include <map>
#include <chrono>
#include <vector>
#include <cstring>

struct MeasureScope {
    MeasureScope(const std::string& name) :
            name(name),
            start(std::chrono::high_resolution_clock::now())
    {
    }

    ~MeasureScope()
    {
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << name << " took \t" << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "ns\n";
    }

    std::string name;
    std::chrono::time_point<std::chrono::high_resolution_clock> start;
};

#define NUM_ELEMENTS (10*1000*1000)
typedef int64_t T;
static T data[NUM_ELEMENTS];

void generateRandomData(T *data, size_t num_elements)
{
    for (int i = 0; i < num_elements; i ++){
        data[i] = rand();
        //data[i] = NUM_ELEMENTS - i;
    }
}

template <typename T>void test_map(const T *data, size_t num_elements, size_t threshold, uint64_t *results)
{
    std::multimap<T, size_t> mapItems;
    for ( size_t i = 0; i < num_elements; i ++)
    {
        mapItems.insert({data[i], i});

        while (mapItems.size() > threshold)
            mapItems.erase(--mapItems.end());
    }

    size_t i = 0;
    for (auto items: mapItems){
        results[i++] = items.second;
    }
}

template <typename T> struct SortableItem
{
    uint64_t a;
    T b;
    bool operator < (const SortableItem& other) const    {return (this->a < other.a);}
};

void test_sort(const T *data, size_t num_elements, size_t threshold, uint64_t *results)
{
    threshold = std::min(threshold, num_elements);
    SortableItem <T>*dataIndexed = new SortableItem<T>[num_elements];
    for (size_t i = 0; i < num_elements; i ++)
    {
        dataIndexed[i].a = data[i];
        dataIndexed[i].b = i;
    }

    std::sort(dataIndexed, dataIndexed + num_elements);


    for (size_t i = 0; i < threshold; i ++){
        results[i] = dataIndexed[i].b;
    }

    delete []dataIndexed;
}

void test_custom1(const T *data, size_t num_elements, size_t threshold, uint64_t *results)
{
    threshold = std::min(num_elements, threshold);
    int current_max = 0;
    int cur;
    int z;
    for ( size_t i = 0; i < num_elements; i ++) {
        //We starting from the highest values and we look for the immediately lower than the given one
        for (cur = current_max; cur > 0 && (data[i] < data[results[cur - 1]]); cur--);

        if (cur < threshold) {
            //Move all the higher values 1 position to the right
            for (z = current_max -1; z >= cur; z--)
                results[z + 1] = results[z];
            current_max = std::min((int)threshold, current_max+1);

            //insert element into the given position
            results[cur] = i;
        }
    }
}

template <typename T> void GetFirstElements(T *data, size_t num_elements, size_t threshold, uint64_t *results)
{
    threshold = std::min(threshold, num_elements);

    SortableItem<T> *dataIndexed = new SortableItem<T>[num_elements];
    for (size_t i = 0; i < num_elements; i++) {
        dataIndexed[i].a = data[i];
        dataIndexed[i].b = i;
    }

    std::nth_element(dataIndexed, dataIndexed + threshold, dataIndexed + num_elements);
    std::sort(dataIndexed, dataIndexed + threshold);

    for (size_t i = 0; i < threshold; i++) {
        results[i] = dataIndexed[i].b;
    }

    delete []dataIndexed;
}


void test_nth(T *data, size_t num_elements, size_t threshold, uint64_t *results) {
    GetFirstElements(data, num_elements, threshold, results);
}

int main(int argc, char *argv[]) {
    std::cout << "Parameters " << argc << std::endl;
    std::cout << "Generating random data...";
    generateRandomData(data, NUM_ELEMENTS);
    std::cout << "done" << std::endl;

    size_t threshold = 10;
    uint64_t results[threshold];
    std::cout << "Getting first " << threshold << " elements ..." << std::endl;

    for (size_t i = 0; i < threshold; i ++)    {results[i] = 0;}
    {
        MeasureScope measure("test_map");
        test_map(data, NUM_ELEMENTS, threshold, results);
        std::cout << "done" << std::endl;
        for (size_t i = 0; i < std::min(threshold,(size_t)5); i ++)    {        std::cout << data[results[i]] << " ";    }
    }

    std::memset(results, 0, sizeof(results));
    for (size_t i = 0; i < threshold; i ++)    {results[i] = 0;}
    {
        MeasureScope measure("test_sort");
        test_sort(data, NUM_ELEMENTS, threshold, results);
        std::cout << "done" << std::endl;
        for (size_t i = 0; i < std::min(threshold,(size_t)5); i ++)    {        std::cout << data[results[i]] << " ";    }
    }

    std::memset(results, 0xFF, sizeof(results));
    for (size_t i = 0; i < threshold; i ++)    {results[i] = 0;}
    {
        MeasureScope measure("test_custom1");
        test_custom1(data, NUM_ELEMENTS, threshold, results);
        std::cout << "done" << std::endl;
        for (size_t i = 0; i < std::min(threshold,(size_t)5); i ++)    {        std::cout << data[results[i]] << " ";    }
    }

    std::memset(results, 0, sizeof(results));
    for (size_t i = 0; i < threshold; i ++)    {results[i] = 0;}
    {
        MeasureScope measure("test_nth");
        test_nth(data, NUM_ELEMENTS, threshold, results);
        std::cout << "done" << std::endl;
        for (size_t i = 0; i < std::min(threshold,(size_t)5); i ++)    {        std::cout << data[results[i]] << " ";  }
    }

    std::cout << std::endl;
    return 0;
}