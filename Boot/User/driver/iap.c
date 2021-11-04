#include "usart.h"
#include "stmflash.h"
#include "iap.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEKս��STM32������
//IAP ����	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2012/9/24
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////	

iapfun jump2app; 
//u16 iapbuf[1024];

void NVIC_DeInit(void)
{
	u32 index = 0;

	NVIC->ICER[0] = 0xFFFFFFFF;
	NVIC->ICER[1] = 0x000007FF;
	NVIC->ICPR[0] = 0xFFFFFFFF;
	NVIC->ICPR[1] = 0x000007FF;

	for(index = 0; index < 0x0B; index++)
	{
		NVIC->IP[index] = 0x00000000;
	} 
}
//appxaddr:Ӧ�ó������ʼ��ַ
//appbuf:Ӧ�ó���CODE.
//appsize:Ӧ�ó����С(�ֽ�).
void iap_write_appbin(u32 appxaddr,u8 *appbuf,u32 appsize)
{
//	STMFLASH_Write(appxaddr,(u16 *)appbuf,appsize);
//	u16 t;
//	u16 i=0;
//	u16 temp;
//	u32 fwaddr=appxaddr;//��ǰд��ĵ�ַ
//	u8 *dfu=appbuf;
//	for(t=0;t<appsize;t+=2)
//	{						    
//		temp=(u16)dfu[1]<<8;
//		temp+=(u16)dfu[0];	  
//		dfu+=2;//ƫ��2���ֽ�
//		iapbuf[i++]=temp;	    
//		if(i==1024)
//		{
//			i=0;
//			STMFLASH_Write(fwaddr,iapbuf,1024);	
//			fwaddr+=2048;//ƫ��2048  16=2*8.����Ҫ����2.
//		}
//	}
//	if(i)STMFLASH_Write(fwaddr,iapbuf,i);//������һЩ�����ֽ�д��ȥ.  
}

//��ת��Ӧ�ó����
//appxaddr:�û�������ʼ��ַ.
void iap_load_app(u32 appxaddr)
{
	if(((*(vu32*)appxaddr)&0x2FFE0000)==0x20000000)	//���ջ����ַ�Ƿ�Ϸ�.
	{ 
		NVIC_DeInit( );
		jump2app=(iapfun)*(vu32*)(appxaddr+4);		//�û��������ڶ�����Ϊ����ʼ��ַ(��λ��ַ)		
		MSR_MSP(*(vu32*)appxaddr);					//��ʼ��APP��ջָ��(�û��������ĵ�һ�������ڴ��ջ����ַ)
		jump2app();									//��ת��APP.
	}
}		 


//����ջ����ַ
//addr:ջ����ַ
__asm void MSR_MSP(u32 addr) 
{
    MSR MSP, r0 			//set Main Stack value
    BX r14
}

