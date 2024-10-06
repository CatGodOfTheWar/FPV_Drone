// Librariile necesare
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <assert.h>
#include <stdint.h>
#include <sched.h>
#include <errno.h>
#include <string.h>
#include <time.h>

//////////////////////////////////////////////////////////////////////
// Versiunea de DSHOT pe care doresti sa o folosesti : 150 ; 300 ; 600 ; 1200
#ifndef DSHOT_VERSION
#define DSHOT_VERSION 150
#endif

//////////////////////////////////////////////////////////////////////
// DSHOT_BIT_ns specifica lungimea la 1 bit pentru un numar de nanosecunde
// (Fiecare protocol Dshot are o valoare teoretica a valori , dar in pratica se poate ajusta la device-ul tau)

#if DSHOT_VERSION == 150
// valoarea teoretica a unui bit in ns pentru Dshot 150
//  #define DSHOT_BIT_ns 6670
#define DSHOT_BIT_ns 5300
#elif DSHOT_VERSION == 300
// valoarea teoretica a unui bit in ns pentru Dshot 300
// #define DSHOT_BIT_ns 3330
#define DSHOT_BIT_ns 2700
#elif DSHOT_VERSION == 600
// valoarea teoretica a unui bit in ns pentru Dshot 600
#define DSHOT_BIT_ns 1670
#elif DSHOT_VERSION == 1200
// valoarea teoretica a unui bit in ns pentru Dshot 1200
#define DSHOT_BIT_ns 830
#endif

//////////////////////////////////////////////////////////////////////
// T0H este durata unui bit care are valoarea 0
// Valoarea teoretica a protocolului
// #define DSHOT_T0H_ns (DSHOT_BIT_ns * 3 / 8)
#define DSHOT_T0H_ns (DSHOT_BIT_ns * 3 / 9)

//////////////////////////////////////////////////////////////////////
// CLOCK_MONOTONIC_RAW este un ceas cu cea mai mare precizie pe care il folosim pentru timing cand vom transmite catre ESC
// DSHOT_USE_CLOCK este un alias la CLOCK_MONOTONIC_RAW
#ifndef DSHOT_USE_CLOCK
#define DSHOT_USE_CLOCK CLOCK_MONOTONIC_RAW
#endif

//////////////////////////////////////////////////////////////////////
// Numarul maxim de erori in nanosecunde
#define DSHOT_MAX_TIMING_ERROR_ns 2000
// Numarul maxim de reincercari de transmitere a unui frame daca cel precedent a esuat
#define DSHOT_MAX_RETRY 10
// Timpul de asteptare inainte de a transmite din nou un frame daca a dat fail cel precedent
#define USLEEP_BEFORE_REBROADCAST 100

//////////////////////////////////////////////////////////////////////
// tt este o structura timespec care contine timpul la acel moment in secunde si nanosecunde
// TIMESPEC_TO_INT(tt) va returna un integer cu valoarea totala de nanosecunde din structura 'tt'
// tt.tv_sec numarul total de secunde din structura 'tt'
// tt.tv_nsec numarul total de nanosecunde din structura 'tt'
// 1000000000LL este o constanta care reprezinta numarul de nano secunde intr-o secunda
#define TIMESPEC_TO_INT(tt) (tt.tv_sec * 1000000000LL + tt.tv_nsec)

//////////////////////////////////////////////////////////////////////
// Offsetul standard pentru adresa GPIO-ului de la raspberry pi
// Offsetul este adaugat la adresa de baza de la memoria perifericelor pentru a putea acesa registri de la GPIO
// 0x00200000 este Offestul standard pentru raspberry pi
#define GPIO_BASE_OFFSET 0x00200000
// Memoria unui computer este structurata in paginii , fiecare pagina are o dimensiune in functie de configurarea sistemului
// 4 * 1024 sunt 4096 bytes => 4 Kilobytes este foarte comuna in multe sisteme si raspberry pi
#define PAGE_SIZE (4 * 1024)
// Reprezinta memoria unui block , similar cu o pagina ,dar este folosit pentru alocarea sau dealocarea memoriei
// In cazul de Raspberry pi este folosit pentru maparea operatilor sau accesarea unei regiuni de memorie care are legatura cu functionalitatea GPIO-ului
#define BLOCK_SIZE (4 * 1024)

//////////////////////////////////////////////////////////////////////
// Setam pinul 'g' ca si input
// Calculeaza directia pinului(input / output)
// (g) / 10
// Pinii de GPIO sunt grupati in banci(banks) , de obicei o banca controleaza mai multi pini(De obicei o banca controleaza 10 GPIO pins)
// Impartirea la 10 neajuta sa calculam din care banca face parte pinul nostru
// (g) % 10
// Impartirea cu rest la 10 este folosita pentru a calcula pozitia pinului in registrul de control
// ((g) % 10) * 3) deoarece fiecare pin in registru are asociat 3 biti vom inmultimi cu 3
// 7 << (((g) % 10) * 3) faci left shift pana cand pini de la pinul nostru 'g' are biti 111
// ~(7 << (((g) % 10) * 3)) este operaotrul NOT care transfroma biti 1 in 0 si viceversa
// /*(dshotGpio + ((g) / 10)) adresa pinului nostru pe care dorim sa il modificam
// &= dupa ce am identificat unde sa afla pinul nostru in banca vom modifica acesta cu bitmask-ul nostru (~(7 << (((g) % 10) * 3))) astfel vom da overwrite la pinul nostru
#define INP_GPIO(g) (*(dshotGpio + ((g) / 10)) &= ~(7 << (((g) % 10) * 3)))
// Setam pinul 'g' ca si output
// Este la la ca si la input doar ca aici vom da overwrite la pinul nostru sa devina de output
#define OUT_GPIO(g) (*(dshotGpio + ((g) / 10)) |= (1 << (((g) % 10) * 3)))

//////////////////////////////////////////////////////////////////////
// Este o scurtatura care ne ajuta sa ajustam un pin specificat de noi pentru a fi ON
#define GPIO_SET (*(dshotGpio + 7))
// Este o scurtatura care ne ajuta sa stergem datele salvate in registrul de control
#define GPIO_CLR (*(dshotGpio + 10))
// Numarul maxim de pini folositi de implementarea protocolului nostru de dshot
#define DSHOT_NUM_PINS 27

//////////////////////////////////////////////////////////////////////
// enum tipul de data specificat de noi care va contine toate comenzile Dshot
enum
{
    // Daca dorim sa oprim motoarele instant vom folosi aceasta comanda
    DSHOT_CMD_MOTOR_STOP = 0,
    // Comnenziile aceastea sunt folosite pentru a trimite semnale periodice catre ESC, ele sunt folosite pentru sincronizare sau pentru a specifica statusul specific al unui semnal
    DSHOT_CMD_BEACON1,
    DSHOT_CMD_BEACON2,
    DSHOT_CMD_BEACON3,
    DSHOT_CMD_BEACON4,
    DSHOT_CMD_BEACON5,
    // Comanda aceasta este folosita pentru a primi informatii de la ESC
    DSHOT_CMD_ESC_INFO, // V2 includes settings
                        // Aceste comenzii sunt folosite pentru a controla directia de rotatie a motoarelor
    DSHOT_CMD_SPIN_DIRECTION_1,
    DSHOT_CMD_SPIN_DIRECTION_2,
    // Aceste comenzii ne ajuta sa ajutam cum se vor comporta motoarele , On(inversam sensul motoarelor) , Off(se vor roti intr-un singur sens)
    DSHOT_CMD_3D_MODE_OFF,
    DSHOT_CMD_3D_MODE_ON,
    // Aceasta comanada este folosita pentru a face request de informatii de la ESC
    DSHOT_CMD_SETTINGS_REQUEST, // Currently not implemented
                                // Aceasta comanda este folosita pentru a instrui ESC sa salveze setariile noastre
    DSHOT_CMD_SAVE_SETTINGS,
    // Aceste comenzii sunt folosite pentru a specifica sensul de rotatie
    DSHOT_CMD_SPIN_DIRECTION_NORMAL = 20,
    DSHOT_CMD_SPIN_DIRECTION_REVERSED = 21,
    // Aceste comenzii ne lasa sa ajutam LED-urile conectate la ESC, dar sunt specifice BLHeli32 doar
    DSHOT_CMD_LED0_ON,                       // BLHeli32 only
    DSHOT_CMD_LED1_ON,                       // BLHeli32 only
    DSHOT_CMD_LED2_ON,                       // BLHeli32 only
    DSHOT_CMD_LED3_ON,                       // BLHeli32 only
    DSHOT_CMD_LED0_OFF,                      // BLHeli32 only
    DSHOT_CMD_LED1_OFF,                      // BLHeli32 only
    DSHOT_CMD_LED2_OFF,                      // BLHeli32 only
    DSHOT_CMD_LED3_OFF,                      // BLHeli32 only
                                             // Aceste comenzi ne lasa sa manipulam sunetul produs de motoarele noastre
    DSHOT_CMD_AUDIO_STREAM_MODE_ON_OFF = 30, // KISS audio Stream mode on/Off
    DSHOT_CMD_SILENT_MODE_ON_OFF = 31,       // KISS silent Mode on/Off
                                             // Aceasta comanda anuleaza telemetry de pe linia de transmisie
    DSHOT_CMD_SIGNAL_LINE_TELEMETRY_DISABLE = 32,
    // Aceasta comanda este folosita pentru a primi in timp real RPM-ul motoarelor
    DSHOT_CMD_SIGNAL_LINE_CONTINUOUS_ERPM_TELEMETRY = 33,
    // Aceasta comnanda reprezinta valoarea maxima a comenzii definite in enum
    DSHOT_CMD_MAX = 47
};

//////////////////////////////////////////////////////////////////////
// I/O access
// static isi vor mentinte valoarea pe parcursul executiei programului si nu pot fi accesate din alt fisier
// void deoarece este un pointer generic fara un tip anume
// Este un pointer pentru a mapa regiunea de memorie care ne va da direct acces catre registri de la GPIO
static void *dshotGpioMap;
// unb pointer care indica catre un unsigned integer de 32 biti care este folosit pentru a accesa registri de la GPIO care sunt mapati in memorie
//  volatile -> ne asigura ca compiler-ul nu va optimiza accesul la memorie prin acest pointer , deoarece se poate schimba in timpul operatiilor
static volatile uint32_t *dshotGpio;
// tine track-ul daca 3D mode este enable/disable
// 3D mode, if dshot3dMode != 0 then reverse rotation is enabled
static int dshot3dMode = 0;

//////////////////////////////////////////////////////////////////////
// static are acelasi scop ca si mai sus
// inline unde este apelata functia , in loc sa fie apelata functia , codul din interiorul ei va fi direct inserat acolo in locul ei pentru performante mai bune
// uint64_t specifica ca functia va returna un unsigned int de 64 biti
static inline uint64_t dshotGetNanoseconds()
{
    // declaram variabila tt de tip struct timespec
    struct timespec tt;
    // vom folosi functia clock_gettime pentru a stoca valoarea la adresa variabilei tt prin folosirea referintei &tt
    clock_gettime(DSHOT_USE_CLOCK, &tt);
    // va returna timpul curent in nanosecunde
    return (TIMESPEC_TO_INT(tt));
}

//////////////////////////////////////////////////////////////////////
// checksum este calculat in functie de packetul nostru pe care dorim sa il transmitem
// Este transmis odata cu packetul si la destinatie este recalculat checksum si se compara cu checksum-ul transmis de noi
// Daca acestea sunt identice inseamna ca nu am intampinat erori de transmisie
// Functia noastra calculeaza checksum pentru packet-ul nostru si combina checksum si flag-ul de telemetry intr-o singura valoare
// Functia va returna o valoare de 4 biti care este conforma protocolului DSHOT
static int dshotAddChecksumAndTelemetry(int packet, int telem)
{
    // comandata aceasta combina packetul nostru cu bit-ul de telemetry
    // packet << 1 face shit la stanga de un bit in packetul nostru pentru a avea loc pentru bit-ul de telemetry
    // telem & 1 o folosim pentru a ne asigura ca telem ocupa un singur bit
    int packet_telemetry = (packet << 1) | (telem & 1);
    int i;
    int csum = 0;
    int csum_data = packet_telemetry;
    for (i = 0; i < 3; i++)
    {
        csum ^= csum_data;
        csum_data >>= 4;
    }
    // ne asiguram ca checksum are doar 4 biti
    csum &= 0xf;
    // facem shift la stanga de 4 biti si adaugam
    return ((packet_telemetry << 4) | csum);
}

//////////////////////////////////////////////////////////////////////
// Functia aceasta o folosim pentru a vedea daca timing-ul nostru intre frame-uri este corect
static void dshotSend(uint32_t allMotorsPinMask, uint32_t *clearMasks)
{
    int i, j;
    int64_t t, tt, t0, offset1, offset2, offset3;
    volatile unsigned *gpioset;
    volatile unsigned *gpioclear;

    // prepare addresses
    gpioset = &GPIO_SET;
    gpioclear = &GPIO_CLR;

    offset1 = DSHOT_T0H_ns;
    offset2 = DSHOT_T0H_ns;
    offset3 = DSHOT_BIT_ns - offset1 - offset2;

    // We will try to send the frame several times if timing was wrong
    for (j = 0; j < DSHOT_MAX_RETRY; j++)
    {
        // send dshot frame bits
        tt = t0 = dshotGetNanoseconds();
        for (i = 0; i < 16; i++)
        {
            tt += offset3;
            while ((t = dshotGetNanoseconds()) < tt)
                ;
            *gpioset = allMotorsPinMask;
            // if we are not in time, abandon the whole dshot frame
            if (t - tt > DSHOT_MAX_TIMING_ERROR_ns)
                break;
            tt += offset1;
            while ((t = dshotGetNanoseconds()) < tt)
                ;
            *gpioclear = clearMasks[i];
            if (t - tt > DSHOT_MAX_TIMING_ERROR_ns)
                break;
            tt += offset2;
            while ((t = dshotGetNanoseconds()) < tt)
                ;
            *gpioclear = allMotorsPinMask;
            if (t - tt > DSHOT_MAX_TIMING_ERROR_ns)
                break;
        }
        if (t - tt > DSHOT_MAX_TIMING_ERROR_ns)
        {
            // we were out of timing, the frame was abandonned and we will retry to broadcast it
            *gpioclear = allMotorsPinMask;
            // printf("debug Dshot Frame was abandonned because of wrong timing in attempt %d, bit %d.\n", j, i); fflush(stdout);
            // relax to OS for a small period of time, hope it reduces the probability that we will
            // be interrupted during next broadcasting.
            usleep(USLEEP_BEFORE_REBROADCAST);
        }
        else
        {
            // ok we are done here.
            // printf("debug Dshot Frame successfully sent.\n"); fflush(stdout);
            return;
        }
    }
    printf("debug Dshot Frame failure.\n");
    fflush(stdout);
}

//////////////////////////////////////////////////////////////////////
// Aceasta functie calculeaza o masca are va avea valoarea 1 pentru pini folsiti la motoare si valoarea 0 in rest
static uint32_t dshotGetAllMotorsPinMask(int motorPins[], int motorMax)
{
    int i;
    uint32_t allMotorsPinsMask;

    // compute masks
    allMotorsPinsMask = 0;
    for (i = 0; i < motorMax; i++)
        allMotorsPinsMask |= (1 << motorPins[i]);
    return (allMotorsPinsMask);
}

//////////////////////////////////////////////////////////////////////
// Aceasta functie se asigura ca avem fiecare bit on sau off la motoare cand vom trimite un frame
void dshotSendFrames(int motorPins[], int motorMax, unsigned frame[])
{
    int i, bi;
    unsigned bit;
    uint32_t clearMasks[16];
    uint32_t msk, allMotorsMask;

    assert(motorMax < DSHOT_NUM_PINS);

    allMotorsMask = dshotGetAllMotorsPinMask(motorPins, motorMax);

    // compute masks for zero bits in all frames
    for (bi = 0; bi < 16; bi++)
    {
        msk = 0;
        bit = (0x8000 >> bi);
        for (i = 0; i < motorMax; i++)
        {
            if ((frame[i] & bit) == 0)
                msk |= (1 << motorPins[i]);
        }
        clearMasks[bi] = msk;
    }

    dshotSend(allMotorsMask, clearMasks);
}

//////////////////////////////////////////////////////////////////////
// Verificam rezivia de raspberry pi ,verificand ce model de procesor este in usage
// Vom aplica offesetul necesar pentru a putea acces GPIO-ul
// Offesul necesar este de obicei gasit in documentatia modelului
static uint32_t getGpioRegBase(void)
{
    const char *revision_file = "/proc/device-tree/system/linux,revision";
    uint8_t revision[4] = {0};
    uint32_t cpu = 4;
    FILE *fd;

    if ((fd = fopen(revision_file, "rb")) == NULL)
    {
        printf("debug Error: Can't open '%s'\n", revision_file);
    }
    else
    {
        if (fread(revision, 1, sizeof(revision), fd) == 4)
        {
            cpu = (revision[2] >> 4) & 0xf;
        }
        else
        {
            printf("debug Error: Revision data too short\n");
        }
        fclose(fd);
    }

    // printf("debug CPU type: %d\n", cpu);
    switch (cpu)
    {
    case 0: // BCM2835 [Pi 1 A; Pi 1 B; Pi 1 B+; Pi Zero; Pi Zero W]
        return (0x20000000 + GPIO_BASE_OFFSET);
    case 1: // BCM2836 [Pi 2 B]
    case 2: // BCM2837 [Pi 3 B; Pi 3 B+; Pi 3 A+]
        return (0x3f000000 + GPIO_BASE_OFFSET);
    case 3: // BCM2711 [Pi 4 B]
        return (0xfe000000 + GPIO_BASE_OFFSET);
    default:
        printf("debug Error: Unrecognised revision code\n");
        return (0xfe000000 + GPIO_BASE_OFFSET);
    }
}

//////////////////////////////////////////////////////////////////////
// Aceasa functie ne asigura infrastuctura necesara pentru a accesa registri de la GPIO
// Configurarea pinilor, citire si scriere
static void dshotSetupIo()
{
    int mem_fd;
    int32_t gpioBase;

    gpioBase = getGpioRegBase();

    /* open /dev/mem */
    if ((mem_fd = open("/dev/gpiomem", O_RDWR | O_SYNC)) < 0)
    {
        printf("debug Error: Can't open /dev/mem \n");
        exit(-1);
    }

    /* mmap GPIO */
    dshotGpioMap = mmap(
        NULL,                   // Any adddress in our space will do
        BLOCK_SIZE,             // Map length
        PROT_READ | PROT_WRITE, // Enable reading & writting to mapped memory
        MAP_SHARED,             // Shared with other processes
        mem_fd,                 // File to map
        gpioBase                // Offset to GPIO peripheral
    );

    close(mem_fd); // No need to keep mem_fd open after mmap

    if (dshotGpioMap == MAP_FAILED)
    {
        printf("debug Mmap error %p\n", dshotGpioMap); // errno also set!
        exit(-1);
    }

    // Always use volatile pointer!
    dshotGpio = (volatile unsigned *)dshotGpioMap;
}

//////////////////////////////////////////////////////////////////////
// Send a command repeatedly during a given perion of time
static void dshotRepeatSendCommand(int motorPins[], int motorMax, int cmd, int telemetry, int timePeriodMsec)
{
    unsigned frame[DSHOT_NUM_PINS + 1];
    int i;
    int64_t t;

    for (i = 0; i < motorMax; i++)
        frame[i] = dshotAddChecksumAndTelemetry(cmd, telemetry);
    t = dshotGetNanoseconds() + timePeriodMsec * 1000000LL;
    while (dshotGetNanoseconds() <= t)
    {
        dshotSendFrames(motorPins, motorMax, frame);
        usleep(USLEEP_BEFORE_REBROADCAST);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Main exported functions of the module implementing raspilot motor instance.
//////////////////////////////////////////////////////////////////////////////////////////////

// This function allows to set bidirectional rotation (mode3dFlag!=0) and reverse rotation logic (reverseDirectionFlag!=0).
// Changing 3D mode is interfering with rotation direction (at least on my ESC), so always reset the direction when changing 3D.
void motorImplementationSet3dModeAndSpinDirection(int motorPins[], int motorMax, int mode3dFlag, int reverseDirectionFlag)
{
    int repeatMsec;

    // Repeat the command for some time to take effect. Do it longer in our case as we are not sure to send
    // all the frames correctly. If repeatMsec == 25, then this function will take 4*25 msec.
    repeatMsec = 25;

    dshot3dMode = mode3dFlag;
    if (dshot3dMode)
    {
        dshotRepeatSendCommand(motorPins, motorMax, DSHOT_CMD_3D_MODE_ON, 1, repeatMsec);
    }
    else
    {
        dshotRepeatSendCommand(motorPins, motorMax, DSHOT_CMD_3D_MODE_OFF, 1, repeatMsec);
    }
    dshotRepeatSendCommand(motorPins, motorMax, DSHOT_CMD_SAVE_SETTINGS, 0, repeatMsec);

    if (reverseDirectionFlag)
    {
        dshotRepeatSendCommand(motorPins, motorMax, DSHOT_CMD_SPIN_DIRECTION_REVERSED, 1, repeatMsec);
    }
    else
    {
        dshotRepeatSendCommand(motorPins, motorMax, DSHOT_CMD_SPIN_DIRECTION_NORMAL, 1, repeatMsec);
    }
    dshotRepeatSendCommand(motorPins, motorMax, DSHOT_CMD_SAVE_SETTINGS, 0, repeatMsec);
}

void motorImplementationInitialize(int motorPins[], int motorMax)
{
    int i, pin;

    dshotSetupIo();

    for (i = 0; i < motorMax; i++)
    {
        pin = motorPins[i];
        INP_GPIO(pin); // must use INP_GPIO before we can use OUT_GPIO
        OUT_GPIO(pin);
        GPIO_CLR = 1 << pin;
    }

    // Maybe by default set to normal direction and no reverse spin
    // motorImplementationSet3dModeAndSpinDirection(motorPins, motorMax, 0, 0);
    // Arm motors by sending DSHOT_CMD_MOTOR_STOP (aka 0) for 5 seconds
    dshotRepeatSendCommand(motorPins, motorMax, DSHOT_CMD_MOTOR_STOP, 0, 5000);
}

void motorImplementationFinalize(int motorPins[], int motorMax)
{
    munmap(dshotGpioMap, BLOCK_SIZE);
}

void motorImplementationSendThrottles(int motorPins[], int motorMax,volatile double motorThrottle[])
{
    int i;
    unsigned frame[DSHOT_NUM_PINS + 1];
    int val;

    assert(motorMax < DSHOT_NUM_PINS);

    for (i = 0; i < motorMax; i++)
    {
        if (dshot3dMode)
        {
            // translate double throttles ranging <-1, 1> to dshot frames.
            if (motorThrottle[i] >= 0)
            {
                val = motorThrottle[i] * 999 + 1048;
            }
            else
            {
                val = -motorThrottle[i] * 999 + 48;
            }
        }
        else
        {
            // translate double throttles ranging <0, 1> to dshot frames.
            val = motorThrottle[i] * 1999 + 48;
        }
        // we used command 0 for zero thrust which should be used as arming sequence as well.
        // but in 3d mode we have to be carefull it seems to reset the motor.
        if (/*motorThrottle[i] == 0 || */ val < 48 || val >= 2048)
            val = DSHOT_CMD_MOTOR_STOP;
        frame[i] = dshotAddChecksumAndTelemetry(val, 0);
    }

    dshotSendFrames(motorPins, motorMax, frame);
}
