#ifndef CHAT_H__
#define CHAT_H__

typedef struct qq_chat
{
	int protocal_num;
	char srcname[32];
	char desname[32];
	char content[128];
}Chat;
/*
 *	协议号		  发送者名     目的者名    内容
 *	protocal_num   srcname     desname     content
 * 	  1            登录者       NULL
 *    2            发送方       接收方     数据
 *    3            NULL(不在线)
 *    4            退出登录用户（退出登录）                   
*/


#endif