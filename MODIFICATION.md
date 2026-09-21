# barrier()

*Februar 2026. — rok održan 18.03.2025.*

Potrebno je obezbediti da svaka nit može da sačeka sve druge niti koje je korisnik
eksplicitno kreirao. Dodati funkciju `barrier` kao statičku metodu u okviru C++ API-ja u
okviru klase `Thread`. Funkcija pomoću sistemskog poziva treba da sačeka dok je i sve
ostale niti ne pozovu. Tj. kada jedna nit pozove ovu funkciju, sve ostale niti mogu da
napreduju samo do poziva iste ove funkcije. Sve niti nastavljaju izvršavanje tek nakon što
i poslednja nit pozove ovu funkciju.

* **C++ API:** `static void Thread::barrier()`

Garantovano je da će sve niti u sistemu pozvati ovu funkciju isti broj puta.

## Test

Test primer dodati među javne testove kao stavku pod rednim brojem 8. Test primer pravi
`X` niti, gde je `X` broj između 1 i 10 i zadaje se kao globalna konstanta u testu. Niti se
prave pomoću CPP API-ja. Sve niti se na početku naprave, pa se tek onda startuju.

Sve niti rade isti posao — u tri iteracije odrade sledeće:

1. odrade obradu,
2. pozovu funkciju `barrier`,
3. ispišu id niti i broj tekuće iteracije.

`id` je promenljiva u kojoj se nalazi redni broj niti. Prvokreirana nit ima id 1, druga 2
itd.

Obrada u nitima treba da bude:

```cpp
for (int i = 0; i < 1000 * id; i++) {
    for (int j = 0; j < 10000; j++);
    thread_dispatch();
}
```

Moguće je koristiti funkcije za ispis iz zaglavlja `printing.hpp` koje je dato u okviru
javnih testova.

## Implementation

_TBD — filled in once the modification is implemented._
