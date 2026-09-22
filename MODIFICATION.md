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

Dva nova sistemska poziva, `0x14` (`thread_add_child`) i `0x15` (`thread_join_all`), i tri
nova polja u TCB-u: `sem_t childrenFinished`, `uint64 noOfChildren` i `_thread* parent`.
Svaki roditelj ima svoj semafor — zajednički (statički) semafor ne bi radio, jer bi signali
dece jedne niti otključavali `joinAll` neke druge.

* `thread_add_child` otvara semafor roditelja (lenjo, pri prvom detetu), upisuje
  `child->parent = running` i uvećava brojač. Otvaranje mora da bude ovde, a ne u
  `thread_join_all`: dete koje se završi pre nego što roditelj stigne do `joinAll`
  signaliziralo bi `nullptr` i ta jedinica bi bila izgubljena zauvek.
* `thread_join_all` koristi `_sem::wait(childrenFinished, n)`, gde je `n` broj dece — jedno
  blokiranje za svu decu odjednom, umesto `n` uzastopnih čekanja.
* `_thread::exit` signalizira semafor roditelja sa jednom jedinicom. Pošto se jedinice
  akumuliraju u brojaču, dete koje se završi pre `joinAll` nije izgubljeno — roditeljev
  `wait` tada prolazi bez blokiranja.

Test (TEST 8) prati zadatak: `A` pravi tri niti `B` i jednu `C`, svaki `B` pravi tri `C`.
Redosled u testu je bitan — `addChild` se poziva tek nakon `start()`, jer `Thread::start`
kreira `myHandle` koji se prosleđuje sistemskom pozivu.

Ispis potvrđuje ugnežđeno čekanje: deset puta `Thread C je gotov`, pa tri puta
`Thread B je gotov`, pa `Thread A je gotov`.
