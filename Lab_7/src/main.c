#include <xc.h>
#include <stdint.h>
#include <math.h>
#include "../include/Configuration.h"
#include "../include/SPI.h"
#include "../include/LIS3DH.h"
#include "../include/UART.h"
#include "../include/StateController.h"

#define _XTAL_FREQ 16000000UL

volatile uint32_t system_ms = 0;
volatile uint8_t fault_event = 0;
volatile uint8_t arm_event = 0;
volatile uint8_t comm_event = 0;
volatile uint8_t accel_sample_flag = 0;
volatile uint8_t telemetry_flag = 0;

static void configure_sys(void)
{
    INTCONbits.GIEH = 0;
    INTCONbits.GIEL = 0;

    OSCCONbits.IRCF = 0b111;
    OSCCONbits.SCS = 0b10;
    OSCTUNEbits.PLLEN = 0;

    ANSELB = 0x00;
    ANSELC = 0x00;
    ANSELD = 0x00;

    TRISB = 0x07;
    WPUB = 0x07;
    INTCON2bits.RBPU = 0;

    LATD = 0x01;
    TRISD = 0x00;

    T2CONbits.TMR2ON = 0;
    T2CONbits.T2OUTPS = 0;
    T2CONbits.T2CKPS = 0b10;
    TMR2 = 0;
    PR2 = 249;

    INTCON2bits.INTEDG0 = 0;
    INTCON2bits.INTEDG1 = 0;
    INTCON2bits.INTEDG2 = 0;

    RCONbits.IPEN = 1;
    INTCON3bits.INT1IP = 0;
    INTCON3bits.INT2IP = 0;
    IPR1bits.TMR2IP = 0;

    INTCONbits.INT0IF = 0;
    INTCON3bits.INT1IF = 0;
    INTCON3bits.INT2IF = 0;
    PIR1bits.TMR2IF = 0;

    INTCONbits.INT0IE = 1;
    INTCON3bits.INT1IE = 1;
    INTCON3bits.INT2IE = 1;
    PIE1bits.TMR2IE = 1;
}

static void start_timer(void)
{
    T2CONbits.TMR2ON = 1;
    INTCONbits.GIEL = 1;
    INTCONbits.GIEH = 1;
}

void __interrupt(high_priority) high_isr(void)
{
    if (INTCONbits.INT0IE && INTCONbits.INT0IF)
    {
        INTCONbits.INT0IF = 0;
        fault_event = 1;
        LATDbits.LATD4 = 0;
    }
}

void __interrupt(low_priority) low_isr(void)
{
    static uint8_t sample_tick = 0;
    static uint8_t telemetry_tick = 0;

    if (INTCON3bits.INT1IE && INTCON3bits.INT1IF)
    {
        INTCON3bits.INT1IF = 0;
        arm_event = 1;
    }

    if (INTCON3bits.INT2IE && INTCON3bits.INT2IF)
    {
        INTCON3bits.INT2IF = 0;
        comm_event = 1;
    }

    if (PIE1bits.TMR2IE && PIR1bits.TMR2IF)
    {
        PIR1bits.TMR2IF = 0;
        system_ms++;

        if (++sample_tick >= 10)
        {
            sample_tick = 0;
            accel_sample_flag = 1;
        }

        if (++telemetry_tick >= 100)
        {
            telemetry_tick = 0;
            telemetry_flag = 1;
        }
    }
}

static void read_events(uint32_t *tick, uint8_t *fault, uint8_t *arm,
                        uint8_t *comm, uint8_t *sample, uint8_t *telemetry)
{
    INTCONbits.GIEH = 0;
    INTCONbits.GIEL = 0;

    *tick = system_ms;
    *fault = fault_event;
    *arm = arm_event;
    *comm = comm_event;
    *sample = accel_sample_flag;
    *telemetry = telemetry_flag;

    fault_event = 0;
    arm_event = 0;
    comm_event = 0;
    accel_sample_flag = 0;
    telemetry_flag = 0;

    INTCONbits.GIEL = 1;
    INTCONbits.GIEH = 1;
}

static uint16_t accel_magnitude(int16_t ax, int16_t ay, int16_t az)
{
    int32_t x = ax;
    int32_t y = ay;
    int32_t z = az;
    return (uint16_t)sqrtf((float)(x * x + y * y + z * z));
}

static void print_telemetry(int16_t ax, int16_t ay, int16_t az)
{
    UART_String("DRONE=");
    UART_String(fsm_state_name());
    UART_String(" MOTION=");
    UART_String(fsm_motion_name());
    UART_String(" X=");
    UART_G(ax);
    UART_String("g Y=");
    UART_G(ay);
    UART_String("g Z=");
    UART_G(az);
    UART_String("g TOTAL=");
    UART_G((int16_t)accel_magnitude(ax, ay, az));
    UART_String("g\r\n");
}

void main(void)
{
    uint8_t who;
    uint8_t sensor_ok;
    uint8_t fault;
    uint8_t arm;
    uint8_t comm;
    uint8_t sample;
    uint8_t telemetry;
    uint32_t tick;
    int16_t ax = 0;
    int16_t ay = 0;
    int16_t az = 0;

    configure_sys();
    UART_Init();
    SPI_Init();
    UART_String("DRONE = SAFE\r\n");

    who = LIS3DH_ReadRegister(LIS3DH_WHO_AM_I);
    UART_String("WHO_AM_I = 0x");
    UART_Hex8(who);
    UART_String("\r\n");

    sensor_ok = LIS3DH_Init();

    if (sensor_ok)
    {
        UART_String("LIS3DH DETECTED\r\n");
        __delay_ms(80);
    }
    else
    {
        UART_String("LIS3DH ERROR\r\n");
    }

    fsm_init(sensor_ok);
    start_timer();

    while (1)
    {
        read_events(&tick, &fault, &arm, &comm, &sample, &telemetry);
        fsm_run(tick, fault, arm, comm);

        if (sample && sensor_ok)
        {
            LIS3DH_ReadAccel(&ax, &ay, &az);
            fsm_motion(ax, ay, az, tick);
        }

        if (telemetry)
        {
            if (sensor_ok)
            {
                print_telemetry(ax, ay, az);
            }
            else
            {
                UART_String("DRONE=FAULT MOTION=NO_SENSOR\r\n");
            }
        }
    }
}
