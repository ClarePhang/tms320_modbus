/*
 * Uart.h
 *
 *  Created on: 05 ���. 2014 �.
 *      Author: lamazavr
 */

#ifndef UART_H_
#define UART_H_

#include "DSP2802x_Device.h"

/**
 * ������������� �����������
 */
void Uart_init();

/**
 * ������������� ����� ��� Uart
 */
void Uart_init_gpio();

/**
 * ������� �����
 * @param a - ���������� ����
 */
void Uart_send(Uint16 a);

/**
 * ������� ������
 * @param msg - ��������� �� ������
 */
void Uart_send_msg(char *msg);



#endif /* UART_H_ */
