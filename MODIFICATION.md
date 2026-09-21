# thread_add_child() / thread_join_all()

Dodati sistemske pozive `thread_add_child` i `thread_join_all`. Poziv `thread_add_child`
registruje dete-nit za tekuću (pozivajuću) nit. Poziv `thread_join_all` blokira tekuću nit
dok se sva njena deca ne izvrše.

* **C API:** `thread_add_child`, `thread_join_all`
* **C++ API:** nestatičke metode `Thread::addChild(Thread* child)` i `Thread::joinAll()`

## Test

Napisati test program u kome nit `A` pravi tri niti tipa `B` i jednu nit tipa `C`, a svaka
nit tipa `B` pravi tri niti tipa `C`. Svaki roditelj čeka svu svoju decu pre nego što
nastavi sa izvršavanjem.

## Implementation

_TBD — filled in once the modification is implemented._
