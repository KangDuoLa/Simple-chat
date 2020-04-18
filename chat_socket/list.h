#ifndef LIST_H_
#define LIST_H_

typedef struct node *mylink;
struct node 
{
	char item[20];  //客户端名字
	int fd;		
	mylink next;
};


mylink make_node(char *name, int fd);						
void mylink_insert(mylink *head, mylink p);
mylink mylink_search(mylink *head, char *keyname);
void mylink_delete(mylink *head, mylink p);
void free_node(mylink p);
void mylink_destory(mylink *head);
void mylink_travel(mylink *head, void (*vist)(mylink));

#endif