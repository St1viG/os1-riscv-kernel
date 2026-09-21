# Histogram matrice

Potrebno je dinamički alocirati matricu `mat` celih brojeva (`int`), dimenzija `M*N`, gde se
`M` i `N` učitavaju sa standardnog ulaza i imaju proizvoljan broj cifara. Matricu nakon
alociranja popuniti pseudoslučajnim vrednostima. Za generisanje pseudoslučajnih vrednosti
koristiti sledeći segment koda:

```c
static unsigned long int next = 1;

int custom_rand(void) {
    next = next * 1103515245 + 12345;
    return (unsigned int) (next / 65536) % 34768;
}

void custom_srand(unsigned int seed) {
    next = seed;
}
```

Potrebno je napraviti histogram `H[10]` broja pojavljivanja vrednosti unutar matrice po
modulu 10 (`mat[i][j] % 10`) za sve elemente. Pravljenje histograma odraditi kreiranjem `M`
uporednih niti, gde svaka nit računa lokalni histogram za jednu vrstu, a nakon toga taj
histogram spaja sa deljenim histogramom u kojem će se nalaziti finalne vrednosti za celu
matricu. Glavna nit treba da ispiše konačni histogram kada sve niti završe obradu.

## Bodovanje

* **20 poena:** tokom izvršavanja niti, nakon svakih 10 obrađenih elemenata pozvati
  `dispatch`.
* **30 poena:** tokom izvršavanja niti, nakon svakih 10 obrađenih elemenata uspavati nit na
  5 perioda tajmera.

## Implementation

Rešenje je u celosti korisnički program — `test/Modification_test.cpp`, uvezan u
`test/userMain.cpp` kao TEST 8. Nijedan sistemski poziv nije dodavan.

* `M` i `N` se čitaju cifru po cifru preko `getc()` dok se ne naiđe na `'\n'`, pa mogu imati
  proizvoljan broj cifara. Prekidna rutina konzole pretvara `'\r'` u `'\n'`, tako da je
  dovoljno proveravati samo `'\n'`.
* Matrica se alocira dinamički — `new int*[M]`, pa `new int[N]` po vrsti. Brojač je kastovan
  na `unsigned` jer bi za signed brojač `g++` ubacio proveru koja zove
  `__cxa_throw_bad_array_new_length`, a tog simbola nema u `-nostdlib` okruženju.
* Svaka od `M` niti (`HistThread`) računa lokalni histogram za svoju vrstu, pa ga pod
  semaforom `histMutex` spaja u deljeni `Hist[10]`. Semafor je neophodan jer je
  `Hist[i] += localHist[i]` čitaj-izmeni-upiši sekvenca, a preotimanje je uključeno.
* Nakon svakih 10 obrađenih elemenata nit se uspavljuje na 5 perioda tajmera
  (`Thread::sleep(5)`) — varijanta za 30 poena.
* Glavna nit čeka `M` signala na semaforu `finished` i tek onda ispisuje histogram, umesto
  aktivnog čekanja na brojaču završenih niti.

Provereno za 4×5, 40×40 i 100×20 — suma histograma je u svim slučajevima tačno `M*N`.
