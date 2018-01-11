#include "napt66_hash_table.h"

extern struct hash_entry source_table[HASHRANGE];
extern struct hash_entry ipproto_table[HASHRANGE];
extern long time(void* ptr);

void hash_table_init(struct hash_entry* table)
{
	int i;
	printk(KERN_INFO "hash_table_init()\n");	
	for(i = 0; i < HASHRANGE; i++){
		table[i].next = NULL;
		table[i].prev = NULL;
		table[i].conntrack_entry = NULL;
	}	
}

//使用elf算法参数哈希值，elf算法用于根据字符串产生哈希值，把key按照协议、地址、端口组成一个字符串，经过运算后得到哈希值。
int hash(int proto,struct in6_addr ipv6,u_int16_t id)
{    
	 unsigned int hash = 0;
	 int i;
	 for(i = 0; i < 16; i++){
	 		hash = hash + ipv6.s6_addr[i];
	 }
	 hash = hash + proto + id;
    return hash%HASHNUMBER;
}

//将连接项在两张表的位置都删除
int hash_del_entry(struct conn_entry* p_entry)
{
	struct hash_entry *pre_entry = p_entry->source->prev;
	struct hash_entry *next_entry = p_entry->source->next;
	//删除source_table中的链节
	
	if(next_entry==NULL){//头结点后的第一个链节特殊处理
		pre_entry->next=NULL;
	}
	else{
		pre_entry->next = next_entry;
		next_entry->prev = pre_entry; 			
	}
	kfree(p_entry->source);//释放内存
	//删除ipproto_table中的链节
	pre_entry = p_entry->ipproto->prev;
	next_entry = p_entry->ipproto->next;
	if(next_entry==NULL){//头结点后的第一个链节特殊处理
		pre_entry->next=NULL;
	}
	else{
		pre_entry->next = next_entry;
		next_entry->prev = pre_entry;
	}
	kfree(p_entry->ipproto);//释放内存
	return 0;
}

//清理链表中的过期连接，参数是空的哈希头节点，不包含连接信息
int hash_clean_entry(struct hash_entry* p_hash_entry)
{
	struct conn_entry* ct_entry;
	int cur_time = time(NULL);
	
	p_hash_entry = p_hash_entry->next;//跳过空的头节点
	
	while(p_hash_entry != NULL){
		ct_entry = p_hash_entry->conntrack_entry;
		p_hash_entry = p_hash_entry->next;

		if(cur_time - ct_entry->time > CLEANUP_TIME){//超时
			hash_del_entry(ct_entry);
			kfree(ct_entry);
		}
	}
	return 0;
}

//搜索到则返回指针，否则返回NULL
struct conn_entry* hash_search_ct(int direc,struct conn_entry* p_entry)
{
	int place;
	struct hash_entry *tmp;
	if(direc == SNAT){//source	SNAT
		place = hash(p_entry->proto , p_entry->lan_ipv6 , p_entry->lan_id);//得到哈希值
		tmp = source_table[place].next;//搜索哈希值对应位置的链表
		while(tmp != NULL){
			if(tmp->conntrack_entry->proto == p_entry->proto &&
				tmp->conntrack_entry->lan_id == p_entry->lan_id &&
				0 == memcmp(&(tmp->conntrack_entry->lan_ipv6),&(p_entry->lan_ipv6),sizeof(struct in6_addr)))
				{
				return tmp->conntrack_entry;//返回搜索结果
			}
			tmp = tmp->next;
	 	 }
	}
	else if(direc == RSNAT){//ipproto	RSNAT
		place = hash(p_entry->proto , p_entry->wan_ipv6 , p_entry->wan_id);//得到哈希值
		tmp = ipproto_table[place].next;//搜索哈希值对应位置的链表
		while(tmp != NULL){
		if(tmp->conntrack_entry->proto == p_entry->proto &&
			tmp->conntrack_entry->wan_id == p_entry->wan_id &&
			0 == memcmp(&(tmp->conntrack_entry->wan_ipv6),&(p_entry->wan_ipv6),sizeof(struct in6_addr)))
			return (*tmp).conntrack_entry;//返回搜索结果
		tmp = tmp->next;
		}
	}	
	return NULL;
}

//在两张hash表中插入ct项
int hash_add_entry(struct conn_entry* p_entry)
{
	//把实体地址分别保存到两个新的链节中，然后把链节添加到对应的表中。
	int source_place,ipproto_place;
	struct hash_entry *source_entry = (struct hash_entry *)kcalloc(1,sizeof (struct hash_entry),GFP_KERNEL);
	struct hash_entry *ipproto_entry = (struct hash_entry *)kcalloc(1,sizeof (struct hash_entry),GFP_KERNEL);
	source_entry->conntrack_entry = p_entry;//保存实体地址到链节
	ipproto_entry->conntrack_entry = p_entry;//保存实体地址到链节
	p_entry->source = source_entry;//保存链节地址到实体
	p_entry->ipproto = ipproto_entry;//保存链节地址到实体
	//添加到source_table
	source_place  =  hash(p_entry->proto , p_entry->lan_ipv6 , p_entry->lan_id);//宏直接简化为lan_id

	hash_clean_entry(&source_table[source_place]);//clean应是对称的，执行一次就够。
	if (source_table[source_place].next==NULL){//如果是第一个新增节点
	 	 source_table[source_place].next=source_entry;
	 	 source_entry->next=NULL;
	 	 source_entry->prev=&source_table[source_place];
	 }
	else{
		 source_entry->next = source_table[source_place].next;
		 source_entry->next->prev=source_entry;
		 source_table[source_place].next = source_entry;
		 source_entry->prev = &source_table[source_place];
	}
	//添加到ipproto_table
	ipproto_place =  hash(p_entry->proto , p_entry->wan_ipv6 , p_entry->wan_id);	
	hash_clean_entry(&ipproto_table[ipproto_place]);	
	if (ipproto_table[ipproto_place].next==NULL){//如果是第一个新增节点
	 	 ipproto_table[ipproto_place].next=ipproto_entry;
	 	 ipproto_entry->next=NULL;
	 	 ipproto_entry->prev=&ipproto_table[ipproto_place];
	 }
	else{
		 ipproto_entry->next = ipproto_table[ipproto_place].next;
		 ipproto_entry->next->prev=ipproto_entry;
	 	 ipproto_table[ipproto_place].next = ipproto_entry;
		 ipproto_entry->prev = &ipproto_table[ipproto_place];
	}	
	return 1;
}
