#ifndef __UART_H
#define __UART_H

int uart_init(int fd, char *path);
int uart_set(int fd, int u_speed, int flow_ctrl, int databits, int stopbits, int parity);
int uart_send(int fd, char *send_buf,int data_len);
int uart_receive(int fd, char *rcv_buf,int data_len); 


#endif 

