# send(thread_t handle, char* message) / receive()

Potrebno je obezbediti podršku za slanje i prijem poruka između niti, dodavanjem sledećih
funkcija i metoda, kao i odgovarajućih sistemskih poziva koji implementiraju sledeće
funkcionalnosti:

* **C API:** `void send(thread_t handle, char* message)`
  **C++ API:** `void Thread::send(char* message)`

  Pozivajuća nit šalje poruku datoj niti (`this`); ukoliko je ovoj niti već stigla neka
  poruka koju ona nije preuzela, pozivajuća nit se suspenduje dok se prethodna poruka ne
  preuzme i tek onda ostavlja poruku i nastavlja izvršavanje.

* **C API:** `char* receive()`
  **C++ API:** `static char* Thread::receive()`

  Pozivajuća nit preuzima poruku koja joj je poslata; ukoliko poruke nema, pozivajuća nit
  se suspenduje dok poruka ne stigne.

## Test

Test primer dodati među javne testove kao stavku pod rednim brojem 8. Test primer pravi tri
niti: A, B i C. Sve niti u petlji obavljaju 5 puta sledeću obradu:

* Nit A šalje poruku niti B (`"Nit A -> Nit B"`), pa zatim niti C (`"Nit A -> Nit C"`), a
  zatim prima poruku i ispisuje je.
* Nit B šalje dve poruke niti C (`"Nit B -> Nit C #1"`, `"Nit B -> Nit C #2"`), a zatim
  prima dve poruke i ispisuje ih.
* Nit C prima 3 poruke i ispisuje ih, zatim šalje poruku niti B (`"Nit C -> Nit B"`), a
  zatim šalje poruku niti A (`"Nit C -> Nit A"`).

Sve niti na kraju svoje obrade ispisuju da su se završile.

## Implementation

_TBD — filled in once the modification is implemented._
