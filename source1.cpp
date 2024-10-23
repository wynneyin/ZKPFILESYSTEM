#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<string>
using namespace std;

const int DISK_MAXLEN = 2560;
const int dirtable_max_size = 80;
const int fatnum = 80;
char Disk[DISK_MAXLEN];
int dbr[2];
int startindex;
int flag1 = 0;


void init();
void createfile(char filename[], int filesize, int type);
void adddirunit(char fileName[], int type);
int findunit(char filename[]);
void ls();
void deletefile(char filename[]);
void deleteunit(int unitindex);
short findfreefat();
void freefat(char filename[]);
void changedir(char dirname[]);
void read(char filename[], int length);
void write(char filename[], string content);
void adjustfat(short num, int unitindex);

struct fcb {
	short blockindex;//which is block
	short filesize;
	short datasize;
};

struct dirunit {
	char filename[80];
	char type;
	short startfat;
	short startfcb;
	short startdir;
};

struct dirtable {
	int dirunitnum;
	dirunit dirs[dirtable_max_size];
};

short fat[fatnum];
//free:0 last:-1 next:id
dirtable* table[10];
fcb* FCB[fatnum];

dirtable rootdirtable;
dirtable* currentdirtable;
char path[100] = "C:\\root\\";

void init() {
	rootdirtable.dirunitnum = 0;
	currentdirtable = &rootdirtable;
	//8B has been allocated
	dbr[0] = fatnum;
	dbr[1] = dirtable_max_size;
	//B_num that allocated to fat table
	int fat_B = fatnum * sizeof(short);
	//calculate startindex of data
	startindex = 8 + fatnum;
}

/*
struct fcb{
	int blockindex;
	int filesize;
	int datasize;
};*/

int FCBrecord = 0; //record current index of  fcb when creating
int TABrecord = 0; //record current index of  dir table when creating
void createfile(char filename[], int filesize, int type) {
	if (strlen(filename) > 80) {
		cout << "file name is too long" << endl;
		return;
	}
	//add dir
	adddirunit(filename, type);
	int index = findunit(filename);
	//create fcb
	fcb* curfcb = (fcb*)new fcb();
	curfcb->blockindex = startindex++;
	curfcb->filesize = filesize;
	curfcb->datasize = 0;//nothing is writed
	FCB[FCBrecord] = curfcb;
	currentdirtable->dirs[index].startfcb = FCBrecord;
	//create fat
	int i = findfreefat();
	currentdirtable->dirs[index].startfat = i;
	fat[i] = -1;
	//create dir
	if (type == 0)
	{
		dirtable* curdirtable = (dirtable*)new dirtable();
		table[TABrecord] = curdirtable;
		curdirtable->dirunitnum = 0;
		currentdirtable->dirs[index].startdir = TABrecord;
	}
}

/*
struct dirunit{
	char filename[80];
	char type;
	int startfat;
};

struct dirtable{
	int dirunitnum;
	dirunit dirs[dirtable_max_size];
};*/

void adddirunit(char filename[], int type)
{
	int dirunitnum = currentdirtable->dirunitnum;
	//whether is full
	if (dirunitnum == dirtable_max_size)
	{
		cout << "dirTables is full, try to delete some file\n";
		return;
	}

	//whether is existed
	if (findunit(filename) != -1)
	{
		printf("file already exist\n");
		return;
	}
	//creater new dirunit
	dirunit* newdirunit = &currentdirtable->dirs[dirunitnum];
	currentdirtable->dirunitnum++;
	int i = strlen(filename);
	while (i--)
		newdirunit->filename[i] = filename[i];
	newdirunit->type = type;

	return;
}

int findunit(char filename[])
{
	//look up in order
	int dirunitnum = currentdirtable->dirunitnum;
	int unitIndex = -1;
	for (int i = 0; i < dirunitnum; i++)
		if (strcmp(filename, currentdirtable->dirs[i].filename) == 0)
			unitIndex = i;
	return unitIndex;  //如果找到改文件返回索引
}

void ls() {
	int uninum = currentdirtable->dirunitnum;
	for (int i = 0; i < uninum; i++) {
		dirunit curunit = currentdirtable->dirs[i];
		cout << curunit.filename << " ";
	}
	cout << endl;
}

void deletefile(char filename[]) {
	int unitindex = findunit(filename);
	if (unitindex == -1) {
		cout << "sorry,not found" << endl;
		return;
	}
	//delete unit in the table
	deleteunit(unitindex);
	freefat(filename);
}

void deleteunit(int unitindex) {
	//let the next item covers 
	int dirunitnum = currentdirtable->dirunitnum;
	for (int i = unitindex; i < dirunitnum - 1; i++) {
		currentdirtable->dirs[i] = currentdirtable->dirs[i + 1];
	}
	currentdirtable->dirunitnum--;
}

short findfreefat() {
	for (short i = 0; i < fatnum; i++) {
		if (fat[i] == 0)return i;
	}
}

void freefat(char filename[]) {
	//find the link in fat and free each of it
	int i = currentdirtable->dirs[findunit(filename)].startfat;
	if (i == -1) {
		fat[i] = 0;
		return;
	}
	while (i != -1) {
		int temp = i;
		i = fat[i];
		fat[temp] = 0;
	}
	if (i == -1) {
		fat[i] = 0;
		return;
	}
}

void changedir(char dirname[]) {
	//see whether the name is valid
	int unitindex = findunit(dirname);
	
	flag1 = unitindex;
	if (unitindex == -1) {
		cout << "sorry,not found" << endl;
		return;
	}
	if (currentdirtable->dirs[unitindex].type == 1) {
		cout << "not a dir" << endl;
		return;
	}
	//change currentdir
	int i = currentdirtable->dirs[unitindex].startdir;
	currentdirtable = table[i];
	//change path
	if (strcmp(dirname, "..") == 0) {
		//reback
		int length = strlen(path);
		for (int i = length - 2; i >= 0; i--) {
			if (path[i] == '\\') {
				path[i + 1] = '\0';
				break;
			}
		}
	}
	else {
		strcat_s(path, dirname);
		strcat_s(path, "\\");
	}
}

void read(char filename[], int length) {
	int unitindex = findunit(filename);
	if (unitindex == -1) {
		cout << "sorry,not found" << endl;
		return;
	}

	//read the data of given length 
	int index = currentdirtable->dirs[unitindex].startfcb;
	fcb* myfcb = FCB[index];
	for (int i = 0; i < length; i++) {
		cout << Disk[i + myfcb->blockindex];
	}
	cout << endl;
}

void write(char filename[], string content) {
	int unitindex = findunit(filename);
	if (unitindex == -1) {
		cout << "sorry,not found" << endl;
		return;
	}
	
	
	int length;
	for (length = 0; content[length] != '\0'; length++);
	
	cout << length;
	//how many clusters need
	int num = (length % 32 == 0) ? length / 32 : length / 32 + 1;
	adjustfat(num, unitindex);

	//renew the filesize
	FCB[currentdirtable->dirs[unitindex].startfcb]->filesize = num;

	//get the start index and write the content in order
	int index = currentdirtable->dirs[unitindex].startfcb;
	fcb* myfcb = FCB[index];
	for (int i = 0; i < length; i++) {
		Disk[i + myfcb->blockindex] = content[i];
	}
	cout << endl;

}

void adjustfat(short num, int unitindex) {
	int index = currentdirtable->dirs[unitindex].startfat;
	for (int i = 0; i < num - 1; i++) {
		short j = findfreefat();
		fat[index] = j;
		index = j;
	}
	fat[index] = -1;
}

void open(char filename[])
{





	for (int i = 0; i < 35; i++)
		cout << '*';
	cout << endl;
	cout << "open correctly:you can write and read file" << endl;
	for (int i = 0; i < 35; i++)
		cout << '*';
	cout << endl;
}







int main() {
	init();
	string s;
	char name[80] = { 0 };
	char content[100] = { 0 };
	int length = 0;
	for (int i = 0; i < 35; i++)
		cout << '*';
	cout << endl;
	cout << "you can do the operation as follows" << endl;
	cout << "1.mkdir+dirname\n2.vi+filename\n3.ls\n4.cd+dirname\n5.read+filename+length\n6.write+filename+data\n7.delete+filename\n8.quit" << endl;
	for (int i = 0; i < 35; i++)
		cout << '*';
	cout << endl;
	while (1) {
		int i = strlen(path);
		int j = 0;
		while (j < i) {
			cout << path[j];
			j++;
		}
		cout << '>';
		char operation[5];
		char op = getchar();
		if (op == '\n')continue;
		cin >> s;
		memcpy(operation, s.c_str(), s.length());
		switch (op) {
		case 'q':return 0;
		case 'm':
			cin >> s;
			memcpy(name, s.c_str(), s.length());
			createfile(name, 1, 0);
			getchar();
			break;
		case 'v':
			cin >> s;
			memcpy(name, s.c_str(), s.length());
			createfile(name, 1, 1);
			getchar();
			break;
		case 'l':
			ls(); getchar(); break;
		case 'c':
			cin >> s;
			memcpy(name, s.c_str(), s.length());
			changedir(name);
			getchar();
			break;
		case 'r':
			cin >> s;
			memcpy(name, s.c_str(), s.length());
			scanf_s("%d", &length);
			read(name, length);
			getchar();
			break;
		case 'w':
			cin >> s;
			memcpy(name, s.c_str(), s.length());
			cin >> s;
			memcpy(content, s.c_str(), s.length());
			write(name, content);
			memset(content, 0, sizeof(content));
			getchar();
			break;
		case 'd':
			cin >> s;
			memcpy(name, s.c_str(), s.length());
			deletefile(name);
			getchar();
			break;
		case 'o':
			cin >> s;
			int index = 0;
			int i = 0;
			char temp[80] = {'0'};
			while (1)
			{
				if (s[index] == '\\')  //遇到分隔符就cd
				{
					//memcpy(name, temp.c_str(), temp.length());
					changedir(temp);
					memset(temp, 0, sizeof(temp));

					i = 0;
				}
				temp[i] = s[index];

				if (index == s.length()-1) ///扫描到尾部就停下
				{
					open(temp);
					memset(temp, 0, sizeof(temp));
					break;
				}

				index++;
				i++;
				
			}


			if (flag1 ==0)
			{
				cout << "打开完毕";		
				getchar();
			}
			
		}
	}
}



