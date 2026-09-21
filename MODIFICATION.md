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

_TBD — filled in once the modification is implemented._
