#include <stdio.h>
#include <Winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#include "math.h"
#pragma warning(disable : 4996)
//������
#define 	cmd_connect			0xaf3c2828 //ok���Ӻ���
#define     cmd_mouse_move		0xaede7345 //ok����ƶ�
#define		cmd_mouse_left		0x9823AE8D //ok����������
#define		cmd_mouse_middle	0x97a3AE8D //ok����м�����
#define		cmd_mouse_right		0x238d8212 //ok����Ҽ�����
#define		cmd_mouse_wheel		0xffeead38 //ok�����ֿ���
#define     cmd_mouse_automove	0xaede7346 //ok����Զ�ģ���˹��ƶ�����
#define     cmd_keyboard_all    0x123c2c2f //ok�������в�������
#define		cmd_reboot			0xaa8855aa //ok��������
#define     cmd_bazerMove       0xa238455a //ok��건�����ƶ�
#define     cmd_monitor         0x27388020 //ok��غ����ϵ�������������
#define     cmd_debug           0x27382021 //ok����������Ϣ
#define     cmd_mask_mouse      0x23234343 //ok ������������
#define     cmd_unmask_all      0x23344343 //ok ���������������
#define     cmd_setconfig       0x1d3d3323 //ok ����IP������Ϣ
#define     cmd_showpic         0x12334883 //��ʾͼƬ

extern SOCKET sockClientfd; //socketͨ�ž��
typedef struct
{
	unsigned int  mac;			//���ӵ�mac��ַ�����룩
	unsigned int  rand;			//���ֵ
	unsigned int  indexpts;		//ʱ���
	unsigned int  cmd;			//ָ����
}cmd_head_t;

typedef struct
{
	unsigned char buff[1024];	//
}cmd_data_t;
typedef struct
{
	unsigned short buff[512];	//
}cmd_u16_t;

//������ݽṹ��
typedef struct
{
	int button;
	int x;
	int y;
	int wheel;
	int point[10];//���ڱ��������߿���(Ԥ��5�׵�)
}soft_mouse_t;

//�������ݽṹ��
typedef struct
{
	char ctrl;
	char resvel;
	char button[10];
}soft_keyboard_t;

//������
typedef struct
{
	cmd_head_t head;
	union {
		cmd_data_t      u8buff;		  //buff
		cmd_u16_t       u16buff;	  //U16
		soft_mouse_t    cmd_mouse;    //��귢��ָ��
		soft_keyboard_t cmd_keyboard; //���̷���ָ��
	};
}client_tx;

enum
{
	err_creat_socket = -9000,	//����socketʧ��
	err_net_version,		//socket�汾����
	err_net_tx,		//socket���ʹ���
	err_net_rx_timeout,		//socket���ճ�ʱ
	err_net_cmd,			//�������
	err_net_pts,			//ʱ�������
	success = 0,				//����ִ��
	usb_dev_tx_timeout,		//USB devic����ʧ��
};



/*
����kmboxNet������������ֱ��Ǻ���
ip  �����ӵ�IP��ַ ����ʾ���ϻ�����ʾ��
port: ͨ�Ŷ˿ں�   ����ʾ���ϻ�����ʾ��
mac : ���ӵ�mac��ַ����ʾ��Ļ������ʾ��
����ֵ�����ӳɹ�����0 ������ֵ�μ��������
*/
int kmNet_init(char* ip, char* port, char* mac);//ok
int kmNet_mouse_move(short x, short y);			//ok
int kmNet_mouse_left(int isdown);				//ok
int kmNet_mouse_right(int isdown);				//ok
int kmNet_mouse_middle(int isdown);				//ok
int kmNet_mouse_wheel(int wheel);				//ok
int kmNet_mouse_all(int button, int x, int y, int wheel);//ok
int kmNet_mouse_move_auto(int x, int y, int time_ms);	//ok
int kmNet_mouse_move_beizer(int x, int y, int ms, int x1, int y1, int x2, int y2);//��������

//���̺���
int kmNet_keydown(int vkey);// ok
int kmNet_keyup(int vkey);  // ok

//���ϵ��
int kmNet_monitor(short port);			//�����ر�����������
int kmNet_monitor_mouse_left();			//��ѯ����������״̬
int kmNet_monitor_mouse_middle();		//��ѯ����м�״̬
int kmNet_monitor_mouse_right();		//��ѯ����Ҽ�״̬
int kmNet_monitor_mouse_side1();		//��ѯ�����1״̬
int kmNet_monitor_mouse_side2();		//��ѯ�����2״̬ 
int kmNet_monitor_keyboard(short  vk_key);//��ѯ����ָ������״̬
//������������ϵ��
int kmNet_mask_mouse_left(int enable);	//���������� 
int kmNet_mask_mouse_right(int enable);	//��������Ҽ� 
int kmNet_mask_mouse_middle(int enable);//��������м� 
int kmNet_mask_mouse_side1(int enable);	//�����������1 
int kmNet_mask_mouse_side2(int enable);	//�����������2
int kmNet_mask_mouse_x(int enable);		//�������X������
int kmNet_mask_mouse_y(int enable);		//�������y������
int kmNet_mask_mouse_wheel(int enable);	//����������
int kmNet_mask_keyboard(short vkey);	//���μ���ָ������
int kmNet_unmask_keyboard(short vkey);	//�������ָ������
int kmNet_unmask_all();					//������������Ѿ����õ���������


//�����ຯ��
int kmNet_reboot(void);
int kmNet_setconfig(char* ip, unsigned short port);		  //���ú���IP��ַ
int kmNet_debug(short port, char enable);				  //����ʹ��
int kmNet_lcd_color(unsigned short rgb565);				  //������LCD��Ļ��ָ����ɫ��䡣 ���������ú�ɫ
int kmNet_lcd_picture_bottom(unsigned char* buff_128_80); //�°벿����ʾ128x80ͼƬ
int kmNet_lcd_picture(unsigned char* buff_128_160);		  //������ʾ128x160ͼƬ


//��Ƶ�ɼ��ຯ��
#if 0
int kvm_setvideo(int width, int height, int fps);//���òɼ���Ƶ�ֱ���֡��
int kvm_getframe(Mat* frame);					 //��ȡ����һ֡ͼ������
int kvm_load_yolo_module(char* path);			 //����yoloģ��·��
int kvm_run_yolo(Mat* frame, int cpu_gpu);		 //ͼ���Ƶ�
#endif
