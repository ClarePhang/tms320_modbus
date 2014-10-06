/*
 * Uart.c
 *
 *  Created on: 05 ���. 2014 �.
 *      Author: lamazavr
 */

#include "Uart.h"
#include "modbus_slave.h"

// ����� ��� modbus
Uint16 UartBuffer[50];

// ���������� ������ � ������
Uint16 UartRxLen = 0;

void Uart_init()
{
	Uart_init_gpio();

    // ������������ ������ ���� �������� �
	// InitSysCtrl() --> InitPeripheralClocks()

    SciaRegs.SCICCR.all =0x0007;   // 1 ���� ���,  ��� loopback
                                   // ��� �������� ��������, 8 ���,
                                   // ����������� ��������

    SciaRegs.SCICTL1.all =0x0043;  // ��� TX, RX, internal SCICLK,
                                   // ���� RX ERR, SLEEP, TXWAKE
    							   // RX interrupt

    SciaRegs.SCICTL2.bit.TXINTENA = 1;
    SciaRegs.SCICTL2.bit.RXBKINTENA = 1;

    SciaRegs.SCIFFTX.all=0xC022;
    SciaRegs.SCIFFRX.all=0x0022;
    SciaRegs.SCIFFCT.all=0x00;

    // SCI BRR = LSPCLK/(SCI BAUDx8) - 1

    // 9600 ���
    // 194 = 15000000 / (9600*8) - 1

    // 256000 ���
    // 6   = 15000000 / (256000*8) - 1

	SciaRegs.SCIHBAUD    = 0x0000;
	SciaRegs.SCILBAUD    = 0x0006;  // ��� 256000 ���

    SciaRegs.SCICTL1.all = 0x0023;  // Relinquish SCI from Reset
    SciaRegs.SCIFFTX.bit.TXFIFOXRESET=1;
    SciaRegs.SCIFFRX.bit.RXFIFORESET=1;

    // ��������� ���������� �� ������ ������ �� uart
	PieCtrlRegs.PIECTRL.bit.ENPIE = 1;
	PieCtrlRegs.PIEIER9.bit.INTx1 = 1;

	IER |= M_INT9; // ��������� ���������� ���� �� ����� 9

	Uart_timer_init();
}

void Uart_init_gpio()
{
	EALLOW;
	GpioCtrlRegs.GPAPUD.bit.GPIO28 = 0;    // �������� �� GPIO28 (SCIRXDA)
	GpioCtrlRegs.GPAPUD.bit.GPIO29 = 1;	   // ���� �������� �� GPIO29 (SCITXDA)
	GpioCtrlRegs.GPAQSEL2.bit.GPIO28 = 3;  // ����������� ���� GPIO28 (SCIRXDA)
	GpioCtrlRegs.GPAMUX2.bit.GPIO28 = 1;   // GPIO28 --> SCIRXDA
	GpioCtrlRegs.GPAMUX2.bit.GPIO29 = 1;   // GPIO29 --> SCITXDA
	EDIS;
}

void Uart_send(Uint16 a)
{
    while (SciaRegs.SCIFFTX.bit.TXFFST != 0) {}
    SciaRegs.SCITXBUF = a;
}

void Uart_send_msg(char *msg)
{
    while(*msg)
    {
    	Uart_send(*msg++);
    }
}

void Uart_timer_init()
{
	EALLOW;
	PieVectTable.TINT0 = &cpu_timer0_isr;
	EDIS;

	InitCpuTimers();
	ConfigCpuTimer(&CpuTimer0, 60, 300000); // 300ms

	CpuTimer0Regs.TCR.all = 0x4001;
	IER |= M_INT1;

	// Enable TINT0 in the PIE: Group 1 interrupt 7
	PieCtrlRegs.PIEIER1.bit.INTx7 = 1;
}

interrupt void cpu_timer0_isr(void)
{
	// stop timer
	CpuTimer0.RegsAddr->TCR.bit.TSS = 1;

	if (UartRxLen > 0)
	{
	   Uint16 len = modbus_func(UartBuffer, UartRxLen, 2);
	   Uint16 i = 0;
	   //TODO: �������� �� ����������
	   for (; i <= len; i++)
	   {
		   Uart_send(UartBuffer[i]);
	   }
	   UartRxLen = 0;
	}

	// start timer
	CpuTimer0.RegsAddr->TCR.bit.TSS = 0;

	// Acknowledge this interrupt to receive more interrupts from group 1
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}
