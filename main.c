/**
 * ������ ���������� ���������� Modbus RTU
 * @autor lamazavr
 * @url how2.org.ua
 * @date 04.10.2014
 */
#include "DSP28x_Project.h"

// ���������
void scia_echoback_init(void);
void scia_fifo_init(void);
void scia_xmit(int a);
void scia_msg(char *msg);
void scia_init_gpio();

// ���������� ������ ������
Uint16 LoopCount;

#include "parameters.h"

// ��� ���������� ��� �����
Uint16 LedsState;
Uint16 TestPrm;

// ����� ��� modbus
Uint16 UartBuffer[50];

void main(void)
{
	char *msg;

	InitSysCtrl();

	DINT;

	InitPieCtrl();
	IER = 0x0000;
	IFR = 0x0000;
	InitPieVectTable();

	// ����������� GPIO ��� SCI
	scia_init_gpio();

	LoopCount = 0;

	scia_fifo_init();      // ��������� SCI FIFO
	scia_echoback_init();  // ��������� SCI

	// ��������� ���������� �� ������ ������ �� uart
	PieCtrlRegs.PIECTRL.bit.ENPIE = 1;
	PieCtrlRegs.PIEIER9.bit.INTx1 = 1;

	IER |= M_INT1; // ��������� ���������� ���� �� ����� 1
	EINT;          // ��������� ���������� ���������

    msg = "\r\nEnter a character:";
    scia_msg(msg);

	for(;;)
    {
       LoopCount++;

       DELAY_US(100);
    }

}

void scia_echoback_init()
{
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

    // SCI BRR = LSPCLK/(SCI BAUDx8) - 1

    // 9600 ���
    // 194 = 15000000 / (9600*8) - 1

    // 256000 ���
    // 6   = 15000000 / (256000*8) - 1

	SciaRegs.SCIHBAUD    = 0x0000;
	SciaRegs.SCILBAUD    = 0x0006;  // ��� 256000 ���

    SciaRegs.SCICTL1.all = 0x0023;  // Relinquish SCI from Reset
}

/**
 * �������� �� SCI
 */
void scia_xmit(int a)
{
    while (SciaRegs.SCIFFTX.bit.TXFFST != 0) {}
    SciaRegs.SCITXBUF = a;
}

void scia_msg(char *msg)
{
    while(*msg)
    {
        scia_xmit(*msg++);
    }
}

void scia_fifo_init()
{
    SciaRegs.SCIFFTX.bit.SCIRST = 1; 		// ����� ����� rx/tx
	SciaRegs.SCIFFTX.bit.SCIFFENA = 1;		// ��������� FIFO
	SciaRegs.SCIFFTX.bit.TXFIFOXRESET = 1;	// ���������� FIFO
	SciaRegs.SCIFFTX.bit.TXFFINTCLR = 1;	// ������� ���� ����������

	// ��������� ��, ��� ���� ����� �������
	//SpiaRegs.SPIFFTX.all=0xE040;
}

void scia_init_gpio()
{
	EALLOW;
	GpioCtrlRegs.GPAPUD.bit.GPIO28 = 0;    // �������� �� GPIO28 (SCIRXDA)
	GpioCtrlRegs.GPAPUD.bit.GPIO29 = 1;	   // ���� �������� �� GPIO29 (SCITXDA)
	GpioCtrlRegs.GPAQSEL2.bit.GPIO28 = 3;  // ����������� ���� GPIO28 (SCIRXDA)
	GpioCtrlRegs.GPAMUX2.bit.GPIO28 = 1;   // GPIO28 --> SCIRXDA
	GpioCtrlRegs.GPAMUX2.bit.GPIO29 = 1;   // GPIO29 --> SCITXDA
	EDIS;
}
