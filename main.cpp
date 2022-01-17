#include <iostream>
#include <map>
#include <chrono>

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
#define NUM_ELEMENTS (1000*1000*100)
typedef int64_t T;
static T data[NUM_ELEMENTS];

void generateRandomData(T *data, size_t num_elements)
{
    for (int i = 0; i < NUM_ELEMENTS; i ++){
        //data[i] = NUM_ELEMENTS - (i+1);
        data[i] = rand();
    }
}

void test_map(const T *data, size_t num_elements, size_t threshold, T *results)
{
    std::multimap<T, size_t> mapItems;
    for ( size_t i = 0; i < NUM_ELEMENTS; i ++)
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

int cmpfunc64 (const void * a, const void * b) {return *static_cast<const T*>(a) - *static_cast<const T*>(b);}

void test_qsort(const T *data, size_t num_elements, size_t threshold, T *results)
{
    threshold = std::min(threshold, num_elements);
    T *dataIndexed = (T*)malloc(sizeof(T) * num_elements * 2);
    for (size_t i = 0; i < num_elements; i ++)
    {
        dataIndexed[i*2] = data[i];
        dataIndexed[i*2+1] = i;
    }

    qsort(dataIndexed, num_elements, sizeof(T)*2, cmpfunc64);


    for (size_t i = 0; i < threshold; i ++){
        results[i] = dataIndexed[i*2+1];
    }

    ::free(dataIndexed);
}

void test_custom1(const T *data, size_t num_elements, size_t threshold, T *results)
{
    threshold = std::min(num_elements, threshold);
    int current_max = 0;
    int cur;
    for ( size_t i = 0; i < NUM_ELEMENTS; i ++) {
        //We starting from the highest values and we look for the immediately lower than the given one
        for (cur = current_max; cur > 0 && (data[i] < data[results[cur - 1]]); cur--);

        if (cur < threshold) {
            //Move all the higher values 1 position to the right
            for (int z = current_max -1; z >= cur; z--)
                results[z + 1] = results[z];
            current_max = std::min((int)threshold, current_max+1);

            //insert element into the given position
            results[cur] = i;
        }
    }
}

int main(int argc, char *argv[]) {
    std::cout << "Parameters " << argc << std::endl;
    std::cout << "Generating random data...";
    generateRandomData(data, NUM_ELEMENTS);
    std::cout << "done" << std::endl;

    size_t threshold = 20;
    T results[threshold];
    std::cout << "Getting first " << threshold << " elements ..." << std::endl;

    for (size_t i = 0; i < threshold; i ++)    {results[i] = 0;}
    {
        MeasureScope measure("test_map");
        test_map(data, NUM_ELEMENTS, threshold, results);
        std::cout << "done" << std::endl;
        for (size_t i = 0; i < threshold; i ++)    {        std::cout << results[i] << "(" << data[results[i]] << ") ";    }
    }

    for (size_t i = 0; i < threshold; i ++)    {results[i] = 0;}
    {
        MeasureScope measure("test_qsort");
        test_qsort(data, NUM_ELEMENTS, threshold, results);
        std::cout << "done" << std::endl;
        for (size_t i = 0; i < threshold; i ++)    {        std::cout << results[i] << "(" << data[results[i]] << ") ";    }
    }

    for (size_t i = 0; i < threshold; i ++)    {results[i] = 0;}
    {
        MeasureScope measure("test_custom1");
        test_custom1(data, NUM_ELEMENTS, threshold, results);
        std::cout << "done" << std::endl;
        for (size_t i = 0; i < threshold; i ++)    {        std::cout << results[i] << "(" << data[results[i]] << ") ";    }
    }



    std::cout << std::endl;
    return 0;
}