#include<stdio.h>
#include<iostream>
#include<stdlib.h>
#include<cstring>

using namespace std;


#define JSON_STR_LENGTH  5000
#define T_JSON 1
#define T_JSON_ARRAY 2
#define T_INT 3
#define T_STR 4
#define T_EXCEPTION 5
#define T_BRACE_LEFT 6
#define T_BRACE_RIGHT 7
#define T_COLON=8;
#define T_BRACKET_LEFT=9;
#define T_BRACKET_RIGHT=10;
#define T_COMMA =11;
#define T_D_QUOTES=12;
void skip_white(char* s,char **addr);
typedef struct List_TYPE
{
	void* val;
	int type;
	struct List_TYPE* next;
}List;

//创建一个空表 表头是为空 是一个岗哨
List* createList(){
	List* list= (List*)malloc(sizeof(List));
	list->next=NULL;
	list->val=NULL;
	return list;
}

List* lastElement(List* list){
	while(list->next!=NULL){
		list=list->next;
	}
	return list;
}

List* insertList(List* list,void* val){
	List* node= (List*)malloc(sizeof(List));
	node->val = val;
	node->next=NULL;


	List* last =lastElement(list);
	last->next = node;
	
	
	return list;
}

List* insertList(List* list,void* val,int type){
	List* node= (List*)malloc(sizeof(List));
	node->val = val;
	node->next=NULL;
	node->type=type;

	List* last =lastElement(list);
	last->next = node;
	return list;
}

List* removeElementFromList(List* list){
	List* tmp;
	if(list->next ==NULL)return NULL;
	else{
		tmp = list->next;
		list->next=list->next->next;
	}

	return tmp;
}


typedef struct JSON_TYPE
{
	int len;		// 节点长度
	char** keys;	// Json中包含的键集合
	void** vals;	// Json中包含的值集合 指向的可能是JSON_TYPE
	int* types;		// 当前val种类,包括 str,json,jsonarray,int,bool(回头加)	
}JSON;

typedef JSON* JSON_PTR;

char jsonstr[500000];

bool isspace(char c){
	if(c==' '||c=='\t'||c=='\n')return true;
	else return false;
}


void trim(char *s){
	char*p = s;
	for(char *t=s;*t!='\0';t++){
		if(!isspace(*t)){
			*s++=*t;		
		}
	}
	*s='\0';
	s=p;
	printf("%s\n", p);
}


/*
	key => "key":"val"
*/
char* parseKey(char* s,char **addr){
	char* p=s;
	/*简单处理 没考虑" "配对问题*/
	if(*p!='\"'){
		return NULL;
	}
	p++;
	
	while( ( *p!='\"' || (*(p-1)=='\\'&&*p=='\"'))&& *(p)!='\0')p++;
	int len = p-s+1-2;
	char* key = (char*)malloc( (len+1)*sizeof(char) );

	for(int i = 0 ;i < len;i++){
		key[i]=*(s+1+i);
	}
	key[len]='\0';
	*addr = (s+2+len);
	return key;
}

/*
	val : 123123123 
	val : {xxxxx:xxx}
	val : []
	val : "asldjlkjwelk"
*/

char* parseInt(char*s,char**addr){
	char* p = s;
	if(*p<'0' || *p >'9')return NULL;
	int len = 0;
	while(*p>='0'&&*p<='9'){
		*p++;
		len++;
	}
	char* num = (char*)malloc((len+1)*sizeof(char));
	for(int i=0;i<len;i++){
		num[i]=*(s+i);
	}
	num[len]='\0';
	*addr=(s+len);
	return num;
}	
JSON* parseJson(char *s,char** addr);

JSON** parseJsonArray(char*s,char**addr){
	char* p = s;

	skip_white(p,addr);
	p=*addr;

	if(*p!='[')return NULL;
	else{
		p++; //跳过[
					
	}
	
	
	
	List* json_list=createList();
	int cnt=0;
	do
	{
		*addr=p;
		JSON* json = parseJson(p,addr);
		p=*addr;

		insertList(json_list,(void*)json,T_JSON);
		cnt++;
		if(*p==','){
			p++;
			skip_white(p,addr);
			p=*addr;
		}else if(*p == ']'){
			p++;
			*addr=p;
			break;
		}
	} while (*p!='\0');
	
	JSON** jsonarray =(JSON**) malloc(cnt*sizeof(JSON*));
	for(int i=0 ;i< cnt;i++){
		List* val_e = removeElementFromList(json_list);
		jsonarray[i]=(JSON*)val_e->val;
	}
	return jsonarray;
}

void* parseVal(char* s,char** addr,int* type){
	char*p = s;
	char c = *p;
	void* val=NULL;
	if(c>='0' && c <='9'){
		*type=T_INT;
		val =(void*) parseInt(p,addr);
		p=*addr;
	}
	else if(c=='\"'){
		*type=T_STR;
		val = (void*)parseKey(p,addr);
		p=*addr;
	}
	else if(c=='{'){
		*type= T_JSON;
		val = (void*)parseJson(p,addr);
		p=*addr;
	}
	else if(c=='['){
		*type= T_JSON_ARRAY;
		val = (void*)parseJsonArray(p,addr);	
		p=*addr;
	}
	else{
		*type =0;
		val=NULL;
		*addr=p+4;
		p=*addr;
	}

	skip_white(p,addr);
	p=*addr;

	return val;
}

/**
JSON解析函数
对原始字符串进行parse
返回JSON
*/

JSON* parseJson(char *s,char** addr){
	char* p = s;

	skip_white(p,addr);
	p=*addr;

	if(*p!='{')return NULL;
	else{
		p++;
		skip_white(p,addr);
		p=*addr;
	}
	
	JSON* json = (JSON*)malloc(sizeof(JSON));
	List* key_list=createList();
	List* val_list=createList();
	int cnt=0;
	do
	{
		*addr=p;
		char *key = parseKey(p,addr);
		printf("key: %s ", key);
		p=*addr;
		//printf("p:%d &p:%d\n", p,addr);
		if(key == NULL)break;
		
		skip_white(p,addr);
		p=*addr;

		if(*p == ':'){
			p++;
			skip_white(p,addr);
			p=*addr;
		}else{
			printf(":解析失败\n");
		}

		insertList(key_list,key,-1);

		int type=-1;
		*addr=p;
		void* val = parseVal(p,addr,&type);
		p=*addr;

		
		
		printf("type: %d val: %s\n",type, (char*)val);

		if(type==-1){
			printf("type解析出错!\n");
			return NULL;
		}

		insertList(val_list,val,type);
		cnt++;

		
		if(*p==','){
			p++;
			skip_white(p,addr);
			p=*addr;
		}else if(*p=='}'){
			p++;
			*addr=p;

			break;
		}
		
	} while (*s!='\0');

	json->len = cnt;
	json->keys= (char**)malloc(cnt*sizeof(char*));
	json->vals= (void**)malloc(cnt*sizeof(void*));
	json->types= (int*)malloc(cnt*sizeof(int));
	
	for(int i = 0 ;i<cnt;i++){
		List* key_e = removeElementFromList(key_list);
		List* val_e = removeElementFromList(val_list);
		json->keys[i]=(char*)key_e->val;
		json->vals[i]=(void*)val_e->val;
		json->types[i]=val_e->type;
	}

	return json;

}
void skip_white(char* s,char **addr){
	while(isspace(*s)){
		s++;
	}
	*addr=s;
}

void print_json(JSON* json,int dep){
//	printf("%d\n", json->len);

	for(int i = 0 ;i< json->len;i++){
		for(int j= 0;j<dep;j++)printf("\t");
		printf("key: %s\tval: %s\n", json->keys[i],json->vals[i]); 
		if(json->types[i]==T_JSON){
			print_json((JSON*)json->vals[i],dep+1);
		}else if(json->types[i] == T_JSON_ARRAY){
			JSON** val = (JSON**)json->vals[i];
			while(*val!=NULL){
				print_json((JSON*)(*val),dep+1);
				val++;
			}
		}
	}
}
int main(){

	char c;
	int index=0;
	
	while((c= getchar())!=EOF){
		jsonstr[index++] = c;
	}
	jsonstr[index]='\0';
	//trim(jsonstr);
	char *s=jsonstr;
	JSON* json= parseJson(s,&s);
	printf("\n\n");
	print_json(json,0);
	// while(*json!=NULL){
	// 	print_json((JSON*)(*json),0);
	// 	json++;
	// }
	

}