# Projektni zadatak — Operativni sistemi 1

**Elektrotehnički fakultet u Beogradu**
Katedra za računarsku tehniku i informatiku
Predmet: **Operativni sistemi 1**
Nastavnik: prof. dr Dragan Milićev
Školska godina: 2025/2026.
Projekat za domaći rad
Verzija dokumenta: **1.0**

> **Važne napomene:** Pre čitanja ovog teksta, obavezno pročitati opšta pravila predmeta i pravila vezana za izradu domaćih zadataka! Pročitati potom ovaj tekst u celini i pažljivo, pre započinjanja realizacije ili traženja pomoći. Ukoliko u zadatku nešto nije dovoljno precizno definisano ili su postavljeni kontradiktorni zahtevi, student treba da uvede razumne pretpostavke, da ih temeljno obrazloži i da nastavi da izgrađuje preostali deo svog rešenja na temeljima uvedenih pretpostavki. Zahtevi su namerno nedovoljno detaljni, jer se od studenata očekuje kreativnost i profesionalni pristup u rešavanju praktičnih problema!

---

## Sadržaj

- [Uvod](#uvod)
- [Opšti zahtevi](#opšti-zahtevi)
  - [Odnos jezgra i korisničke aplikacije](#odnos-jezgra-i-korisničke-aplikacije)
  - [Odnos jezgra i sistema-domaćina](#odnos-jezgra-i-sistema-domaćina)
- [Interfejs jezgra](#interfejs-jezgra)
  - [C API](#c-api)
  - [ABI](#abi)
  - [C++ API](#c-api-1)
- [Opis platforme](#opis-platforme)
  - [Kratak prikaz arhitekture i asemblera procesora RISC-V](#kratak-prikaz-arhitekture-i-asemblera-procesora-risc-v)
    - [Osnovne karakteristike arhitekture](#osnovne-karakteristike-arhitekture)
    - [Pregled nekih instrukcija i načina adresiranja](#pregled-nekih-instrukcija-i-načina-adresiranja)
    - [Konvencije C/C++ prevodioca za poziv funkcije](#konvencije-cc-prevodioca-za-poziv-funkcije)
    - [Obrada sistemskih poziva, izuzetaka i prekida](#obrada-sistemskih-poziva-izuzetaka-i-prekida)
    - [Konzola](#konzola)
  - [Uputstva za razvojno okruženje](#uputstva-za-razvojno-okruženje)
  - [Uputstva za izvršno okruženje](#uputstva-za-izvršno-okruženje)
- [Smernice za rešavanje zadatka](#smernice-za-rešavanje-zadatka)
  - [Arhitektura i glavne projektne odluke](#arhitektura-i-glavne-projektne-odluke)
  - [Ključne apstrakcije](#ključne-apstrakcije)
  - [Preotimanje, promena konteksta i raspoređivanje](#preotimanje-promena-konteksta-i-raspoređivanje)
  - [Implementacija nekih zahtevanih funkcionalnosti](#implementacija-nekih-zahtevanih-funkcionalnosti)
  - [Ulaz/izlaz](#ulazizlaz)
  - [Asinhroni prekid i tajmer](#asinhroni-prekid-i-tajmer)
  - [Implementacija interfejsnih slojeva](#implementacija-interfejsnih-slojeva)
  - [Predlog redosleda izrade](#predlog-redosleda-izrade)
- [Način ocenjivanja](#način-ocenjivanja)
  - [Način predaje projekta](#način-predaje-projekta)
  - [Način ocenjivanja projekta](#način-ocenjivanja-projekta)
  - [Način provere projekta](#način-provere-projekta)

---

## Uvod

Cilj ovog projekta jeste realizacija malog, ali sasvim funkcionalnog jezgra (engl. *kernel*) operativnog sistema koji podržava niti (engl. *multithreaded operating system*) sa deljenjem vremena (engl. *time sharing*). U daljem tekstu, ovakav sistem biće kratko nazivan samo **jezgrom**.

U okviru ovog projekta treba realizovati alokator memorije i upravljanje nitima. Jezgro treba da obezbedi koncept niti (engl. *thread*), semafora i podršku deljenju vremena (engl. *time sharing*), kao i asinhronu promenu konteksta i preotimanje (engl. *preemption*) na prekid od tajmera i od tastature.

Jezgro treba da bude realizovano kao „bibliotečno“, tako da korisnički program (aplikacija) i samo jezgro dele isti adresni prostor, odnosno da predstavljaju statički povezan jedinstven program unapred učitan u operativnu memoriju računara. Konkurentni procesi kreirani unutar aplikacije biće zapravo samo „laki“ procesi, tj. niti (engl. *thread*) pokrenuti unutar tog programa. Ovakva konfiguracija karakteristična je za ugrađene (engl. *embedded*) sisteme, koji ne izvršavaju proizvoljne programe koji se učitavaju i izvršavaju na zahtev korisnika, već izvršavaju samo onaj program (zajedno sa operativnim sistemom) koji je već ugrađen u ciljni hardver.

Jezgro se implementira za arhitekturu procesora **RISC-V** i školskog računara sa ovim procesorom. Za implementaciju se može koristiti asembler za ovaj procesor i jezik C/C++. Implementirano jezgro će se izvršavati u virtuelnom okruženju – emulatoru procesora RISC-V.

---

## Opšti zahtevi

### Odnos jezgra i korisničke aplikacije

Jezgro treba realizovati na jeziku **C++**, uz korišćenje asemblera za ciljni procesor po potrebi. Korisnička aplikacija sadržaće test primere i biće obezbeđena kao skup izvornih fajlova koje treba prevesti i povezati sa prevedenim kôdom jezgra i datim bibliotekama `app.lib` i `hw.lib` u jedinstven program (`.exe`). Biblioteka `app.lib` sadržaće preveden i povezan kôd korisničkog test programa. Biblioteka `hw.lib` sadržaće module koji se obezbeđuju kao moduli koji pristupaju (zamišljenom, virtuelnom) hardveru, odnosno moduli od kojih će jezgro zavisiti (engl. *stub*).

Glavni program, tj. izvor kontrole toka korisničke aplikacije treba da bude u funkciji:

```cpp
void userMain ();
```

Funkcija `main()` nema argumente niti povratnu vrednost i ostaje u nadležnosti samog jezgra, pa realizacija jezgra može da pod svojom kontrolom ima radnje koje će se izvršiti pri pokretanju programa, a zatim treba da pokrene nit nad funkcijom `userMain()`.

### Odnos jezgra i sistema-domaćina

Jezgro i korisnička aplikacija treba da se posmatraju kao jedinstven izvršni program dobijen prevođenjem i statičkim povezivanjem kôda na izvornom programskom jeziku na kom su realizovani. Oni će biti pokrenuti unutar edukativnog operativnog sistema **xv6** kao sistema-domaćina. Ovaj sistem xv6 je za ovu priliku značajno modifikovan tako što su mu izbačene mnoge funkcionalnosti (promena konteksta i raspoređivanje procesa, upravljanje memorijom, fajl podsistem, upravljanje diskom itd.). Sistem-domaćin ima ulogu samo da se sam pokrene i inicijalizuje ciljni hardver, potom napravi samo jedan proces sa virtuelnim adresnim prostorom koji zauzima celu raspoloživu fizičku memoriju, učita kôd programa koji čine realizovano jezgro i sa njim povezana aplikacija i zatim pokrene njegovo izvršavanje u kontekstu tog jedinog procesa. Osim toga, sistem-domaćin obezbeđuje i osnovne usluge hardvera: periodičan prekid od tajmera i pristup konzoli (tastaturi i ekranu). Na PC računarima i njihovim operativnim sistemima cela ova računarska platforma (procesor RISC-V, tajmer i konzola, kao i sistem-domaćin xv6) simuliraju se odgovarajućom virtuelnom mašinom (emulatorom).

Sve ovo jednostavno znači da će realizovano jezgro posmatrati svoju platformu kao jednostavan računar sa RISC-V procesorom i jedinstvenim adresnim prostorom operativne memorije (samo fizički adresni prostor) u koji je učitan izvršiv program dobijen povezivanjem jezgra i aplikacije, kao što je to slučaj kod ugrađenih sistema.

Jezgro i korisnička aplikacija moraju da se kao program regularno završe, naravno ukoliko u samoj korisničkoj aplikaciji nema neregularnosti. To znači da po završetku svih niti pokrenutih u korisničkoj aplikaciji ceo program treba regularno da se završi. Test primeri ispitivača biće regularni, pa svaki neregularan završetak programa znači neregularnost u samom jezgru, osim ako za neki konkretan test primer nije drugačije naznačeno.

U realizaciji jezgra **nije dozvoljeno koristiti usluge operativnog sistema-domaćina** niti operativnog sistema PC računara na kom se sve ovo izvršava, a koje se odnose na koncepte niti ili procesa, semafora, prekida, sinhronizacije i komunikacije između niti ili procesa, itd. Drugim rečima, sve zahtevane koncepte i funkcionalnosti potrebno je realizovati u potpunosti samostalno i „od nule“.

Osim biblioteka eksplicitno navedenih ovde koje će biti posebno pripremljene, u realizaciji jezgra ne treba koristiti nikakve druge, pa ni standardne C/C++ biblioteke, statičke ili dinamičke, jer one po pravilu sadrže sistemske pozive operativnog sistema-domaćina.

---

## Interfejs jezgra

Jezgro treba da obezbedi tri vrste interfejsa prema korisničkom programu, slojevito organizovane kao na sledećoj slici. Neki sloj A, koji je na slici prikazan iznad drugog sloja B, koristi usluge tog drugog sloja B da bi ispunio svoje odgovornosti prema sloju prikazanom iznad sloja A.

```
┌─────────────────────────────────┐
│   User's program (app.lib)      │
├─────────────────────────────────┤
│   C++ OO API            ◄── osenčeno (realizovati)
├─────────────────────────────────┤
│   C API                 ◄── osenčeno (realizovati)
├─────────────────────────────────┤
│   ABI                   ◄── osenčeno (realizovati)
╞═════════════════════════════════╡  ◄ granica privilegovanog režima
│   Kernel                ◄── osenčeno (realizovati)
├─────────────────────────────────┤
│   HW access module (hw.lib)     │
└─────────────────────────────────┘
```

Zadatak je realizovati sve osenčene slojeve softvera. Modul za pristup hardveru (`hw.lib`) biće raspoloživ i dat u obliku statičke biblioteke. Korisnički program (`app.lib`) za testiranje biće dat kao statička biblioteka, a student treba da napravi i svoje primere korisničkog programa koji testiraju jezgro. Svi prikazani slojevi statički su povezani u jedinstven izvršni program.

Programski kôd svih prikazanih slojeva izvršava se u istom (jedinstvenom) adresnom prostoru. Programski kôd jezgra i modula za pristup hardveru (softver ispod tamne debele linije) izvršava se u **privilegovanom (sistemskom) režimu** rada ciljnog procesora, dok se slojevi iznad jezgra izvršavaju u **neprivilegovanom (korisničkom) režimu** rada procesora. Jezgro (i njegova funkcija `main`) će inicijalno biti pokrenuti u sistemskom režimu.

- **ABI** (engl. *application binary interface*) je binarni interfejs sistemskih poziva koji se vrše pomoću softverskog prekida ciljnog procesora. Ovaj sloj obezbeđuje prenos argumenata sistemskog poziva preko registara procesora, prelazak u privilegovani režim rada procesora i prelazak na kôd jezgra.
- **C API** (engl. *application programming interface*) je klasičan, proceduralan (ne objektno orijentisan) programski interfejs sistemskih poziva implementiran kao skup funkcija. Ove funkcije u svojoj implementaciji mogu imati jedan sistemski poziv, više njih ili nijedan sistemski poziv iz sloja ABI, u zavisnosti od svoje uloge. Tako su ove funkcije zapravo omotač (engl. *wrapper*) oko interfejsa ABI.
- **C++ API** je objektno orijentisan API koji pruža objektni pogled na koncepte koje jezgro podržava. Implementiran je kao jednostavan objektno orijentisan omotač oko funkcija iz sloja C API pisan na jeziku C++.

### C API

Funkcije ovog interfejsa opisane su u sledećoj tabeli, a deklaracije su date u fajlu `syscall_c.hpp`.

| Broj | Potpis | Objašnjenje |
|------|--------|-------------|
| `0x01` | `void* mem_alloc (size_t size);` | Alocira prostor od (najmanje) `size` bajtova memorije, zaokruženo i poravnato na blokove veličine `MEM_BLOCK_SIZE`. Vraća pokazivač na deo alociranog prostora od kojeg do kraja datog prostora ima (najmanje) `size` bajtova u slučaju uspeha, a `null` u slučaju neuspeha. `MEM_BLOCK_SIZE` je celobrojna konstanta veća od ili jednaka 64, a manja od ili jednaka 1024. |
| `0x02` | `int mem_free (void*);` | Oslobađa prostor prethodno alociran pomoću `mem_alloc`. Vraća 0 u slučaju uspeha, negativnu vrednost u slučaju greške (kôd greške). Argument mora imati vrednost vraćenu iz `mem_alloc`. Ukoliko to nije slučaj, ponašanje je nedefinisano. |
| `0x11` | `class _thread;`<br>`typedef _thread* thread_t;`<br>`int thread_create (thread_t* handle, void(*start_routine)(void*), void* arg);` | Pokreće nit nad funkcijom `start_routine`, pozivajući je sa argumentom `arg`. U slučaju uspeha, u `*handle` upisuje „ručku“ novokreirane niti i vraća 0, a u slučaju neuspeha vraća negativnu vrednost (kôd greške). „Ručka“ je interni identifikator koji jezgro koristi da bi identifikovalo nit. |
| `0x12` | `int thread_exit ();` | Gasi tekuću nit. U slučaju neuspeha vraća negativnu vrednost (kôd greške). |
| `0x13` | `void thread_dispatch ();` | Potencijalno oduzima procesor tekućoj i daje nekoj drugoj (ili istoj) niti. |
| `0x21` | `class _sem;`<br>`typedef _sem* sem_t;`<br>`int sem_open (sem_t* handle, unsigned init);` | Kreira semafor sa inicijalnom vrednošću `init`. U slučaju uspeha, u `*handle` upisuje ručku novokreiranog semafora i vraća 0, a u slučaju neuspeha vraća negativnu vrednost (kôd greške). |
| `0x22` | `int sem_close (sem_t handle);` | Oslobađa semafor sa datom ručkom. Sve niti koje su se zatekle da čekaju na semaforu se deblokiraju, pri čemu njihov `wait` vraća grešku. U slučaju uspeha vraća 0, inače negativnu vrednost. |
| `0x23` | `int sem_wait (sem_t id);` | Operacija `wait` na semaforu sa datom ručkom. U slučaju uspeha vraća 0, a u slučaju neuspeha (uključujući i slučaj kada je semafor dealociran dok je nit čekala) vraća negativnu vrednost. |
| `0x24` | `int sem_signal (sem_t id);` | Operacija `signal` na semaforu sa datom ručkom. U slučaju uspeha vraća 0, inače negativnu vrednost. |
| `0x25` | `int sem_wait_n (sem_t id, unsigned n);` | Operacija `wait` koja zahteva `n` jedinica resursa. Ako je trenutna vrednost semafora ≥ `n`, vrednost se umanjuje za `n` i funkcija se odmah uspešno završava. U suprotnom se pozivajuća nit blokira dok ne postane moguće izvršiti operaciju (ili dok semafor ne bude dealociran). |
| `0x26` | `int sem_signal_n (sem_t id, unsigned n);` | Operacija `signal` koja povećava vrednost semafora za `n`. Ukoliko postoje niti koje čekaju, može doći do deblokiranja jedne ili više njih. |
| `0x31` | `typedef unsigned long time_t;`<br>`int time_sleep (time_t);` | Uspavljuje pozivajuću nit na zadati period u internim jedinicama vremena (periodama tajmera). U slučaju uspeha vraća 0, inače negativnu vrednost. |
| `0x41` | `const int EOF = -1;`<br>`char getc ();` | Učitava jedan znak iz bafera znakova učitanih sa konzole. Ako je bafer prazan, suspenduje pozivajuću nit dok se znak ne pojavi. Vraća učitani znak u slučaju uspeha, a konstantu `EOF` u slučaju greške. |
| `0x42` | `void putc (char);` | Ispisuje dati znak na konzolu. |

Nit se kreira sa podrazumevanom veličinom steka (`DEFAULT_STACK_SIZE`) i podrazumevanom veličinom vremenskog odreska (`DEFAULT_TIME_SLICE`). Ove konstante deklarisane su u `hw.h`, a definisane u `hw.lib`:

```cpp
extern const size_t DEFAULT_STACK_SIZE;
extern const time_t DEFAULT_TIME_SLICE;
```

Memorijski prostor koji je slobodan za alokaciju počinje od adrese `HEAP_START_ADDR`, a završava se na adresi `HEAP_END_ADDR-1`. Ove konstante, kao i konstanta `MEM_BLOCK_SIZE`, deklarisane su u `hw.h`, a definisane u `hw.lib`:

```cpp
extern const void* HEAP_START_ADDR, HEAP_END_ADDR;
extern const size_t MEM_BLOCK_SIZE;
```

### ABI

Sistemski poziv u ovom sloju obavlja se softverskim prekidom (odgovarajućom instrukcijom procesora). Parametri se u sistemski poziv prenose kroz registre procesora na sledeći način:

- `a0`: kôd sistemskog poziva, jednak broju iz prve kolone tabele date za C API;
- `a1, a2, ...`: parametri sistemskog poziva, redom po potpisu iz C API-a sleva nadesno;
- `a0`: povratna vrednost.

Svi potpisi navedenih sistemskih poziva iz C API-a se u potpunosti preslikavaju navedenom konvencijom na odgovarajuće ABI pozive i imaju istu semantiku, osim sledećih izuzetaka:

- Sistemski poziv broj `0x01`, `mem_alloc`, ima isti potpis, samo što parametar (`size`) izražava veličinu prostora u **blokovima**, a ne u bajtovima. To znači da funkcija `mem_alloc` iz C API-a treba da zadatu vrednost u bajtovima zaokruži na cele blokove (tako da zahtevani prostor stane u te blokove) i izrazi u blokovima pre nego što izvrši ovaj sistemski poziv ABI-a.
- Sistemski poziv broj `0x11`, `thread_create`, ima ABI potpis ekvivalentan sledećem C potpisu:

```cpp
int thread_create (
    thread_t* handle,
    void(*start_routine)(void*),
    void* arg,
    void* stack_space
);
```

Kreira nit nad funkcijom `start_routine`, pozivajući je sa argumentom `arg`, smeštajući stek te niti u već odvojen prostor na čiju poslednju lokaciju ukazuje `stack_space`. To znači da sistemski poziv u ABI sloju kreira nit sa zadatom pozicijom steka, pa funkcija `thread_create` iz C API-a treba najpre da alocira stek (sistemskim pozivom `mem_alloc`) pre nego što izvrši ovaj sistemski poziv ABI-a.

### C++ API

Klase ovog interfejsa treba definisati u fajlu `syscall_cpp.hpp`. Interfejsi ovih klasa imaju sledeći oblik:

```cpp
#ifndef _syscall_cpp
#define _syscall_cpp

#include "syscall_c.hpp"

void* ::operator new (size_t);
void ::operator delete (void*);

class Thread {
public:
    Thread (void (*body)(void*), void* arg);
    virtual ~Thread ();
    int start ();
    static void dispatch ();
    static int sleep (time_t);
protected:
    Thread ();
    virtual void run () {}
private:
    thread_t myHandle;
    void (*body)(void*); void* arg;
};

class Semaphore {
public:
    Semaphore (unsigned init = 1);
    virtual ~Semaphore ();
    int wait ();
    int signal ();
private:
    sem_t myHandle;
};

class PeriodicThread : public Thread {
public:
    void terminate ();
protected:
    PeriodicThread (time_t period);
    virtual void periodicActivation () {}
private:
    time_t period;
};

class Console {
public:
    static char getc ();
    static void putc (char);
};

#endif
```

Globalne operatorske funkcije `new` i `delete` treba implementirati tako da obmotavaju sistemske pozive `mem_alloc` i `mem_free`, respektivno. Apstraktna klasa `PeriodicThread` služi za podršku periodičnim nitima sa periodom zadatom konstruktorom — korisnik treba da izvede klasu iz ove i redefiniše polimorfnu operaciju `periodicActivation`. Klasa `Console` je uslužna, fasadna klasa koja samo obezbeđuje odgovarajući prostor imena za operacije koje obmotavaju sistemske pozive za pristup konzoli.

U slučaju da je redefinisana operacija `run` u izvedenoj klasi, ali i pozvan konstruktor osnovne klase koji prima pokazivač na funkciju (nekorektan način korišćenja), ponašanje treba da bude kao da je samo postavljen pokazivač na funkciju. Drugim rečima, ukoliko je konstruktorom postavljen pokazivač na funkciju, operaciju `run` treba ignorisati u svakom slučaju.

Postojanje destruktora u interfejsu označava da se dati objekti mogu uništavati i da je eventualno potrebno obezbediti odgovarajuću sinhronizaciju ili dealokaciju sistemskim pozivom. Greške vraćene iz sistemskih poziva signalizirati povratnim vrednostima (gde je to moguće) na isti način kao i u C API-ju.

Zadatak je u potpunosti implementirati sve ove klase i operacije. Kako bi korisnički kôd iz `app.lib` bio kompatibilan sa ovim definicijama, definicije ovih klasa **ne smeju se proširivati** bilo kakvim nestatičkim podacima članovima, osnovnim klasama, niti se sme menjati redosled i skup virtuelnih funkcija članica. Ne smeju se menjati ni potpisi postojećih funkcija članica.

---

## Opis platforme

### Kratak prikaz arhitekture i asemblera procesora RISC-V

Razmatra se varijanta **RV64IMA** procesora RISC-V. Zvanična i potpuna specifikacija ("The RISC-V Instruction Set Manual") data je na: <https://riscv.org/technical/specifications/>

#### Osnovne karakteristike arhitekture

- 64-bitni troadresni RISC procesor sa load/store arhitekturom. Svi registri su 64-bitni.
- Adresibilna jedinica je bajt. Podaci širine 1, 2, 4 ili 8 bajtova. Podrazumevano *little endian*.
- Instrukcije su širine 32 bita.
- Dva režima rada relevantna za projekat: **korisnički** i **sistemski** (treći je irelevantan).
- Registri dostupni i u korisničkom i u sistemskom režimu:[^1]
  - `sp`: pokazivač steka;
  - `ra`: registar za povratnu adresu pozvanog potprograma;
  - `s0..s11`: registri opšte namene čiju će vrednost očuvati pozvani potprogram;
  - `a0..a7`: registri za prenos argumenata i povratne vrednosti potprograma;
  - `t0..t6`: privremeni registri;
  - `zero`: „registar“ ožičen na nulu (upis nema efekta, čitanje uvek vraća 0);
  - `gp` i `tp`: registri posebne namene.
- Registri dostupni **samo u sistemskom režimu**:
  - `sstatus`: statusni registar;
  - `sip` i `sie`: registri za prekide;
  - `sscratch`: privremeni registar;
  - `sepc`: sačuvana vrednost registra `pc` u korisničkom režimu;
  - `scause`: opis razloga za prelazak u sistemski režim;
  - `stvec`: adresa prekidne rutine, poravnata na 4 bajta.
- Registar `pc` ukazuje na tekuću instrukciju i nije direktno dostupan programu za upis.
- **Stek:**[^2]
  - raste ka nižim adresama;
  - `sp` pokazuje na poslednju zauzetu lokaciju;
  - vrednost registra `sp` mora biti deljiva sa 16.
- RV64IMA podržava samo sledeće kategorije instrukcija:
  - **I** – celobrojne instrukcije;
  - **M** – množenje/deljenje celih brojeva;
  - **A** – atomične instrukcije (sadrže i spin lock instrukcije, ali kako je sistem jednoprocesorski, one nisu neophodne).
- Usled odsustva F, D i Q ekstenzija, procesor nema direktnu podršku za aritmetiku u pokretnom zarezu — ne treba je koristiti ni u jezgru ni u korisničkom programu.

#### Pregled nekih instrukcija i načina adresiranja

- **Load/store instrukcije:**
  ```
  l{b|h|w|d} reg, offset(reg)
  s{b|h|w|d} reg, offset(reg)
  ```
  Sufiks `b`, `h`, `w`, `d` definiše širinu podatka (1B, 2B, 4B, 8B). `offset(reg)` je registarsko indirektno adresiranje sa označenim pomerajem širine 12 bita.
- **`call`**: pseudoinstrukcija za poziv funkcije (operand je ime funkcije, npr. `call f`); obično se prevodi u relativan skok pomoću `auipc` i `jalr`. `auipc` sabira `pc` sa pomerajem; `jalr` skače na adresu u registru i pamti povratnu adresu u `ra`.
- **`csr`**: instrukcije za pristup sistemskim registrima:
  - `csrr reg, sreg`: upis vrednosti sistemskog registra `sreg` u korisnički registar `reg`;
  - `csrw reg, sreg`: upis vrednosti korisničkog registra `reg` u sistemski registar `sreg`;
  - `csrrw reg1, sreg, reg2`: u `sreg` se upisuje `reg2`, a stara vrednost `sreg` u `reg1`.
- Asemblerski fajl mora imati ekstenziju `.S`.
- Pojedinačne instrukcije se mogu ugraditi u C/C++ kôd pomoću asemblerskog bloka (primeri za gcc):

```cpp
uint64 x;
// Upis vrednosti registra sstatus u automatsku promenljivu x
asm volatile("csrr %0, sstatus" : "=r" (x));
// Upis vrednosti automatske promenljive x u registar sstatus
asm volatile("csrw sstatus, %0" : : "r" (x));
// Instrukcija koja učitava podatak iz memorije u registar s0
asm volatile("ld s0, 8(sp)");
```

#### Konvencije C/C++ prevodioca za poziv funkcije

- Pozvana funkcija je dužna da po povratku očuva vrednosti registara opšte namene (`s0..s11`) i registra `sp`.
- Povratna adresa se pamti u registru `ra`.
- Parametri se prenose preko `a0…a7` (sleva nadesno), a preko memorije ako ne stanu u registre.
- Povratna vrednost se prenosi preko `a0` i po potrebi `a1`.
- Pseudoinstrukcija `ret` upisuje vrednost `ra` u `pc`.

Primer prevoda funkcije:

```c
int f (int a, int b) {
    if (a == 0)
        return 0;
    return f(a - 1, b) + b;
}
```

```asm
f:
    bnez a0, offset   // bnez – branch on not equal to zero
    ret               // u a0 je povratna vrednost
offset:
    addi sp, sp, -32  // prostor na steku za sačuvane i privremene vrednosti
    sd ra, 24(sp)     // čuva povratnu adresu pre poziva funkcije
    sd s0, 16(sp)     // čuva registre iz grupe s0..s11 koje koristi
    sd s1, 8(sp)
    addi s0, sp, 32   // s0 se koristi kao „frame pointer“[^5]
    mv s1, a1         // čuva vrednost a1 u s1 jer može da se promeni
    addiw a0, a0, -1  // argumenti poziva funkcije se stavljaju u a0 i a1
    // Naredne dve instrukcije rezultat pseudoinstrukcije call f
    auipc ra, 0x0
    jalr -36(ra)      // skok na funkciju f uz čuvanje povratne adrese u ra
    addw a0, a0, s1
    ld ra, 24(sp)     // restaurira se vrednost ra radi povratka
    ld s0, 16(sp)     // restaurira registre iz grupe s0..s11
    ld s1, 8(sp)
    addi sp, sp, 32   // sp se vraća na prethodno stanje
    ret
```

#### Obrada sistemskih poziva, izuzetaka i prekida

- U sistemski režim prelazi se instrukcijom softverskog prekida `ecall`, izuzetkom ili spoljašnjim prekidom.
- Prilikom obrade, procesor radi sledeće:
  - vrednost `pc` upisuje u `sepc` (adresa instrukcije `ecall` ili prve neizvršene/prekinute instrukcije);
  - u `sstatus` upisuje:
    - u bit **SPP** (bit 8) vrednost koja pokazuje iz kog režima se dogodio skok (0 – korisnički, 1 – sistemski);
    - u bit **SIE** (bit 1) nulu, čime se maskiraju spoljašnji prekidi; u korisničkom režimu se ovaj bit ignoriše;
    - u bit **SPIE** (bit 5) prethodnu vrednost bita SIE.
  - u `scause` upisuje:
    - u bit najveće težine (BNT) informaciju o tome da li se dogodio spoljašnji prekid;
    - u ostale bite razlog, prema tabeli:

| BNT | Vrednost | Opis |
|-----|----------|------|
| 1 | 1 | Softverski prekid iz trećeg, najprivilegovanijeg režima rada procesora[^6] |
| 1 | 9 | Spoljašnji hardverski prekid |
| 0 | 2 | Ilegalna instrukcija |
| 0 | 5 | Nedozvoljena adresa čitanja |
| 0 | 7 | Nedozvoljena adresa upisa |
| 0 | 8 | `ecall` iz korisničkog režima |
| 0 | 9 | `ecall` iz sistemskog režima |

- Registar `stvec` sadrži oznaku režima **MODE** (najniža dva bita) i baznu adresu **BASE**. MODE = 0 (direktni režim): procesor skače na `BASE` za `ecall`, izuzetke i spoljašnje prekide.[^7] MODE = 1 (vektorski režim): za `ecall` i izuzetke skače na `BASE`, a za spoljašnje prekide na `BASE + 4 × broj_prekida`.
- Sistemski registri se čitaju instrukcijom `csr`, dostupnom samo u sistemskom režimu.
- **Povratak iz sistemskog režima** radi se instrukcijom `sret` (dostupna samo u sistemskom režimu):
  - režim u koji se prelazi definisan je bitom SPP;
  - bit SIE dobija vrednost bita SPIE;
  - registar `pc` dobija vrednost registra `sepc`.
- Ostali registri se ne čuvaju hardverski — to je odgovornost prekidne rutine.
- Registar `sip` sadrži aktivne zahteve za prekid. Bit **SSIP** (bit 1) označava softverski prekid (upis 1 postavlja zahtev, upis 0 označava obrađen prekid). Bit **SEIP** (bit 9) označava spoljašnji hardverski prekid.
- Registar `sie` je registar za maskiranje prekida. Bit **SSIE** (bit 1) — softverski prekidi; bit **SEIE** (bit 9) — spoljašnji hardverski prekidi. Ako se izvršava u sistemskom režimu i bit SIE u `sstatus` je 0, vrednost `sie` se ignoriše.
- **Prekid od tajmera** realizovan je kao softverski prekid.[^8] Prepoznaje se po vrednosti 1 samo u bitima najmanje i najveće težine u `scause`. Tajmer generiše prekid **deset puta u sekundi**.
- **Prekid od konzole** realizovan je kao spoljašnji hardverski prekid. Postoji kontroler prekida; funkcija `plic_claim` (deklarisana u `hw.h`) vraća broj prekida — prekid od konzole ima broj **10 (`0x0a`)**. Nakon obrade, kontroler se obaveštava funkcijom `plic_complete` (parametar je broj obrađenog prekida).

#### Konzola

- Konzola je eksterni terminal sa kojim računar komunicira serijskom vezom (UART protokol).[^9] Program interaguje samo sa kontrolerom serijske veze.
- Kontroler ima interni bafer za prijem; podatak stiže na svaki pritisnut taster. Na pojavu prvog znaka u baferu kontroler generiše prekid; znakovi se mogu čitati dok je bit spremnosti postavljen.
- Slično važi za slanje: kada je kontroler spreman za slanje, generiše prekid; podaci se prenose dok je bit spremnosti postavljen.
- Kontroler generiše **isti prekid** i kad je spreman za prijem (slanje na konzolu) i kad ima znak sa tastature spreman za čitanje.
- Kontroler poseduje tri registra (jedan statusni, jedan za prijem, jedan za slanje). Registri za podatke su veličine 1 bajt. Adrese su date kao konstante u `hw.h`: `CONSOLE_STATUS`, `CONSOLE_TX_DATA`, `CONSOLE_RX_DATA`. U statusnom registru bit 0 označava da se podatak može pročitati, a bit 5 da kontroler može primiti podatak za slanje. U okviru jednog prekida mogu se prebacivati podaci dok su odgovarajući statusni biti na jedinici.

**Zaustavljanje emulatora iz programskog koda:** Upisom 32-bitne vrednosti `0x5555` na adresu `0x100000` emulator RISC-V procesora se zaustavlja. Na ovaj način je moguće zaustaviti proces emulatora nakon što završi korisnički program.

### Uputstva za razvojno okruženje

Razvojno okruženje je dato u okviru virtuelne mašine koja se pokreće na PC računaru amd64 arhitekture pod Windows ili Linux uz **VMWare Workstation Player** (besplatan za nekomercijalnu upotrebu). VM je dostupna na sajtu predmeta. Ista VM biće dostupna na laboratorijskim računarima tokom odbrane. **Za vreme odbrane veza ka internetu je isključena**, pa student ne može instalirati dodatni softver.

Preporučeno razvojno okruženje je **CLion** (instalirano na VM; akademska licenca uz `student.etf.bg.ac.rs` imejl; plivajuće licence u laboratoriji).

U arhivi na sajtu predmeta nalaze se: biblioteke (`hw.lib`, `mem.lib`, `console.lib`), zaglavlja, `Makefile`, pomoćni fajlovi za povezivanje i debagovanje, i direktorijumi `src` i `h` za studentski kôd. Direktorijum se učitava u CLion preko „Open project“.

Program se prevodi i pokreće komandom `make`[^10] uz dati `Makefile`. Fajlovi `.cpp` se prevode kao C++, `.S` kao asembler, zaglavlja iz `h`. Rezultat je binarni izvršni fajl `kernel` za RISC-V. **Student ne treba da menja `Makefile`**, osim ako ne želi neku od biblioteka (`mem.lib` ili `console.lib`). U `Makefile` postoji linija:

```make
LIBS = lib/mem.lib lib/hw.lib lib/console.lib
```

Ako neka biblioteka nije potrebna, samo je ukloniti iz te linije (npr. izbrisati `lib/mem.lib`).

`make` se iz CLion-a pokreće opcijom „Make“ uz cilj (engl. *target*). Tri cilja: **`qemu`** (prevođenje i pokretanje), **`qemu-gdb`** (pokretanje u režimu debagovanja), **`clean`** (čišćenje). Za prevođenje se koristi `gcc` za RISC-V.

### Uputstva za izvršno okruženje

Za pokretanje se koristi emulator **qemu** (emulira RISC-V procesor i periferije). `make` ciljevi `qemu`/`qemu-gdb` pokreću qemu, zadaju konfiguraciju i izvršni fajl, i povezuju ga sa terminalom razvojnog okruženja kao standardni ulaz/izlaz. Nakon završetka programa qemu ostaje uključen — gasi se opcijom „Stop“.

qemu pruža **udaljeno debagovanje**[^12]. Cilj `qemu-gdb` pokreće qemu za debagovanje; qemu osluškuje na portu čiji broj ispisuje na terminalu i ne pokreće program dok se ne započne debagovanje. Sesija se uspostavlja iz `gdb` dostavljanjem IP adrese i porta (npr. `localhost:26000`). Sve mogućnosti CLion debagovanja se mogu koristiti (breakpoint, watch, instrukcija po instrukcija). `gdb` komanda `info reg` ispisuje sadržaj svih programski dostupnih registara. Prekid se postiže opcijom „Stop“.

---

## Smernice za rešavanje zadatka

> Sadržaj ovog poglavlja **nije deo obavezujućih zahteva**, već pomoć u implementaciji. Student je slobodan da napravi svoje, drugačije rešenje, uz dobre tehničke razloge.

### Arhitektura i glavne projektne odluke

Zahtevani sistem je daleko od jednostavnog i složenošću prevazilazi dotadašnje studentske zadatke. Važno je poštovati principe softverskog inženjerstva. **Ne treba žuriti ka kodovanju** (engl. *rush to code*) — najpre osmisliti arhitekturu, module, njihove odgovornosti i interfejse, rešiti ključne mehanizme (uz odbacive prototipove), pa tek onda krenuti u implementaciju iterativno i inkrementalno, uz temeljno testiranje.

#### Arhitektura

Predlaže se jezgro realizovati kao **monolitan softver** — sav kôd jezgra izvršava se u istom adresnom prostoru, u privilegovanom režimu, a međusobni pozivi usluga unutar jezgra su obični pozivi potprograma u istom toku kontrole. To podrazumeva dobru objektnu dekompoziciju na klase i hijerarhije, uz polimorfne pozive gde je primereno. Arhitektura interfejsa već je definisana kao slojevita.

#### Monoprocesorski ili multiprocesorski sistem

Ovaj projekat podrazumeva implementaciju **jednoprocesorskog** jezgra, pa su pitanja identifikacije procesora, spin lock-ova, posebnih stekova po procesoru i raspoređivanja na više procesora irelevantna, a implementacija znatno jednostavnija.

#### Preotimanje

Odluka da li sistem omogućava **preotimanje** (engl. *preemption*) tokom izvršavanja korisničkog kôda (tj. asinhronu promenu konteksta) ostavljena je studentu — može se opredeliti da ovaj deo ne radi (videti [Način ocenjivanja](#način-ocenjivanja)). Sistem sa preotimanjem ima brži odziv, ali je složeniji jer asinhroni prekidi stižu u nepredvidivim trenucima.

Dodatno pitanje: da li se preotimanje (asinhrona promena konteksta) može dešavati i **tokom izvršavanja kôda jezgra**. Takva jezgra (engl. *preemptive kernel*) imaju još bolji odziv, ali su još složenija. **Nije neophodno** praviti takvo jezgro, ali ambiciozniji studenti mogu.

#### Međusobno isključenje

Ako jezgro **ne** omogućava preotimanje tokom svog izvršavanja, ceo kôd jezgra je jedna kritična sekcija — obezbeđuje se maskiranjem prekida na ulasku (što procesor i sam radi). Alternativno, prekidi se mogu dozvoliti tokom kôda jezgra, ali se u prekidnoj rutini samo evidentiraju i obrade sekvencijalno kasnije. Spin lock nije potreban (jednoprocesorski sistem).

Ako jezgro **omogućava** preotimanje tokom svog izvršavanja, treba:
- identifikovati deljene strukture podataka i operacije nad njima; svakoj pridružiti sinhronizacionu primitivu, operaciju posmatrati kao kritičnu sekciju;
- bazične kritične sekcije (promena konteksta, semafori) štititi **maskiranjem prekida**;
- ostale kritične sekcije (ako se izvršavaju u kontekstu korisničkih niti) mogu se štititi semaforima;
- paziti na **mrtvu blokadu** (npr. uvek isti redosled zaključavanja);
- kod ugnežđenih sekcija sa maskiranjem prekida demaskirati tek pri izlasku iz najspoljašnje (npr. brojanjem dubine ugnežđivanja).

#### Stek na kom se izvršava kôd jezgra

U najjednostavnijoj varijanti kôd jezgra se izvršava na steku tekuće niti, ali je zbog rizika korupcije memorije sigurnije preusmeriti ga na zaseban stek sa dovoljno prostora. Varijante:
- **ceo kôd jezgra na jednom jedinom steku** — nije moguće ako jezgro dozvoljava preotimanje tokom svog izvršavanja (a kod multiprocesora svaki procesor mora imati svoj);
- **svakoj korisničkoj niti pridružen poseban dodatni (sistemski) stek** kao logički nastavak korisničkog; svaki deo kôda jezgra izvršava se u kontekstu neke niti, sve do trenutka promene konteksta.

#### Memorijski kontekst

Sav kôd (i korisničkih niti i jezgra) izvršava se u istom, jedinstvenom memorijskom kontekstu, pa o ovome ne treba voditi računa.

#### Zaključak

Osim postupka promene konteksta i obrade sistemskih poziva/prekida/izuzetaka, kôd jezgra je standardan objektno orijentisan C++ softver. **Ne** koristiti bibliotečne funkcije — uključujući zabranu poziva funkcija za alokaciju memorije i operatora `new` za prављење dinamičkih objekata (jer podrazumevano poziva bibliotečnu alokaciju). Bez preotimanja unutar jezgra nije potrebna dodatna sinhronizacija; sa preotimanjem treba obezbediti sinhronizaciju kako je opisano.

### Ključne apstrakcije

Predlog ključnih C++ klasa:[^17]

- **`MemoryAllocator`**: singleton[^18] klasa za alokaciju i dealokaciju memorije kontinualnom alokacijom.
- **`Thread`** (ili `PCB`): apstrahuje nit, čuva njen kontekst i druge atribute.
- **`Scheduler`**: singleton klasa — raspoređivač, tj. algoritam raspoређivanja.
- **`Semaphore`**: apstrahuje semafor i operacije nad njim.
- **`Console`**: singleton klasa — sprega ka konzoli.

**Alokacija objekata za dinamičko instanciranje** (npr. `Thread`, `Semaphore`) ne sme se rešavati ugrađenim `new` (oslanja se na `mem_alloc`). Predlaže se uvođenje statičkih operacija klasa za pravljenje/uništavanje, ili preklapanje `new`/`delete` za te klase. Opcije za alokaciju prostora:
- **Statička alokacija** — unapred alociran niz pregradaka; vodi se evidencija zauzetih/slobodnih. Uvodi logička ograničenja kapaciteta (ograničen broj niti/semafora).
- **Dinamička alokacija** — dinamička alokacija nizova alokatorom jezgra po potrebi; fleksibilnije ali složenije.

Funkcionalnost alokacije pregradaka dobro je izdvojiti u posebnu, **šablonsku** klasu.

### Preotimanje, promena konteksta i raspoređivanje

Procesor ima **jedinstvenu rutinu** za obradu sistemskog poziva, spoljašnjeg prekida i izuzetka (`stvec`), a registri daju uzrok. Dovoljno je napraviti **jednu prekidnu rutinu**. Ona je jedino mesto prelaska iz korisničkih niti u jezgro i nazad (i između korisničkog i sistemskog režima).

**Varijanta: kôd jezgra na steku pojedinačne niti.** Pozivi funkcija koriste stek tekuće niti; izvršavanje prekidne rutine i ugnežđenih poziva je obično ugnežđivanje poziva. Bez asinhronih promena dovoljno je sačuvati samo registre koje rutina menja. Kod asinhronih prekida postoji problem: korisnički kôd može koristiti bilo koji programski dostupan registar (npr. `t` registre); kako se asinhroni prekid može dogoditi dok su te vrednosti „žive“, treba ih sačuvati. Prekidna rutina tada treba da:

1. Pređe na sistemski deo steka tekuće niti i sačuva sve (ili samo neke) korisničke registre; ako je podržana asinhrona promena konteksta — **sve**.
2. Obradi uzrok skoka (sistemski poziv, izuzetak ili prekid). Tu može doći do promene tekuće niti i konteksta (steka).
3. Po potrebi restaurira sačuvane registre i vrati se.

Promena konteksta lokalizuje se u poseban potprogram poput `yield`:

```cpp
void yield (Thread* oldThread, Thread* newThread);
```

`yield` čuva kontekst tekuće niti (na njen stek ili u PCB), prebacuje se na stek odredišne niti i povraća njen kontekst.

**Varijanta: kôd jezgra na jednom zajedničkom steku.** Izvršavanje jezgra je kao poseban tok kontrole (nit jezgra). Prelazak u jezgro i nazad može se posmatrati kao promena konteksta sa korisničke niti na nit jezgra (i nazad). Rutina treba da:

1. Pređe na sistemski stek i sačuva ceo kontekst (sve korisničke registre) u mesto za čuvanje konteksta niti.
2. Obradi uzrok skoka. Može doći do promene tekuće niti (ali ne i steka).
3. Vrati kontekst tekuće niti, vrati korisnički stek i vrati se.

Koraci 1 i 3 mogu se implementirati potprogramima sličnim `yield` (korutine[^20]); kontekst se **mora** čuvati u PCB.

Sadržaj koraka 2 najbolje je izdvojiti u poseban C/C++ potprogram sa razgranatim skokom na obradu uzroka (uključujući obradu sistemskog poziva) — `switch` ili niz pokazivača na funkcije indeksiran kôdom sistemskog poziva. Pošto se parametri prenose registrima (a ABI parametri su već u registrima), poziv C funkcije iz prekidne rutine je pojednostavljen. Čuvanje/restauracija konteksta (`yield`, odn. koraci 1 i 3) mogu se pisati na asembleru.

**Niti jezgra** mogu se tretirati isto kao ostale, osim što im se telo potencijalno izvršava u privilegovanom režimu. Razlikuju se dve vrste niti (korisnička/sistemska); pri restauraciji konteksta postavlja se bit **SPP** na odgovarajuću vrednost.

**Stanja niti i skup spremnih niti:** npr. ulančana lista spremnih niti u nadležnosti klase `Scheduler`. Pokazivače za ulančavanje bolje je organizovati **unutar same strukture niti** (umesto dinamičke alokacije malih čvorova, zbog režije i fragmentacije), ali paziti da se isti pokazivači ne koriste istovremeno za ulančavanje u različite strukture i da se ne naruši enkapsulacija.[^21]

**Algoritam raspoređivanja:** dovoljan je najjednostavniji **FIFO (FCFS)**; ambiciozniji mogu napredniji.[^22] Za situaciju kada nema spremnih korisničkih niti, najjednostavnije je obezbediti **idle** nit koja se vrti u praznoj petlji i dobija procesor samo ako nema drugih spremnih niti (ili nit jezgra koja radi režijske poslove).

### Implementacija nekih zahtevanih funkcionalnosti

**Alokacija/dealokacija (`MemoryAllocator`)** — nekim algoritmom kontinualne alokacije (*first fit* ili *best fit*).

**Kreiranje niti** — najveći izazov je formiranje početnog konteksta. Telo korisničke niti „umota“ se u funkciju-omotač (engl. *wrapper*), a početni kontekst niti postavi tako da počne izvršavanje od omotača.[^23] Posebno paziti da se kontekst postavi tačno tako da se pri restauraciji izvršavanje prebaci na telo omotača. Omotač dohvata pokazivač na funkciju niti (nekoliko jednostavnih rešenja). Iz omotača se nikada ne sme vratiti — nakon poziva tela niti treba da sadrži sistemski poziv za **gašenje niti** (ako se nit nije već ugasila). Telo omotača i tela niti izvršavaju se u korisničkom režimu.

**`thread_exit`, `thread_dispatch`** — bez posebnih teškoća. Slično važi za semafore.

**`time_sleep` (uspavljivanje/buđenje)** — jedno jednostavno rešenje: uspavane niti u ulančanoj listi uređenoj po vremenu buđenja.[^24] Za svaki element čuva se **relativno** vreme buđenja (u periodama tajmera) u odnosu na prethodni element (može biti 0); za prvi element to je interval do buđenja od sadašnjeg trenutka. Na svaku periodu tajmera dekrementira se samo vrednost prvog elementa; kada dođe do nule, u red spremnih se vraćaju svi elementi sa početka liste čija je vrednost 0.

### Ulaz/izlaz

Prenos podataka ka/iz kontrolera konzole zahteva **prozivanje** (engl. *polling*, ispitivanje bita spremnosti), uz prekid na novi znak/spremnost. Prenos ne treba raditi uposlenim čekanjem u kontekstu sistemskog poziva. Zato se sistemski poziv razdvaja od prenosa **baferisanjem** (ograničeni/neograničeni baferi za ulaz i izlaz):

- **`putc`** — znak se smešta u izlazni bafer (korisnička nit = proizvođač). Ako je bafer pun: blokirati nit ili vratiti grešku. Posebna **nit jezgra** (potrošač) uzima znakove i prenosi ih kontroleru uz prozivanje; ako je bafer prazan — blokira se.
- **getc / prekid tastature** — u obradi prekida ulazni znakovi se čitaju sa kontrolera dok ih ima (prozivanjem) i smeštaju u ulazni bafer (prekidna rutina = proizvođač). Broj učitanih znakova se može ograničiti. Ako je bafer pun — znakovi se odbacuju (hardverski proizvođač se ne može blokirati). **`getc`** uzima znak iz ulaznog bafera (korisnička nit = potrošač); ako je prazan — blokira nit.

Interne niti jezgra pokreću se pri inicijalizaciji; njihovo telo se izvršava u **sistemskom režimu** (radi pristupa hardverskim registrima).

**Sinhronizacija internih i korisničkih niti:**
- Varijanta sa stekom po niti — mogu se koristiti semafori jezgra.
- Varijanta sa zajedničkim sistemskim stekom — zahtev korisničke niti (npr. `putc`) se ili **mora** prihvatiti/upisati u bafer ili odbiti greškom; ne može se suspendovati pozivajuća nit ako upis nije moguć (njen kontekst je već napušten). Dakle: ispitati ima li mesta i ako ima — smestiti bez posebne sinhronizacije,[^27] inače vratiti grešku. Druga opcija: uslovna sinhronizacija semaforima **unutar tela C API funkcije `putc`**, dok ABI `putc` samo upisuje u bafer; ovo je jednostavnije ali manje robusno (ne sprečava direktan ABI poziv bez sinhronizacije).

### Asinhroni prekid i tajmer

Spoljašnji prekidi dolaze iz dva izvora: **konzole** i **tajmera**.

Na prekid od **tajmera** principijelno uraditi:
- **Ažurirati preostalo vreme** tekuće niti (umanjiti za jednu periodu); ako je vreme isteklo — promena tekuće niti i konteksta. Pri izboru nove tekuće niti dodeliti joj vremenski odsečak (kvantum). Preostalo vreme najlakše čuvati u jednoj statičkoj promenljivoj iza odgovarajućeg interfejsa, sa pažljivo dodeljenom odgovornošću klasi.
- **Ažurirati evidenciju uspavanih niti** i po potrebi „probuditi“ odgovarajuće niti.

### Implementacija interfejsnih slojeva

- **ABI sloj** — implementacija prekidne rutine prema datim uputstvima.
- **C API sloj** — jednostavan: funkcija pripremi parametre u argumente i izvrši instrukciju sistemskog poziva. Pošto je to zajedničko za sve pozive, dobro je izdvojiti opštu funkciju koja prima opšte argumente i izvršava `ecall`; C API funkcije se svode na poziv te opšte funkcije. Izuzetak su funkcije koje malo prerade argumente ili imaju više sistemskih poziva.
- **C++ API sloj** — adaptira neobjektni C API u objektni. Klase nose (kao atribute) ručke objekata napravljenih u jezgru, a funkcije članice preusmeravaju na C API uz prosleđivanje ručke. Slično za konstruktore i destruktore.

### Predlog redosleda izrade

Predlog jednog razumnog (ne nužno najboljeg) redosleda, iterativno i inkrementalno[^28], uz detaljno testiranje:

1. Alokator memorije (klasa `MemoryAllocator`).
2. Prekidna rutina: samo prenos argumenata i prelazak u odgovarajući režim, bez promene konteksta i bez ulaska dalje u jezgro — poziva se iz korisničkog programa (bez niti) preko `ecall` i odmah se vraća.
3. Razgranati skok na pojedinačni sistemski poziv; C funkcija zajedničkog dela; sistemski pozivi `mem_alloc` i `mem_free` (poziv iz rutine, ABI i C API). Testiranje iz „običnog“ C programa.
4. Kostur jezgra: kostur klase `Thread` i klasa `Scheduler` u potpunosti.
5. Sistemski stek (zajednički ili po niti). Kompletiranje prekidne rutine čuvanjem/restauracijom registara. Promena konteksta (u `yield` ili u rutini).[^29]
6. Formiranje početnog konteksta niti. Sistemski pozivi `dispatch` i `thread_create` (ABI i C API). Testiranje nitima koje se nikada ne završavaju (program gasiti nasilno).
7. Gašenje niti. `thread_exit` (C API i ABI).
8. Semafori: klasa `Semaphore` u potpunosti, svi sistemski pozivi za semafore (ABI i C API).
9. Asinhroni prekid od tajmera, raspodela vremena (promena konteksta na istek odsečka — *time sharing*).
10. Uspavljivanje i buđenje niti, `time_sleep` (C API i ABI).
11. Konzola, izlazni smer: bafer, interna nit jezgra, `putc` (C API i ABI).
12. Konzola, ulazni smer: bafer, prekidna rutina, `getc` (C API i ABI).
13. C++ API u celini.

---

## Način ocenjivanja

### Način predaje projekta

Projekat se predaje **isključivo kao jedna zip arhiva**, sa dva direktorijuma:
- **`src`** — svi `.cpp` i `.S` fajlovi;
- **`inc`** — svi `.h`/`.hpp` fajlovi.

To je ujedno i jedini dozvoljeni sadržaj arhive. Arhiva **ne sme** sadržati izvršne fajlove, biblioteke, test fajlove, git repozitorijume, niti bilo šta što nije tekstualni izvorni kôd (C, C++, asembler). Projekat se može predati više puta do roka (prvi radni dan pre ispita); čuva se samo poslednja verzija. Predaja projekta (uz prijavu ispita i položene kolokvijume) je preduslov za izlazak na ispit. Nakon roka, predati zadaci se brišu.

Sajt za predaju: <http://rti.etf.bg.ac.rs/domaci/index.php?servis=os1_projekat>

Projekat predat posle roka se ne razmatra. Nepoštovanje pravila predaje povlači negativne poene.

### Način ocenjivanja projekta

Projekat se može uraditi u celini ili delimično, podeljen na delove:

| Broj | Naziv | Sadržaj zadatka | Poeni |
|------|-------|-----------------|:-----:|
| 1 | Alokacija memorije | Sistemski pozivi `mem_alloc` i `mem_free`. | 5 |
| 2 | Niti | Niti i `thread_create`, `thread_exit`, `thread_dispatch`. Samo **sinhrona** promena konteksta prilikom bilo kog sistemskog poziva. | 10 |
| 3 | Semafori | Semafori i `sem_open`, `sem_close`, `sem_wait`, `sem_signal`. | 5 |
| 4 | Asinhrona promena konteksta | Deljenje vremena i asinhrona promena konteksta na prekid od tajmera i tastature. `time_sleep`, `getc`, `putc`, klasa `PeriodicThread`. | 10 |
| 5 | Bonus | Projekat odbranjen u predroku. | 10 |

Svaki zadatak obuhvata implementaciju **sva tri sloja interfejsa** za obuhvaćene sistemske pozive.

- Ako student **ne** radi zadatak 1, uvezuje gotov `mem.lib` (sadrži alokator, ali **ne** implementira slojeve interfejsa — to ostaje studentu; nema sinhronizaciju, nije thread-safe).
- Ako student **ne** radi zadatak 4, na raspolaganju je gotova prekidna rutina za prekide od tajmera i konzole; uvezuje se `console.lib` (sadrži `getc`/`putc`, ali **ne** slojeve interfejsa; nije thread-safe; može dozvoliti prekide tokom izvršavanja). Da bi gotove funkcije radile, u prekidnoj rutini treba pozvati `console_handler`.
- Zadaci **3, 4 i 5 prirodno zahtevaju urađen zadatak 2.**

**Uslovi za uspešnu odbranu:**
- proći javne testove za delove koji nose ukupno **najmanje 20 poena**;
- na odbrani (tajni testovi, modifikacije, usmeni) dobiti **najmanje 15 poena** ukupno za sve urađene delove (delovi ne moraju dobiti maksimum).

### Način provere projekta

Provera se sprovodi **javnim** i **tajnim** testovima, kao i opcionim **testovima performansi** (samo za bonus poene).

- **Javni testovi** — dostupni unapred, otvorenog kôda.
- **Tajni testovi** — nisu dostupni tokom izrade; proveravaju: propisnu alokaciju/dealokaciju memorije; propisno pokretanje/gašenje niti; ispravno konkurentno izvršavanje uz sinhronu promenu konteksta i sinhronizacione primitive; ispravno konkurentno izvršavanje uz asinhronu promenu konteksta (deljenje vremena i prekidi).
- **Testovi performansi** (tajni) mogu ispitivati: režijsko vreme promene konteksta; režijsko vreme operacija sa semaforom; vreme odziva na prekid; broj konkurentnih niti do preopterećenja; broj niti sa sve manjim vremenskim intervalom; zauzeće memorije za strukture niti/semafora/događaja; druge vremenske i prostorne parametre.

**Nezadovoljenje bilo kog javnog testa povlači odbijanje čitavog zadatka ili celog projekta sa 0 poena.** Nezadovoljenje tajnog/performansnog testa nosi negativne poene za taj zadatak.

Od studenta se na odbrani može tražiti da u zadatom vremenu samostalno uradi jednu ili više **manjih modifikacija** (proveravaju se javnim i tajnim testovima); neurađena modifikacija povlači negativne poene ili odbijanje.

Na odbrani se pregleda **kvalitet izvornog kôda**: stil i urednost, stilska ujednačenost, poštovanje principa proceduralnog i objektnog programiranja, dobra arhitektura, raspodela odgovornosti, dekompozicija, jasni interfejsi i enkapsulacija, jasni i kratki potprogrami. Uočeni nedostaci povlače negativne poene ili odbijanje.

Ispitivač može proveravati **samostalnost** i poznavanje detalja projekta i gradiva usmenim pitanjima; neznanje, nesigurnost ili nepreciznost mogu nositi negativne poene ili odbijanje celog projekta.

---

## Napomene (footnotes)

[^1]: Sve ove registre prevodilac potencijalno koristi, pa na to treba obratiti pažnju pri čuvanju konteksta procesora, posebno kod asinhrone promene konteksta. Svi registri su zapravo potpuno ravnopravni registri opšte namene (`x0..x31`); navedeni mnemonici su sinonimi koje asembler i prevodilac tretiraju po konvenciji za C/C++.

[^2]: Ovaj procesor zapravo uopšte ne poznaje koncept steka, niti ga koristi (nema `push`/`pop`, ne koristi ga ni pri obradi prekida/izuzetaka/sistemskih poziva). Organizacija steka je u isključivoj nadležnosti prevodioca ili programera na asembleru.

[^5]: Frame pointer je pokazivač na oblast steka od koje počinje aktivacioni blok sa automatskim podacima tekućeg potprograma. Koristi se da bi se automatski podaci adresirali relativno u odnosu na nepromenljiv pokazivač (konstantni pomeraji), umesto u odnosu na `sp` koji se može menjati.

[^6]: Procesor poseduje dva načina izazivanja softverskog prekida: `ecall` (za sistemske pozive iz korisničkog/sistemskog režima) i drugi mehanizam (za treći, najprivilegovaniji režim — nije vidljiv studentima). Pristup hardveru tajmera moguć je samo iz tog trećeg režima; prekid od tajmera prosleđuje se jezgru kao taj drugi tip softverskog prekida, koji se obrađuje principijelno isto kao `ecall`.

[^7]: To znači da ovaj procesor ima jednu jedinstvenu prekidnu rutinu za sve vrste prekida.

[^8]: Posledica prilagođenja okruženja radi jednostavnijeg korišćenja. Prekid od tajmera prepoznaje se po datom razlogu — potiče samo od tajmera.

[^9]: Serijska veza označava komunikacionu liniju preko koje se podaci prenose redom bit po bit.

[^10]: Komanda `make` se detaljno izučava na predmetu Praktikum iz operativnih sistema.

[^12]: Udaljeno debagovanje omogućava da se debagovani program izvršava u jednom procesu, a debager u drugom; komuniciraju preko softverskih priključnica (engl. *socket*).

[^17]: Da bi se identifikatori jezgra razlikovali od onih u API-u, može se koristiti poseban `namespace` ili prefiks (npr. `k`, kao `kmalloc`).

[^18]: Singleton je projektni obrazac koji obezbeđuje da klasa ima samo jedan objekat, dostupan na unapred definisan način. Alternativa je uslužna klasa (samo statičke članice).

[^20]: Tokovi kontrole koji eksplicitno jedan drugom prebacuju kontrolu (poput `yield`) nazivaju se korutine (engl. *coroutine*).

[^21]: O ovakvoj implementaciji na C/C++ detaljna diskusija data je u materijalima za predmet „Objektno orijentisano programiranje" (<http://oop.etf.rs>).

[^22]: O algoritmima raspoređivanja procesa detaljno se govori u predmetu „Operativni sistemi 2".

[^23]: Jednostavnije je da funkcija-omotač nije nestatička funkcija članica klase (inače bi joj morao da se prosledi skriveni `this`).

[^24]: Uzeti u obzir raniji komentar o implementaciji dinamički ulančanih lista.

[^27]: Pretpostavlja se da je međusobno isključenje obezbeđeno na nivou celog kôda jezgra (maskiranje prekida na ulasku u prekidnu rutinu).

[^28]: „Iterativno i inkrementalno" — softver se razvija u iteracijama; u svakoj se prolazi kroz projektovanje, implementaciju i testiranje manjeg podskupa funkcionalnosti, a sistem se nadograđuje u narednim iteracijama. Cilj je rano doći do minimalnog, ali funkcionalnog i izvršivog kostura.

[^29]: Rezultat ovog koraka nije sasvim funkcionalna verzija; testira se parcijalno, dok se ne završi naredni korak.
