#include "kmboxNet.h"
#include "HidTable.h"
#include <time.h>
#define monitor_ok    2
#define monitor_exit  0
SOCKET sockClientfd = 0;				//��������ͨ�ž��
SOCKET sockMonitorfd = 0;				//��������ͨ�ž��
client_tx tx;							//���͵�����
client_tx rx;							//���յ�����
SOCKADDR_IN addrSrv;
soft_mouse_t    softmouse;				//�����������
soft_keyboard_t softkeyboard;			//������������
static int monitor_run = 0;				//�����������Ƿ�����
static int mask_keyboard_mouse_flag = 0;//��������״̬
static short monitor_port = 0;


#pragma pack(1)
typedef struct {
	unsigned char report_id;
	unsigned char buttons;		// 8 buttons available
	short x;					// -32767 to 32767
	short y;					// -32767 to 32767
	short wheel;				// -32767 to 32767
}standard_mouse_report_t;

typedef struct {
	unsigned char report_id;
	unsigned char buttons;      // 8 buttons���Ƽ�
	unsigned char data[10];     //���水��
}standard_keyboard_report_t;
#pragma pack()

standard_mouse_report_t		hw_mouse;   //Ӳ�������Ϣ
standard_keyboard_report_t	hw_keyboard;//Ӳ��������Ϣ

//����һ��A��B֮��������
int myrand(int a, int b)
{
	int min = a < b ? a : b;
	int max = a > b ? a : b;
	return ((rand() % (max - min)) + min);
}

unsigned int StrToHex(char* pbSrc, int nLen)
{
	char h1, h2;
	unsigned char s1, s2;
	int i;
	unsigned int pbDest[16] = { 0 };
	for (i = 0; i < nLen; i++) {
		h1 = pbSrc[2 * i];
		h2 = pbSrc[2 * i + 1];
		s1 = toupper(h1) - 0x30;
		if (s1 > 9)
			s1 -= 7;
		s2 = toupper(h2) - 0x30;
		if (s2 > 9)
			s2 -= 7;
		pbDest[i] = s1 * 16 + s2;
	}
	return pbDest[0] << 24 | pbDest[1] << 16 | pbDest[2] << 8 | pbDest[3];
}

int NetRxReturnHandle(client_tx* rx, client_tx* tx)		 //���յ�����
{
	if (rx->head.cmd != tx->head.cmd)
		return  err_net_cmd;//���������
	if (rx->head.indexpts != tx->head.indexpts)
		return  err_net_pts;//ʱ�������
	return 0;				//û�д��󷵻�0
	//return  rx->head.rand;//�����ķ���ֵ


}


/*
����kmboxNet������������ֱ��Ǻ���
ip  �����ӵ�IP��ַ ����ʾ���ϻ�����ʾ,���磺192.168.2.88��
port: ͨ�Ŷ˿ں�   ����ʾ���ϻ�����ʾ�����磺6234��
mac : ���ӵ�mac��ַ����ʾ��Ļ������ʾ�����磺12345��
����ֵ:0����������ֵ�뿴�������
*/
int kmNet_init(char* ip, char* port, char* mac)
{
	WORD wVersionRequested; WSADATA wsaData;	int err;
	wVersionRequested = MAKEWORD(1, 1);
	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0) 		return err_creat_socket;
	if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1) {
		WSACleanup(); sockClientfd = -1;
		return err_net_version;
	}
	srand((unsigned)time(NULL));
	sockClientfd = socket(AF_INET, SOCK_DGRAM, 0);
	addrSrv.sin_addr.S_un.S_addr = inet_addr(ip);
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(atoi(port));//�˿�UUID[1]>>16��16λ
	tx.head.mac = StrToHex(mac, 4);		 //���ӵ�mac �̶� UUID[1]
	tx.head.rand = rand();				 //���ֵ�������������������ݰ����ܡ�������������Ԥ��
	tx.head.indexpts = 0;				 //ָ��ͳ��ֵ
	tx.head.cmd = cmd_connect;			 //ָ��
	memset(&softmouse, 0, sizeof(softmouse));	//���������������
	memset(&softkeyboard, 0, sizeof(softkeyboard));//���������������
	err = sendto(sockClientfd, (const char*)&tx, sizeof(cmd_head_t), 0, (struct sockaddr*)&addrSrv, sizeof(addrSrv));
	Sleep(20);//��һ�����ӿ���ʱ��ȽϾ�
	int clen = sizeof(addrSrv);
	err = recvfrom(sockClientfd, (char*)&rx, 1024, 0, (struct sockaddr*)&addrSrv, &clen);
	if (err < 0)
		return err_net_rx_timeout;
	return NetRxReturnHandle(&rx, &tx);
}

/*
����ƶ�x,y����λ��һ�����ƶ����޹켣ģ�⣬�ٶ����.
�Լ�д�켣�ƶ�ʱʹ�ô˺�����
����ֵ��0����ִ�У�����ֵ�쳣��
*/
int kmNet_mouse_move(short x, short y)
{
	int err;
	if (sockClientfd <= 0)		return err_creat_socket;
	tx.head.indexpts++;				//ָ��ͳ��ֵ
	tx.head.cmd = cmd_mouse_move;	//ָ��
	tx.head.rand = rand();			//�������ֵ
	softmouse.x = x;
	softmouse.y = y;
	memcpy(&tx.cmd_mouse, &softmouse, sizeof(soft_mouse_t));
	int length = sizeof(cmd_head_t) + sizeof(soft_mouse_t);
	sendto(sockClientfd, (const char*)&tx, length, 0, (struct sockaddr*)&addrSrv, sizeof(addrSrv));
	softmouse.x = 0;
	softmouse.y = 0;
	SOCKADDR_IN sclient;
	int clen = sizeof(sclient);
	err = recvfrom(sockClientfd, (char*)&rx, 1024, 0, (struct sockaddr*)&sclient, &clen);
	if (err < 0)
		return err_net_rx_timeout;
	return NetRxReturnHandle(&rx, &tx);
}



/*
����������
isdown :0�ɿ� ��1����
����ֵ��0����ִ�У�����ֵ�쳣��
*/
int kmNet_mouse_left(int isdown)
{
	int err;
	if (sockClientfd <= 0)		return err_creat_socket;
	tx.head.indexpts++;				//ָ��ͳ��ֵ
	tx.head.cmd = cmd_mouse_left;	//ָ��
	tx.head.rand = rand();			//�������ֵ
	softmouse.button = (isdown ? (softmouse.button | 0x01) : (softmouse.button & (~0x01)));
	memcpy(&tx.cmd_mouse, &softmouse, sizeof(soft_mouse_t));
	int length = sizeof(cmd_head_t) + sizeof(soft_mouse_t);
	sendto(sockClientfd, (const char*)&tx, length, 0, (struct sockaddr*)&addrSrv, sizeof(addrSrv));
	SOCKADDR_IN sclient;
	int clen = sizeof(sclient);
	err = recvfrom(sockClientfd, (char*)&rx, 1024, 0, (struct sockaddr*)&sclient, &clen);
	if (err < 0)
		return err_net_rx_timeout;
	return NetRxReturnHandle(&rx, &tx);
}

/*
����м�����
isdown :0�ɿ� ��1����
����ֵ��0����ִ�У�����ֵ�쳣��
*/
int kmNet_mouse_middle(int isdown)
{
	int err;
	if (sockClientfd <= 0)		return err_creat_socket;
	tx.head.indexpts++;				//ָ��ͳ��ֵ
	tx.head.cmd = cmd_mouse_middle;	//ָ��
	tx.head.rand = rand();			//�������ֵ
	softmouse.button = (isdown ? (softmouse.button | 0x04) : (softmouse.button & (~0x04)));
	memcpy(&tx.cmd_mouse, &softmouse, sizeof(soft_mouse_t));
	int length = sizeof(cmd_head_t) + sizeof(soft_mouse_t);
	sendto(sockClientfd, (const char*)&tx, length, 0, (struct sockaddr*)&addrSrv, sizeof(addrSrv));
	SOCKADDR_IN sclient;
	int clen = sizeof(sclient);
	err = recvfrom(sockClientfd, (char*)&rx, 1024, 0, (struct sockaddr*)&sclient, &clen);
	if (err < 0)
		return err_net_rx_timeout;
	return NetRxReturnHandle(&rx, &tx);
}

/*
����Ҽ�����
isdown :0�ɿ� ��1����
����ֵ��0����ִ�У�����ֵ�쳣��
*/
int kmNet_mouse_right(int isdown)
{
	int err;
	if (sockClientfd <= 0)		return err_creat_socket;
	tx.head.indexpts++;				//ָ��ͳ��ֵ
	tx.head.cmd = cmd_mouse_right;	//ָ��
	tx.head.rand = rand();			//�������ֵ
	softmouse.button = (isdown ? (softmouse.button | 0x02) : (softmouse.button & (~0x02)));
	memcpy(&tx.cmd_mouse, &softmouse, sizeof(soft_mouse_t));
	int length = sizeof(cmd_head_t) + sizeof(soft_mouse_t);
	sendto(sockClientfd, (const char*)&tx, length, 0, (struct sockaddr*)&addrSrv, sizeof(addrSrv));
	SOCKADDR_IN sclient;
	int clen = sizeof(sclient);
	err = recvfrom(sockClientfd, (char*)&rx, 1024, 0, (struct sockaddr*)&sclient, &clen);
	if (err < 0)
		return err_net_rx_timeout;
	return NetRxReturnHandle(&rx, &tx);
}

//�����ֿ���
int kmNet_mouse_wheel(int wheel)
{
	int err;
	if (sockClientfd <= 0)		return err_creat_socket;
	tx.head.indexpts++;				//ָ��ͳ��ֵ
	tx.head.cmd = cmd_mouse_wheel;	//ָ��
	tx.head.rand = rand();			//�������ֵ
	softmouse.wheel = wheel;
	memcpy(&tx.cmd_mouse, &softmouse, sizeof(soft_mouse_t));
	int length = sizeof(cmd_head_t) + sizeof(soft_mouse_t);
	softmouse.wheel = 0;
	sendto(sockClientfd, (const char*)&tx, length, 0, (struct sockaddr*)&addrSrv, sizeof(addrSrv));
	SOCKADDR_IN sclient;
	int clen = sizeof(sclient);
	err = recvfrom(sockClientfd, (char*)&rx, 1024, 0, (struct sockaddr*)&sclient, &clen);
	if (err < 0)
		return err_net_rx_timeout;
	return NetRxReturnHandle(&rx, &tx);
}


/*
���ȫ������ƺ���
*/
int kmNet_mouse_all(int button, int x, int y, int wheel)
{
	int err;
	if (sockClientfd <= 0)		return err_creat_socket;
	tx.head.indexpts++;				//ָ��ͳ��ֵ
	tx.head.cmd = cmd_mouse_wheel;	//ָ��
	tx.head.rand = rand();			//�������ֵ
	softmouse.button = button;
	softmouse.x = x;
	softmouse.y = y;
	softmouse.wheel = wheel;
	memcpy(&tx.cmd_mouse, &softmouse, sizeof(soft_mouse_t));
	int length = sizeof(cmd_head_t) + sizeof(soft_mouse_t);
	softmouse.x = 0;
	softmouse.y = 0;
	softmouse.wheel = 0;
	sendto(sockClientfd, (const char*)&tx, length, 0, (struct sockaddr*)&addrSrv, sizeof(addrSrv));
	SOCKADDR_IN sclient;
	int clen = sizeof(sclient);
	err = recvfrom(sockClientfd, (char*)&rx, 1024, 0, (struct sockaddr*)&sclient, &clen);
	if (err < 0)
		return err_net_rx_timeout;
	return NetRxReturnHandle(&rx, &tx);
}

/*
����ƶ�x,y����λ��ģ����Ϊ�ƶ�x,y����λ��������ּ����쳣�ļ��.
û��д�ƶ����ߵ��Ƽ��ô˺������˺������������Ծ���󣬰�����С�����ƽ�
Ŀ��㡣��ʱ��kmNet_mouse_move�ߡ�
ms�������ƶ���Ҫ���ٺ���.ע��ms����ֵ��Ҫ̫С��̫Сһ������ּ��������쳣��
�������˲�����ʵ����ʱ���msС��
*/
int kmNet_mouse_move_auto(int x, int y, int ms)
{
	int err;
	if (sockClientfd <= 0)		return err_creat_socket;
	tx.head.indexpts++;				 //ָ��ͳ��ֵ
	tx.head.cmd = cmd_mouse_automove;//ָ��
	tx.head.rand = ms;			     //�������ֵ
	softmouse.x = x;
	softmouse.y = y;
	memcpy(&tx.cmd_mouse, &softmouse, sizeof(soft_mouse_t));
	int length = sizeof(cmd_head_t) + sizeof(soft_mouse_t);
	softmouse.x = 0;				//����
	softmouse.y = 0;				//����
	sendto(sockClientfd, (const char*)&tx, length, 0, (struct sockaddr*)&addrSrv, sizeof(addrSrv));
	SOCKADDR_IN sclient;
	int clen = sizeof(sclient);
	err = recvfrom(sockClientfd, (char*)&rx, 1024, 0, (struct sockaddr*)&sclient, &clen);
	if (err < 0)
		return err_net_rx_timeout;
	return NetRxReturnHandle(&rx, &tx);
}



/*
���ױ��������߿���
x,y 	:Ŀ�������
ms		:��ϴ˹�����ʱ����λms��
x1,y1	:���Ƶ�p1������
x2,y2	:���Ƶ�p2������
*/
int kmNet_mouse_move_beizer(int x, int y, int ms, int x1, int y1, int x2, int y2)
{
	int err;
	if (sockClientfd <= 0)		return err_creat_socket;
	tx.head.indexpts++;			 //ָ��ͳ��ֵ
	tx.head.cmd = cmd_bazerMove; //ָ��
	tx.head.rand = ms;			 //�������ֵ
	softmouse.x = x;
	softmouse.y = y;
	softmouse.point[0] = x1;
	softmouse.point[1] = y1;
	softmouse.point[2] = x2;
	softmouse.point[3] = y2;
	memcpy(&tx.cmd_mouse, &softmouse, sizeof(soft_mouse_t));
	int length = sizeof(cmd_head_t) + sizeof(soft_mouse_t);
	softmouse.x = 0;
	softmouse.y = 0;
	sendto(sockClientfd, (const char*)&tx, length, 0, (struct sockaddr*)&addrSrv, sizeof(addrSrv));
	SOCKADDR_IN sclient;
	int clen = sizeof(sclient);
	err = recvfrom(sockClientfd, (char*)&rx, 1024, 0, (struct sockaddr*)&sclient, &clen);
	if (err < 0)
		return err_net_rx_timeout;
	return NetRxReturnHandle(&rx, &tx);
}



int kmNet_keydown(int vk_key)
{
	int i;
	if (vk_key >= KEY_LEFTCONTROL && vk_key <= KEY_RIGHT_GUI)//���Ƽ�
	{
		switch (vk_key)
		{
		case KEY_LEFTCONTROL: softkeyboard.ctrl |= BIT0; break;
		case KEY_LEFTSHIFT:   softkeyboard.ctrl |= BIT1; break;
		case KEY_LEFTALT:     softkeyboard.ctrl |= BIT2; break;
		case KEY_LEFT_GUI:    softkeyboard.ctrl |= BIT3; break;
		case KEY_RIGHTCONTROL:softkeyboard.ctrl |= BIT4; break;
		case KEY_RIGHTSHIFT:  softkeyboard.ctrl |= BIT5; break;
		case KEY_RIGHTALT:    softkeyboard.ctrl |= BIT6; break;
		case KEY_RIGHT_GUI:   softkeyboard.ctrl |= BIT7; break;
		}
	}
	else
	{//�����  
		for (i = 0; i < 10; i++)//���ȼ��������Ƿ����vk_key
		{
			if (softkeyboard.button[i] == vk_key)
				goto KM_down_send;// ���������Ѿ���vk_key ֱ�ӷ��;���
		}
		//��������û��vk_key 
		for (i = 0; i < 10; i++)//�������е����ݣ���vk_key���ӵ�������
		{
			if (softkeyboard.button[i] == 0)
			{// ���������Ѿ���vk_key ֱ�ӷ��;���
				softkeyboard.button[i] = vk_key;
				goto KM_down_send;
			}
		}
		//�����Ѿ����� ��ô���޳��ʼ���Ǹ�
		memcpy(&softkeyboard.button[0], &softkeyboard.button[1], 10);
		softkeyboard.button[9] = vk_key;
	}
KM_down_send:
	int err;
	if (sockClientfd <= 0)		return err_creat_socket;
	tx.head.indexpts++;				//ָ��ͳ��ֵ
	tx.head.cmd = cmd_keyboard_all;	//ָ��
	tx.head.rand = rand();			// �������ֵ
	memcpy(&tx.cmd_keyboard, &softkeyboard, sizeof(soft_keyboard_t));
	int length = sizeof(cmd_head_t) + sizeof(soft_keyboard_t);
	sendto(sockClientfd, (const char*)&tx, length, 0, (struct sockaddr*)&addrSrv, sizeof(addrSrv));
	SOCKADDR_IN sclient;
	int clen = sizeof(sclient);
	err = recvfrom(sockClientfd, (char*)&rx, 1024, 0, (struct sockaddr*)&sclient, &clen);
	if (err < 0)
		return err_net_rx_timeout;
	return NetRxReturnHandle(&rx, &tx);
}




int kmNet_keyup(int vk_key)
{
	int i;
	if (vk_key >= KEY_LEFTCONTROL && vk_key <= KEY_RIGHT_GUI)//���Ƽ�
	{
		switch (vk_key)
		{
		case KEY_LEFTCONTROL: softkeyboard.ctrl &= ~BIT0; break;
		case KEY_LEFTSHIFT:   softkeyboard.ctrl &= ~BIT1; break;
		case KEY_LEFTALT:     softkeyboard.ctrl &= ~BIT2; break;
		case KEY_LEFT_GUI:    softkeyboard.ctrl &= ~BIT3; break;
		case KEY_RIGHTCONTROL:softkeyboard.ctrl &= ~BIT4; break;
		case KEY_RIGHTSHIFT:  softkeyboard.ctrl &= ~BIT5; break;
		case KEY_RIGHTALT:    softkeyboard.ctrl &= ~BIT6; break;
		case KEY_RIGHT_GUI:   softkeyboard.ctrl &= ~BIT7; break;
		}
	}
	else
	{//�����  
		for (i = 0; i < 10; i++)//���ȼ��������Ƿ����vk_key
		{
			if (softkeyboard.button[i] == vk_key)// ���������Ѿ���vk_key 
			{
				memcpy(&softkeyboard.button[i], &softkeyboard.button[i + 1], 10 - i);
				softkeyboard.button[9] = 0;
				goto KM_up_send;
			}
		}
	}
KM_up_send:
	int err;
	if (sockClientfd <= 0)		return err_creat_socket;
	tx.head.indexpts++;				//ָ��ͳ��ֵ
	tx.head.cmd = cmd_keyboard_all;	//ָ��
	tx.head.rand = rand();			// �������ֵ
	memcpy(&tx.cmd_keyboard, &softkeyboard, sizeof(soft_keyboard_t));
	int length = sizeof(cmd_head_t) + sizeof(soft_keyboard_t);
	sendto(sockClientfd, (const char*)&tx, length, 0, (struct sockaddr*)&addrSrv, sizeof(addrSrv));
	SOCKADDR_IN sclient;
	int clen = sizeof(sclient);
	err = recvfrom(sockClientfd, (char*)&rx, 1024, 0, (struct sockaddr*)&sclient, &clen);
	if (err < 0)
		return err_net_rx_timeout;
	return NetRxReturnHandle(&rx, &tx);
}


//��������
int kmNet_reboot(void)
{
	int err;
	if (sockClientfd <= 0)		return err_creat_socket;
	tx.head.indexpts++;				//ָ��ͳ��ֵ
	tx.head.cmd = cmd_reboot;		//ָ��
	tx.head.rand = rand();			// �������ֵ
	int length = sizeof(cmd_head_t);
	sendto(sockClientfd, (const char*)&tx, length, 0, (struct sockaddr*)&addrSrv, sizeof(addrSrv));
	SOCKADDR_IN sclient;
	int clen = sizeof(sclient);
	err = recvfrom(sockClientfd, (char*)&rx, 1024, 0, (struct sockaddr*)&sclient, &clen);
	WSACleanup();
	sockClientfd = -1;
	if (err < 0)
		return err_net_rx_timeout;
	return NetRxReturnHandle(&rx, &tx);

}



//������������
//static HANDLE handle_listen = NULL;
DWORD WINAPI ThreadListenProcess(LPVOID lpParameter)
{
	WSADATA wsaData; int ret;
	WSAStartup(MAKEWORD(1, 1), &wsaData);			//�����׽��֣�SOCK_DGRAMָ��ʹ�� UDP Э��
	sockMonitorfd = socket(AF_INET, SOCK_DGRAM, 0);	//���׽���
	sockaddr_in servAddr;
	memset(&servAddr, 0, sizeof(servAddr));			//ÿ���ֽڶ���0���
	servAddr.sin_family = PF_INET;				//ʹ��IPv4��ַ
	servAddr.sin_addr.s_addr = INADDR_ANY;	        //�Զ���ȡIP��ַ
	servAddr.sin_port = htons(monitor_port);		//�����˿�
	ret = bind(sockMonitorfd, (SOCKADDR*)&servAddr, sizeof(SOCKADDR));
	SOCKADDR cliAddr;  //�ͻ��˵�ַ��Ϣ
	int nSize = sizeof(SOCKADDR);
	char buff[1024];  //������
	monitor_run = monitor_ok;
	while (1) {
		int ret = recvfrom(sockMonitorfd, buff, 1024, 0, &cliAddr, &nSize);	//����������
		if (ret > 0)
		{
			memcpy(&hw_mouse, buff, sizeof(hw_mouse));							//�������״̬
			memcpy(&hw_keyboard, &buff[sizeof(hw_mouse)], sizeof(hw_keyboard));	//��������״̬
		}
		else
		{
			break;
		}
	}
	monitor_run = 0;
	sockMonitorfd = 0;
	return 0;

}

//ʹ�ܼ�����  �˿ں�ȡֵ��Χ��1024��49151
int kmNet_monitor(short port)
{
	int err;
	if (sockClientfd <= 0)		return err_creat_socket;
	tx.head.indexpts++;				//ָ��ͳ��ֵ
	tx.head.cmd = cmd_monitor;		//ָ��
	if (port) {
		monitor_port = port;				//�Ǹ��˿���������������������
		tx.head.rand = port | 0xaa55 << 16;	// �������ֵ
	}
	else
		tx.head.rand = 0;	// �������ֵ
	int length = sizeof(cmd_head_t);
	sendto(sockClientfd, (const char*)&tx, length, 0, (struct sockaddr*)&addrSrv, sizeof(addrSrv));
	SOCKADDR_IN sclient;
	int clen = sizeof(sclient);
	err = recvfrom(sockClientfd, (char*)&rx, 1024, 0, (struct sockaddr*)&sclient, &clen);
	if (sockMonitorfd > 0)	//�رռ���
	{
		closesocket(sockMonitorfd);
		sockMonitorfd = 0;
	}
	if (port)
	{
		CreateThread(NULL, 0, ThreadListenProcess, NULL, 0, NULL);
	}
	Sleep(10);//����ʱ�����߳�������
	if (err < 0)
		return err_net_rx_timeout;
	return NetRxReturnHandle(&rx, &tx);
}


/*
��������������״̬
����ֵ
-1����δ������������ ��Ҫ�ȵ���kmNet_monitor��1��
0 :�����������ɿ�
1����������������
*/
int kmNet_monitor_mouse_left()
{
	if (monitor_run != monitor_ok) return -1;
	return (hw_mouse.buttons & 0x01) ? 1 : 0;
}


/*//������������м�״̬
����ֵ
-1����δ������������ ��Ҫ�ȵ���kmNet_monitor��1��
0 :��������м��ɿ�
1����������м�����
*/
int kmNet_monitor_mouse_middle()
{
	if (monitor_run != monitor_ok) return -1;
	return (hw_mouse.buttons & 0x04) ? 1 : 0;
}

/*//������������Ҽ�״̬
����ֵ
-1����δ������������ ��Ҫ�ȵ���kmNet_monitor��1��
0 :��������Ҽ��ɿ�
1����������Ҽ�����
*/
int kmNet_monitor_mouse_right()
{
	if (monitor_run != monitor_ok) return -1;
	return (hw_mouse.buttons & 0x02) ? 1 : 0;
}


/*//�������������1״̬
����ֵ
-1����δ������������ ��Ҫ�ȵ���kmNet_monitor��1��
0 :���������1�ɿ�
1�����������1����
*/
int kmNet_monitor_mouse_side1()
{
	if (monitor_run != monitor_ok) return -1;
	return (hw_mouse.buttons & 0x08) ? 1 : 0;
}

/*//�������������2״̬
����ֵ
-1����δ������������ ��Ҫ�ȵ���kmNet_monitor��1��
0 :���������2�ɿ�
1�����������2����
*/
int kmNet_monitor_mouse_side2()
{
	if (monitor_run != monitor_ok) return -1;
	return (hw_mouse.buttons & 0x10) ? 1 : 0;
}



//��������ָ������״̬
int kmNet_monitor_keyboard(short  vkey)
{
	unsigned char vk_key = vkey & 0xff;
	if (monitor_run != monitor_ok) return -1;
	if (vk_key >= KEY_LEFTCONTROL && vk_key <= KEY_RIGHT_GUI)//���Ƽ�
	{
		switch (vk_key)
		{
		case KEY_LEFTCONTROL: return  hw_keyboard.buttons & BIT0 ? 1 : 0;
		case KEY_LEFTSHIFT:   return  hw_keyboard.buttons & BIT1 ? 1 : 0;
		case KEY_LEFTALT:     return  hw_keyboard.buttons & BIT2 ? 1 : 0;
		case KEY_LEFT_GUI:    return  hw_keyboard.buttons & BIT3 ? 1 : 0;
		case KEY_RIGHTCONTROL:return  hw_keyboard.buttons & BIT4 ? 1 : 0;
		case KEY_RIGHTSHIFT:  return  hw_keyboard.buttons & BIT5 ? 1 : 0;
		case KEY_RIGHTALT:    return  hw_keyboard.buttons & BIT6 ? 1 : 0;
		case KEY_RIGHT_GUI:   return  hw_keyboard.buttons & BIT7 ? 1 : 0;
		}
	}
	else//�����
	{
		for (int i = 0; i < 10; i++)
		{
			if (hw_keyboard.data[i] == vk_key)
			{
				return 1;
			}
		}
	}
	return 0;

}


//���������ڲ���ӡ��Ϣ�����͵�ָ���˿ڣ�����ʹ�ã�
int kmNet_debug(short port, char enable)
{
	int err;
	if (sockClientfd <= 0)		return err_creat_socket;
	tx.head.indexpts++;					//ָ��ͳ��ֵ
	tx.head.cmd = cmd_debug;			//ָ��
	tx.head.rand = port | enable << 16;	// �������ֵ
	int length = sizeof(cmd_head_t);
	sendto(sockClientfd, (const char*)&tx, length, 0, (struct sockaddr*)&addrSrv, sizeof(addrSrv));
	SOCKADDR_IN sclient;
	int clen = sizeof(sclient);
	err = recvfrom(sockClientfd, (char*)&rx, 1024, 0, (struct sockaddr*)&sclient, &clen);
	if (err < 0)
		return err_net_rx_timeout;
	return NetRxReturnHandle(&rx, &tx);

}

//���������� 
int kmNet_mask_mouse_left(int enable)
{
	int err;
	if (sockClientfd <= 0)		return err_creat_socket;
	tx.head.indexpts++;					//ָ��ͳ��ֵ
	tx.head.cmd = cmd_mask_mouse;		//ָ��
	tx.head.rand = enable ? (mask_keyboard_mouse_flag |= BIT0) : (mask_keyboard_mouse_flag &= ~BIT0);	// ����������
	int length = sizeof(cmd_head_t);
	sendto(sockClientfd, (const char*)&tx, length, 0, (struct sockaddr*)&addrSrv, sizeof(addrSrv));
	SOCKADDR_IN sclient;
	int clen = sizeof(sclient);
	err = recvfrom(sockClientfd, (char*)&rx, 1024, 0, (struct sockaddr*)&sclient, &clen);
	if (err < 0)
		return err_net_rx_timeout;
	return NetRxReturnHandle(&rx, &tx);
}

//��������Ҽ� 
int kmNet_mask_mouse_right(int enable)
{
	int err;
	if (sockClientfd <= 0)		return err_creat_socket;
	tx.head.indexpts++;					//ָ��ͳ��ֵ
	tx.head.cmd = cmd_mask_mouse;		//ָ��
	tx.head.rand = enable ? (mask_keyboard_mouse_flag |= BIT1) : (mask_keyboard_mouse_flag &= ~BIT1);	// ����������
	int length = sizeof(cmd_head_t);
	sendto(sockClientfd, (const char*)&tx, length, 0, (struct sockaddr*)&addrSrv, sizeof(addrSrv));
	SOCKADDR_IN sclient;
	int clen = sizeof(sclient);
	err = recvfrom(sockClientfd, (char*)&rx, 1024, 0, (struct sockaddr*)&sclient, &clen);
	if (err < 0)
		return err_net_rx_timeout;
	return NetRxReturnHandle(&rx, &tx);
}


//��������м� 
int kmNet_mask_mouse_middle(int enable)
{
	int err;
	if (sockClientfd <= 0)		return err_creat_socket;
	tx.head.indexpts++;					//ָ��ͳ��ֵ
	tx.head.cmd = cmd_mask_mouse;		//ָ��
	tx.head.rand = enable ? (mask_keyboard_mouse_flag |= BIT2) : (mask_keyboard_mouse_flag &= ~BIT2);	// ����������
	int length = sizeof(cmd_head_t);
	sendto(sockClientfd, (const char*)&tx, length, 0, (struct sockaddr*)&addrSrv, sizeof(addrSrv));
	SOCKADDR_IN sclient;
	int clen = sizeof(sclient);
	err = recvfrom(sockClientfd, (char*)&rx, 1024, 0, (struct sockaddr*)&sclient, &clen);
	if (err < 0)
		return err_net_rx_timeout;
	return NetRxReturnHandle(&rx, &tx);
}


//�����������1 
int kmNet_mask_mouse_side1(int enable)
{
	int err;
	if (sockClientfd <= 0)		return err_creat_socket;
	tx.head.indexpts++;					//ָ��ͳ��ֵ
	tx.head.cmd = cmd_mask_mouse;		//ָ��
	tx.head.rand = enable ? (mask_keyboard_mouse_flag |= BIT3) : (mask_keyboard_mouse_flag &= ~BIT3);	// ����������
	int length = sizeof(cmd_head_t);
	sendto(sockClientfd, (const char*)&tx, length, 0, (struct sockaddr*)&addrSrv, sizeof(addrSrv));
	SOCKADDR_IN sclient;
	int clen = sizeof(sclient);
	err = recvfrom(sockClientfd, (char*)&rx, 1024, 0, (struct sockaddr*)&sclient, &clen);
	if (err < 0)
		return err_net_rx_timeout;
	return NetRxReturnHandle(&rx, &tx);
}



//�����������2
int kmNet_mask_mouse_side2(int enable)
{
	int err;
	if (sockClientfd <= 0)		return err_creat_socket;
	tx.head.indexpts++;					//ָ��ͳ��ֵ
	tx.head.cmd = cmd_mask_mouse;		//ָ��
	tx.head.rand = enable ? (mask_keyboard_mouse_flag |= BIT4) : (mask_keyboard_mouse_flag &= ~BIT4);	// ����������
	int length = sizeof(cmd_head_t);
	sendto(sockClientfd, (const char*)&tx, length, 0, (struct sockaddr*)&addrSrv, sizeof(addrSrv));
	SOCKADDR_IN sclient;
	int clen = sizeof(sclient);
	err = recvfrom(sockClientfd, (char*)&rx, 1024, 0, (struct sockaddr*)&sclient, &clen);
	if (err < 0)
		return err_net_rx_timeout;
	return NetRxReturnHandle(&rx, &tx);
}


//�������X������
int kmNet_mask_mouse_x(int enable)
{
	int err;
	if (sockClientfd <= 0)		return err_creat_socket;
	tx.head.indexpts++;					//ָ��ͳ��ֵ
	tx.head.cmd = cmd_mask_mouse;		//ָ��
	tx.head.rand = enable ? (mask_keyboard_mouse_flag |= BIT5) : (mask_keyboard_mouse_flag &= ~BIT5);	// ����������
	int length = sizeof(cmd_head_t);
	sendto(sockClientfd, (const char*)&tx, length, 0, (struct sockaddr*)&addrSrv, sizeof(addrSrv));
	SOCKADDR_IN sclient;
	int clen = sizeof(sclient);
	err = recvfrom(sockClientfd, (char*)&rx, 1024, 0, (struct sockaddr*)&sclient, &clen);
	if (err < 0)
		return err_net_rx_timeout;
	return NetRxReturnHandle(&rx, &tx);
}


//�������y������
int kmNet_mask_mouse_y(int enable)
{
	int err;
	if (sockClientfd <= 0)		return err_creat_socket;
	tx.head.indexpts++;					//ָ��ͳ��ֵ
	tx.head.cmd = cmd_mask_mouse;		//ָ��
	tx.head.rand = enable ? (mask_keyboard_mouse_flag |= BIT6) : (mask_keyboard_mouse_flag &= ~BIT6);	// ����������
	int length = sizeof(cmd_head_t);
	sendto(sockClientfd, (const char*)&tx, length, 0, (struct sockaddr*)&addrSrv, sizeof(addrSrv));
	SOCKADDR_IN sclient;
	int clen = sizeof(sclient);
	err = recvfrom(sockClientfd, (char*)&rx, 1024, 0, (struct sockaddr*)&sclient, &clen);
	if (err < 0)
		return err_net_rx_timeout;
	return NetRxReturnHandle(&rx, &tx);
}

//����������
int kmNet_mask_mouse_wheel(int enable)
{
	int err;
	if (sockClientfd <= 0)		return err_creat_socket;
	tx.head.indexpts++;					//ָ��ͳ��ֵ
	tx.head.cmd = cmd_mask_mouse;		//ָ��
	tx.head.rand = enable ? (mask_keyboard_mouse_flag |= BIT7) : (mask_keyboard_mouse_flag &= ~BIT7);	// ����������
	int length = sizeof(cmd_head_t);
	sendto(sockClientfd, (const char*)&tx, length, 0, (struct sockaddr*)&addrSrv, sizeof(addrSrv));
	SOCKADDR_IN sclient;
	int clen = sizeof(sclient);
	err = recvfrom(sockClientfd, (char*)&rx, 1024, 0, (struct sockaddr*)&sclient, &clen);
	if (err < 0)
		return err_net_rx_timeout;
	return NetRxReturnHandle(&rx, &tx);
}


//���μ���ָ������
int kmNet_mask_keyboard(short vkey)
{
	int err;
	BYTE v_key = vkey & 0xff;
	if (sockClientfd <= 0)		return err_creat_socket;
	tx.head.indexpts++;					//ָ��ͳ��ֵ
	tx.head.cmd = cmd_mask_mouse;		//ָ��
	tx.head.rand = (mask_keyboard_mouse_flag & 0xff) | (v_key << 8);	// ���μ���vkey
	int length = sizeof(cmd_head_t);
	sendto(sockClientfd, (const char*)&tx, length, 0, (struct sockaddr*)&addrSrv, sizeof(addrSrv));
	SOCKADDR_IN sclient;
	int clen = sizeof(sclient);
	err = recvfrom(sockClientfd, (char*)&rx, 1024, 0, (struct sockaddr*)&sclient, &clen);
	if (err < 0)
		return err_net_rx_timeout;
	return NetRxReturnHandle(&rx, &tx);
}


//������μ���ָ������
int kmNet_unmask_keyboard(short vkey)
{
	int err;
	BYTE v_key = vkey & 0xff;
	if (sockClientfd <= 0)		return err_creat_socket;
	tx.head.indexpts++;					//ָ��ͳ��ֵ
	tx.head.cmd = cmd_unmask_all;		//ָ��
	tx.head.rand = (mask_keyboard_mouse_flag & 0xff) | (v_key << 8);	// ���μ���vkey
	int length = sizeof(cmd_head_t);
	sendto(sockClientfd, (const char*)&tx, length, 0, (struct sockaddr*)&addrSrv, sizeof(addrSrv));
	SOCKADDR_IN sclient;
	int clen = sizeof(sclient);
	err = recvfrom(sockClientfd, (char*)&rx, 1024, 0, (struct sockaddr*)&sclient, &clen);
	if (err < 0)
		return err_net_rx_timeout;
	return NetRxReturnHandle(&rx, &tx);
}


//������������Ѿ����õ���������
int kmNet_unmask_all()
{
	int err;
	if (sockClientfd <= 0)		return err_creat_socket;
	tx.head.indexpts++;					//ָ��ͳ��ֵ
	tx.head.cmd = cmd_unmask_all;		//ָ��
	mask_keyboard_mouse_flag = 0;
	tx.head.rand = mask_keyboard_mouse_flag;
	int length = sizeof(cmd_head_t);
	sendto(sockClientfd, (const char*)&tx, length, 0, (struct sockaddr*)&addrSrv, sizeof(addrSrv));
	SOCKADDR_IN sclient;
	int clen = sizeof(sclient);
	err = recvfrom(sockClientfd, (char*)&rx, 1024, 0, (struct sockaddr*)&sclient, &clen);
	if (err < 0)
		return err_net_rx_timeout;
	return NetRxReturnHandle(&rx, &tx);
}



//����������Ϣ  ��IP��˿ں�
int kmNet_setconfig(char* ip, unsigned short port)
{
	int err;
	if (sockClientfd <= 0)		return err_creat_socket;
	tx.head.indexpts++;					//ָ��ͳ��ֵ
	tx.head.cmd = cmd_setconfig;		//ָ��
	tx.head.rand = inet_addr(ip); ;
	tx.u8buff.buff[0] = port >> 8;
	tx.u8buff.buff[1] = port >> 0;
	int length = sizeof(cmd_head_t) + 2;
	sendto(sockClientfd, (const char*)&tx, length, 0, (struct sockaddr*)&addrSrv, sizeof(addrSrv));
	SOCKADDR_IN sclient;
	int clen = sizeof(sclient);
	err = recvfrom(sockClientfd, (char*)&rx, 1024, 0, (struct sockaddr*)&sclient, &clen);
	if (err < 0)
		return err_net_rx_timeout;
	return NetRxReturnHandle(&rx, &tx);
}


//������LCD��Ļ��ָ����ɫ��䡣 ���������ú�ɫ
int kmNet_lcd_color(unsigned short rgb565)
{
	int err;
	if (sockClientfd <= 0)		return err_creat_socket;
	for (int y = 0; y < 40; y++)
	{
		tx.head.indexpts++;		    //ָ��ͳ��ֵ
		tx.head.cmd = cmd_showpic;	//ָ��
		tx.head.rand = 0 | y * 4;
		for (int c = 0; c < 512; c++)
			tx.u16buff.buff[c] = rgb565;
		int length = sizeof(cmd_head_t) + 1024;
		sendto(sockClientfd, (const char*)&tx, length, 0, (struct sockaddr*)&addrSrv, sizeof(addrSrv));
		SOCKADDR_IN sclient;
		int clen = sizeof(sclient);
		err = recvfrom(sockClientfd, (char*)&rx, length, 0, (struct sockaddr*)&sclient, &clen);
		if (err < 0)
			return err_net_rx_timeout;
	}
	return NetRxReturnHandle(&rx, &tx);

}

//�ڵײ���ʾһ��128x80��ͼƬ
int kmNet_lcd_picture_bottom(unsigned char* buff_128_80)
{
	int err;
	if (sockClientfd <= 0)		return err_creat_socket;
	for (int y = 0; y < 20; y++)
	{
		tx.head.indexpts++;		    //ָ��ͳ��ֵ
		tx.head.cmd = cmd_showpic;	//ָ��
		tx.head.rand = 80 + y * 4;
		memcpy(tx.u8buff.buff, &buff_128_80[y * 1024], 1024);
		int length = sizeof(cmd_head_t) + 1024;
		sendto(sockClientfd, (const char*)&tx, length, 0, (struct sockaddr*)&addrSrv, sizeof(addrSrv));
		SOCKADDR_IN sclient;
		int clen = sizeof(sclient);
		err = recvfrom(sockClientfd, (char*)&rx, length, 0, (struct sockaddr*)&sclient, &clen);
		if (err < 0)
			return err_net_rx_timeout;
	}
	return NetRxReturnHandle(&rx, &tx);
}

//�ڵײ���ʾһ��128x160��ͼƬ
int kmNet_lcd_picture(unsigned char* buff_128_160)
{
	int err;
	if (sockClientfd <= 0)		return err_creat_socket;
	for (int y = 0; y < 40; y++)
	{
		tx.head.indexpts++;		    //ָ��ͳ��ֵ
		tx.head.cmd = cmd_showpic;	//ָ��
		tx.head.rand = y * 4;
		memcpy(tx.u8buff.buff, &buff_128_160[y * 1024], 1024);
		int length = sizeof(cmd_head_t) + 1024;
		sendto(sockClientfd, (const char*)&tx, length, 0, (struct sockaddr*)&addrSrv, sizeof(addrSrv));
		SOCKADDR_IN sclient;
		int clen = sizeof(sclient);
		err = recvfrom(sockClientfd, (char*)&rx, length, 0, (struct sockaddr*)&sclient, &clen);
		if (err < 0)
			return err_net_rx_timeout;
	}
	return NetRxReturnHandle(&rx, &tx);
}