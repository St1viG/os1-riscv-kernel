#include "../h/syscall_cpp.hpp"
#include "Modification_test.hpp"
#include "printing.hpp"


static unsigned long int next = 1;

int custom_rand(void) {
    next = next * 1103515245 + 12345;
    return (unsigned int) (next / 65536) % 34768;
}

void custom_srand(unsigned int seed) {
    next = seed;
}

static int Hist[10] = {0,0,0,0,0,0,0,0,0,0};
static Semaphore* histMutex = nullptr;  // stiti spajanje u deljeni histogram
static Semaphore* finished = nullptr;   // svaka nit signalizira kraj obrade

class HistThread: public Thread{
    int n;
    int* row;
    void HistThreadBody();
public:
    HistThread(int n, int* row) : Thread(), n(n), row(row){}

    void run() override {
        HistThreadBody();
    }
};


void HistThread::HistThreadBody(){
    int localHist[10] = {0,0,0,0,0,0,0,0,0,0};
    for(int j = 0; j < n; j++){
        localHist[row[j] % 10]++;
        if((j + 1) % 10 == 0)
            sleep(5);
    }

    histMutex->wait();
    for(int i = 0; i < 10; i++)
        Hist[i] += localHist[i];
    histMutex->signal();

    finished->signal();
}

static int readInt(char const* prompt){
    printString(prompt);
    int x = 0;
    for(char c = getc(); c != '\n'; c = getc()){
        if(c >= '0' && c <= '9'){
            x *= 10;
            x += c - '0';
        }
    }
    return x;
}

void Histogram_test(){
    int m = readInt("Unesite M (broj vrsta): ");
    int n = readInt("Unesite N (broj kolona): ");

    // (unsigned) jer bi za signed brojac g++ ubacio proveru koja zove
    // __cxa_throw_bad_array_new_length, a toga nema u -nostdlib okruzenju
    int** mat = new int*[(unsigned) m];
    for(int i = 0; i < m; i++){
        mat[i] = new int[(unsigned) n];
        for(int j = 0; j < n; j++)
            mat[i][j] = custom_rand();
    }

    for(int i = 0; i < m; i++){
        for(int j = 0; j < n; j++){
            printInt(mat[i][j]);
            printString(" ");
        }
        printString("\n");
    }

    histMutex = new Semaphore(1);
    finished = new Semaphore(0);

    Thread** t = new Thread*[(unsigned) m];
    for(int i = 0; i < m; i++)
        t[i] = new HistThread(n, mat[i]);

    for(int i = 0; i < m; i++)
        t[i]->start();

    for(int i = 0; i < m; i++)
        finished->wait();

    int sum = 0;
    for(int i = 0; i < 10; i++){
        sum += Hist[i];
        printInt(Hist[i]);
        printString(" ");
    }
    printString("\n Suma histograma:");
    printInt(sum);
    printString("\n");

    for(int i = 0; i < m; i++){
        delete t[i];
        delete[] mat[i];
    }
    delete[] t;
    delete[] mat;
    delete histMutex;
    delete finished;
}
