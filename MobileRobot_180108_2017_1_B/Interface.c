#include <util/delay.h>
#include <avr/interrupt.h>
#include "Interface.h"
#include "Move.h"
#include "Motor.h"

volatile unsigned char rx1_flg=0, rx1_buff=0;
volatile unsigned char CameraV1_buff[137],CameraV1_EN=0, CameraV1_flg=0, CameraV1_cnt=0;

// Function  : MCU �ʱ�ȭ.
// Parameter : ����
// Return    : ����
void MCU_init(void)
{
	// USART1 initialization
	// Communication Parameters: 8 Data, 1 Stop, No Parity
	// USART1 Receiver: On
	// USART1 Transmitter: On
	// USART1 Mode: Asynchronous
	// USART1 Baud Rate: 9600
	UCSR1A=0x00;
	UCSR1B=0x98;
	UCSR1C=0x06;
	//UBRR1H=0x00;
	//UBRR1L=0x5F;
	UBRR1H=0x00;
	UBRR1L=0x07;

	sei();
}

ISR(USART1_RX_vect)
{
	char data;
	
	data=UDR1;
	while(!(UCSR1A&0xB8)); //���ſϷ� �� ���� ������� ������
	rx1_flg=1;              //���ſϷ� �÷��� set
	rx1_buff=data;

	if(CameraV1_EN){
		//PORTB^=0X07;
		if(rx1_buff=='V'){
			CameraV1_cnt=0;
			//PORTB^=0X07;
		}
		CameraV1_buff[CameraV1_cnt++]=rx1_buff;
	}
}

char getchar1(void)  //uart �����ϱ�
{
	char data;
	
	data=rx1_buff;
	rx1_buff=0;
	cli();           //�۷ι� ���ͷ�Ʈ disable
	rx1_flg=0;       //���ſϷ� �÷��� reset
	sei();           //�۷ι� ���ͷ�Ʈ enable          
	return data;     //���ŵ����� ��ȯ
}

void putchar1(char data) //uart �۽�ȭ��
{
	while(!(UCSR1A&0x20));  //�۽� �غ� �Ϸ��
	UDR1=data;              //�۽� ���ۿ� ������ ����
}



// Function  : LCD, LED, Buzzer, Switch�� ����ϱ� ���� �ʱ�ȭ
// Parameter : ����
// Return    : ����
void Interface_init(void)
{
    // LCD / EX I/O Expander
	TWDR = 0xFF;
	TWBR = 0x48;
}

// Function  : I2C�� ����Ͽ� LCD���� �ѹ���Ʈ ����
// Parameter :
//          data - ������ �ѹ���Ʈ ������
// Return    : ����
void lcd_write_data(unsigned char data)
{
	// ����
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTA);
	// �غ���� üũ
    while(!(TWCR & (1 << TWINT)));
	
	// �ּ� ����
    TWDR = SLA << 1;
	
	TWCR = (1 << TWINT) | (1 << TWEN);
	while(!(TWCR & (1 << TWINT)));
	
	TWDR = data;
	TWCR = (1 << TWINT) | (1 << TWEN);
	while(!(TWCR & (1 << TWINT)));
	
    // ����
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
}

// Function  : ���ϴ� ���ο� ������ ���ڿ� ���
// Parameter :
//          line   - ����� ���ι�ȣ(0~3)
//          string - ����� ���ڿ�(�ִ� 20��)
// Return    : ����
void lcd_display_str(unsigned char Y_line, unsigned char X_line,char *string) //lcd ��Ʈ�������� ����
{
    int i=X_line;

	if((Y_line<4) && (X_line<20)){
		lcd_write_data(0x1B);   lcd_write_data(0x4C);
		lcd_write_data(X_line); lcd_write_data(Y_line);
		_delay_us(100);
    	while(((i++)<20)&&*string){
        	lcd_write_data(*string++);
			_delay_us(40);
		}
    }
}

void write_lcd_data(unsigned char Y_line, unsigned char X_line, long data) //lcd������ ����
{
    if(Y_line<4 && X_line<20){
		lcd_write_data(0x1B);   lcd_write_data(0x4C);
		lcd_write_data(X_line); lcd_write_data(Y_line);
		_delay_us(100);
		lcd_write_data(data);
		_delay_us(40);
    }
}

void lcd_clear_screen(void){ lcd_write_data(0x1B); lcd_write_data(0x43); _delay_ms(20); } //lcd��üȭ�� Ŭ����
void display_char(unsigned char line, unsigned char col, unsigned char data) ////lcd char ������ ǥ��
{
    unsigned char val=0, val_buff=0;

	val=data;   

	val_buff=val/100;             
	write_lcd_data(line, col, val_buff+'0' );
	val=val-(100*val_buff);

	val_buff=val/10;
	write_lcd_data(line, 1+col, val_buff+'0' );
	val=val-(10*val_buff);

	write_lcd_data(line, 2+col, val+'0');
}

