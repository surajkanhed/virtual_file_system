/*
	CUSTOMISED VRTUAL FILE SYSTEM
	
	Description :
	
			• This project provides all functionality to the user which is same as Linux File system.
			• It provides necessary commands, system calls implementations of file system through
			  customized shell.
			• In this project we implement all necessary data structures of file system like Incore Inode
			  Table, File Table, UAREA, User File Descriptor table.

*/


#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>


//*********************************

#define MAXINODE 50
#define MAXFILESIZE 1024

#define READ 1
#define WRITE 2

#define REGULAR 1
#define SPECIAL 2

#define START 0
#define CURRENT 1
#define END 2

//***********************************

typedef struct superblock
{
	int Totalinode;
	int Freeinode;
	
}SUPERBLOCK,*PSUPERBLOCk;

//**********************************

typedef struct inode
{
	char FileName[50];
	int InodeNumber;
	int FileSize;
	int ActualFileSize;
	int FileType;
	char *Buffer;
	int LinkCount;
	int ReferenceCount;
	int permission;
	struct inode *next;
}INODE,*PINODE,**PPINODE;

//************************************

typedef struct filetable
{
	int readoffset;
	int writeoffset;
	int count;
	int mode;
	PINODE ptrinode;
}FILETABLE,*PFILETABLE;

//************************************

typedef struct ufdt
{
	PFILETABLE ptrfiletable;
}UFDT;

//***********************************

UFDT UFDTArr[50];
SUPERBLOCK SUPERBLOCKobj;
PINODE Head=NULL;


//************************************

void man(char *name)
{
	if (name==NULL)
	{
		return;
	}
	if(strcmp(name,"create")==0)
	{
		printf("Description : used to create new regular file \n");
		printf("Usage : Create file name permission\n");
	}
	else if(strcmp(name,"read")==0)
	{
		printf("Description : use to read data from regular file\n");
		printf("Usage : read File_name No_of_Byte_to_Read \n");
	}
	else if(strcmp(name,"write")==0)
	{
		printf("Description : use to write into regular file \n");
		printf("Usage : write file name \n After this enter the data that we want to write \n");
	}
	else if(strcmp(name,"ls")==0)
	{
		printf("Description : use to list all the information of all the file \n");
		printf("Usage : ls \n");
	}
	else if(strcmp(name,"stat")==0)
	{
		printf("Description : use to display all the information of the file  \n");
		printf("Usage : stat file_name \n");
	}
	else if(strcmp(name,"fstat")==0)
	{
		printf("Description : use to display information of file \n");
		printf("Usage : fstat file_descriptor \n");
	}
	else if(strcmp(name,"truncate")==0)
	{
		printf("Description : use to remove data from file \n");
		printf("Usage : truncate file_name \n");
	}
	else if(strcmp(name,"open")==0)
	{
		printf("Description : use to open existing file \n");
		printf("Usage : open file_name mode \n");
	}
	else if(strcmp(name,"close")==0)
	{
		printf("Description : used to close opened file \n");
		printf("Usage : close file_name \n");
	}
	else if(strcmp(name,"closeall")==0)
	{
		printf("Description : use to cloase all opened file \n");
		printf("Usage : closeall \n");
	}
	else if(strcmp(name,"lseek")==0)
	{
		printf("Description : use to changed file offset \n");
		printf("Usage : lseek file_name ChangeInOffset StartPoint \n");
	}
	else if(strcmp(name,"rm")==0)
	{
		printf("Description : Used to delete the file \n");
		printf("Usage : rm file_name \n");
	}
	else
	{
		printf("ERROR : No manual entry available \n");
	}
}

//******************************************************

void DisplayHelp()
{
	printf("ls : To list out all file \n");
	printf("clear :  To clear consol \n");
	printf("open : To open the file \n");
	printf("close : To close the file \n");
	printf("closeall : To close all opened file \n");
	printf("read : To read the contents from the file \n");
	printf("write : To write contents into file \n");
	printf("exit : To terminate the file system project \n");
	printf("stat : To display information of file using the file name \n");
	printf("fstat : To display the information of the file using the file descriptor \n");
	printf("truncate : To remove the all data from the file  \n");
	printf("rm : To delete the file \n");	
}
//*******************************************************************


int GetFDFromName(char *name)
{
	int i=0;
	while(i < MAXINODE)
	{
		if(UFDTArr[i].ptrfiletable != NULL)
		{
			if(strcmp((UFDTArr[i].ptrfiletable -> ptrinode->FileName),name)==0)
			{
				break;
			}
		}
		i++;
	}
	if(i==50)
	{
		return -1;
	}
	else
	{
		return i;
	}
}

//*************************************

PINODE Get_Inode(char *name)
{
	PINODE temp=Head;
	int i=0;
	if(name==NULL)
	{
		return NULL;
	}
	while(temp != NULL)
	{
		if(strcmp(name,temp->FileName)==0)
		{
			break;
		}
		temp=temp->next;
	}
	return temp;
}

//*******************************
void CreateDILB()
{
	PINODE newn=NULL;
	PINODE temp=Head;
	int i=0;
	while(i<MAXINODE)
	{
		newn=(PINODE)malloc(sizeof(INODE));
		
		newn->LinkCount=0;
		newn->ReferenceCount=0;
		newn->FileType=0;
		newn->FileSize=0;
		newn->Buffer=NULL;
		newn->next=NULL;
		newn->InodeNumber=i;
		if(temp==NULL)
		{
			Head=newn;
			temp=Head;
		}
		else
		{
			temp->next=newn;
			temp=temp->next;
		}
		i++;
	}
	printf("DILB created successfully\n");
}

//*******************************************

void InitialiseSuperBlock()
{
	int i=0;
	while(i<50)
	{
		UFDTArr[i].ptrfiletable=NULL;
		i++;
	}
	SUPERBLOCKobj.Totalinode=MAXINODE;
	SUPERBLOCKobj.Freeinode=MAXINODE;
}


//*******************************************


int CreateFile(char *name,int permission)
{
	int i=0;
	PINODE temp=Head;
	if((name==NULL) || (permission ==0) || (permission>3))
	//if file name is not given or invalide permission given then return with -1.
	{
		return -1;
	}
	if(SUPERBLOCKobj.Freeinode == 0)  
	//if freeinode is not present on the disk. then return with -2
	{
		return -2;
	}
	(SUPERBLOCKobj.Freeinode)--;
	
	if(Get_Inode(name) != NULL) 
	//when that name of file is already exist then retrun with -3.
	{
		return -3;
	}
	while(temp != NULL)
	{
		if(temp->FileType == 0)
		{	
			break;
		}
		temp=temp->next;
	}
	while(i < MAXINODE)
	{
		if(UFDTArr[i].ptrfiletable == NULL)
		{
			break;
		}
		i++;
	}
	UFDTArr[i].ptrfiletable=(PFILETABLE)malloc(sizeof(FILETABLE));
	if((UFDTArr[i].ptrfiletable) == NULL)  
	//if unable to allocate the memory.
	{
		return -4;
	}
	UFDTArr[i].ptrfiletable->count=1;
	UFDTArr[i].ptrfiletable->mode=permission;
	UFDTArr[i].ptrfiletable->readoffset=0;
	UFDTArr[i].ptrfiletable->writeoffset=0;
	
	UFDTArr[i].ptrfiletable->ptrinode=temp;
	strcpy(UFDTArr[i].ptrfiletable->ptrinode->FileName,name) ;
	//copy name of the file in that inode stucture.
	UFDTArr[i].ptrfiletable->ptrinode->FileSize=MAXFILESIZE;
	UFDTArr[i].ptrfiletable->ptrinode->ActualFileSize = 0;
	UFDTArr[i].ptrfiletable->ptrinode->FileType=REGULAR;
	UFDTArr[i].ptrfiletable->ptrinode->LinkCount=1;
	UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount=1;
	UFDTArr[i].ptrfiletable->ptrinode->permission=permission;
	UFDTArr[i].ptrfiletable->ptrinode->Buffer=(char *)malloc(MAXFILESIZE);
	memset(UFDTArr[i].ptrfiletable->ptrinode->Buffer,0,1024);
	//memset(konala(Buffer),kashane(0),kiti(1024(size)));



	return i;
}
//**************************************


int rm_file(char *name)
{
	int fd = 0;
	fd=GetFDFromName(name);
	if(fd == -1)
	{
		return -1;
	}
	(UFDTArr[fd].ptrfiletable->ptrinode->LinkCount)--;
	
	if((UFDTArr[fd].ptrfiletable->ptrinode->LinkCount) == 0)
	{
		(UFDTArr[fd].ptrfiletable->ptrinode->FileType) = 0;
		free(UFDTArr[fd].ptrfiletable);
	}
	(UFDTArr[fd].ptrfiletable) = NULL;
	(SUPERBLOCKobj.Freeinode)++;
}

//****************************************

int ReadFile(int fd, char *arr,int size)
{
	int read_size=0;
	if(UFDTArr[fd].ptrfiletable == NULL)
	{
		return -1;
	}
	if(((UFDTArr[fd].ptrfiletable->mode) != READ) && ((UFDTArr[fd].ptrfiletable->mode) != READ+WRITE))
	{
		return -2;
	}
	if(((UFDTArr[fd].ptrfiletable->ptrinode->permission) != READ) && ((UFDTArr[fd].ptrfiletable->ptrinode->permission) != READ+WRITE))
	{
		return -2;
	}
	if((UFDTArr[fd].ptrfiletable->readoffset) == MAXFILESIZE)
	{
		return -3;
	}
	if((UFDTArr[fd].ptrfiletable->ptrinode->FileType) != REGULAR)
	{
		return -4;
	}
	read_size=((UFDTArr[fd].ptrfiletable->ptrinode->ActualFileSize)-(UFDTArr[fd].ptrfiletable->readoffset));
	if(read_size<size)
	{
		strncpy(arr,((UFDTArr[fd].ptrfiletable->ptrinode->Buffer)+(UFDTArr[fd].ptrfiletable->readoffset)),read_size);
		(UFDTArr[fd].ptrfiletable->readoffset)=(UFDTArr[fd].ptrfiletable->readoffset)+read_size;
	}
	else
	{		
	strncpy(arr,((UFDTArr[fd].ptrfiletable->ptrinode->Buffer)+(UFDTArr[fd].ptrfiletable->readoffset)),size);	
	(UFDTArr[fd].ptrfiletable->readoffset)=(UFDTArr[fd].ptrfiletable->readoffset)+size;
	}
	return size;
}

//**************************************************************
int WriteFile(int fd , char *arr, int isize)
{
	if(((UFDTArr[fd].ptrfiletable->mode) !=WRITE) && ((UFDTArr[fd].ptrfiletable -> mode) != (READ+WRITE)))
	{
		return -1;
	}
	if(((UFDTArr[fd].ptrfiletable->ptrinode->permission) !=WRITE) && ((UFDTArr[fd].ptrfiletable->ptrinode->permission ) != (READ+WRITE)))
	{
		return -1;
	}
	if((UFDTArr[fd].ptrfiletable->writeoffset) == MAXFILESIZE)
	{
		return -2;
	}
	if((UFDTArr[fd].ptrfiletable->ptrinode->FileType) != REGULAR)
	{
		return -3;
	}
	//data copy from arr to buffer (and buffer into file)
	strcpy(((UFDTArr[fd].ptrfiletable->ptrinode->Buffer)+(UFDTArr[fd].ptrfiletable->writeoffset)),arr);
	(UFDTArr[fd].ptrfiletable->writeoffset)=(UFDTArr[fd].ptrfiletable->writeoffset)+isize;
	(UFDTArr[fd].ptrfiletable->ptrinode->ActualFileSize)=(UFDTArr[fd].ptrfiletable->ptrinode->ActualFileSize)+isize;
	return isize;
	
}

//********************************************
int OpenFile(char *name,int mode)
{
	int i=0;
	PINODE temp=NULL;
	if((name==NULL) ||(mode <=0))
	{
		return -1;
	}
	temp=Get_Inode(name);
	if(temp==NULL)
	{
		return -2;
	}
	if(temp->permission < mode)
	{
		return -3;
	}
	while(i<MAXINODE)
	{
		if(UFDTArr[i].ptrfiletable == NULL)
		{
			break;
		}
		i++;
	}
	UFDTArr[i].ptrfiletable=(PFILETABLE)malloc(sizeof(FILETABLE));
	if(UFDTArr[i].ptrfiletable == NULL)
	{
		return -4;
	}
	UFDTArr[i].ptrfiletable->count=1;
	UFDTArr[i].ptrfiletable->mode=mode;
	if(mode==(READ+WRITE))
	{
		UFDTArr[i].ptrfiletable->readoffset=0;
		UFDTArr[i].ptrfiletable->writeoffset=0;
	}
	if(mode == READ)
	{
		UFDTArr[i].ptrfiletable->readoffset=0;
	}
	if(mode == WRITE)
	{
		UFDTArr[i].ptrfiletable->writeoffset=0;
	}
	
	UFDTArr[i].ptrfiletable->ptrinode=temp;
	(UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount)++;
	
	return i;
}
//*******************************************


void CloseFileByFD(int fd)
{
	UFDTArr[fd].ptrfiletable->readoffset = 0;
	UFDTArr[fd].ptrfiletable->writeoffset = 0;
	(UFDTArr[fd].ptrfiletable->ptrinode->ReferenceCount)--;
}

//**********************************************************


int CloseFileByName(char *name)
{
	int fd = 0;
	fd=GetFDFromName(name);
	if(fd < 0)
	{
		return -1;
	}
	UFDTArr[fd].ptrfiletable->readoffset = 0;
	UFDTArr[fd].ptrfiletable->writeoffset = 0;
	(UFDTArr[fd].ptrfiletable->ptrinode->ReferenceCount)--;
	
	return 0;
}

//*********************************************************


void CloseAllFile()
{
	int i=0;
	while(i < MAXINODE)
	{
		if(UFDTArr[i].ptrfiletable != NULL)
		{
			UFDTArr[i].ptrfiletable->readoffset = 0;
			UFDTArr[i].ptrfiletable->writeoffset = 0;
			(UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount)--;
			break;
		}
		i++;
	}
	
}

//**********************************************************

int LseekFile(int fd,int size,int from)
{
	if((fd < 0) || (from > 2))
	{
		return -1;
	}
	if(UFDTArr[fd].ptrfiletable == NULL)
	{
		return -1;
	}
	if((UFDTArr[fd].ptrfiletable->mode == READ) || (UFDTArr[fd].ptrfiletable->mode == READ+WRITE))
	{
		if(from == CURRENT)
		{
			if(((UFDTArr[fd].ptrfiletable->readoffset)+size) > (UFDTArr[fd].ptrfiletable->ptrinode->ActualFileSize))
			{
				return -1;
			}
			if((UFDTArr[fd].ptrfiletable->readoffset)<0)
			{
				return -1;
			}
			(UFDTArr[fd].ptrfiletable->readoffset) = (UFDTArr[fd].ptrfiletable->readoffset) + size;
		}
		else if(from == START)
		{
			if(size > (UFDTArr[fd].ptrfiletable->ptrinode->ActualFileSize))
			{
				(UFDTArr[fd].ptrfiletable->readoffset) + (UFDTArr[fd].ptrfiletable->ptrinode->ActualFileSize);
				return 0;
			}
			if(size < 0)
			{
				return -1;
			}
			(UFDTArr[fd].ptrfiletable->readoffset) = size;
		}
		else if(from == END)
		{
			if((UFDTArr[fd].ptrfiletable->ptrinode->ActualFileSize) + size > MAXFILESIZE)
			{
				return -1;
			}
			if(((UFDTArr[fd].ptrfiletable->readoffset) + size) < 0)
			{
				return -1;
			}
			(UFDTArr[fd].ptrfiletable->readoffset) = (UFDTArr[fd].ptrfiletable->ptrinode->ActualFileSize) + size;
		}
	}
	else if(UFDTArr[fd].ptrfiletable->mode == WRITE)
	{
		if(from == CURRENT)
		{
			if(((UFDTArr[fd].ptrfiletable->writeoffset) + size) > MAXFILESIZE)
			{
				return -1;
			}
			if((UFDTArr[fd].ptrfiletable->writeoffset) < 0)
			{
				return -1;
			}
			if(((UFDTArr[fd].ptrfiletable->writeoffset) + size) > (UFDTArr[fd].ptrfiletable->ptrinode->ActualFileSize))
			{
				(UFDTArr[fd].ptrfiletable->writeoffset) + (UFDTArr[fd].ptrfiletable->ptrinode->ActualFileSize); 
			}
			(UFDTArr[fd].ptrfiletable->writeoffset) = (UFDTArr[fd].ptrfiletable->writeoffset) + size;
		}
		else if(from == START)
		{
			if(size > MAXFILESIZE)
			{
				return -1;
			}
			if(size < 0)
			{
				return -1;
			}
			if(size > (UFDTArr[fd].ptrfiletable->ptrinode->ActualFileSize))
			{
				UFDTArr[fd].ptrfiletable->ptrinode->ActualFileSize = size;
				(UFDTArr[fd].ptrfiletable->writeoffset) = size;		
			}
		}
		else if(from == END)
		{
			if(((UFDTArr[fd].ptrfiletable->ptrinode->ActualFileSize) + size) > MAXFILESIZE)
			{
				return -1;
			}
			if(((UFDTArr[fd].ptrfiletable->writeoffset) + size) < 0)
			{
				return -1;
			}
			(UFDTArr[fd].ptrfiletable->writeoffset) = (UFDTArr[fd].ptrfiletable->ptrinode->ActualFileSize) + size;
		}		
	}
}



//******************************************************
void ls_file()
{
	int i=0;
	PINODE temp = Head;
	if(SUPERBLOCKobj.Freeinode == MAXINODE)
	{
		printf("ERROR : There is no files\n");
		return;
	}
	printf("\nFileName\tInodeNumber\tFileSize\tLinkCount\n");
	printf("---------------------------------------------\n");
	while (temp != NULL)
	{
		if(temp->FileType != 0)
		{
			printf("%s\t\t%d\t\t%d\t\t%d\t\n",temp->FileName,temp->InodeNumber,temp->ActualFileSize,temp->LinkCount);
			
		}
		temp=temp->next;
	}
	
}
//**********************************************************

int fstat_file(int fd)
{
	PINODE temp = NULL;
	int i=0;
	if(fd < 0)
	{
		return -1;
	}
	if(UFDTArr[fd].ptrfiletable == NULL)
	{
		return -2;
	}
	
	temp = UFDTArr[fd].ptrfiletable->ptrinode;
	
	printf("\n--------- Statictics information ------------\n");
	printf("File name : %s\n",temp->FileName);
	printf("File Size : %d\n",temp->FileSize);
	printf("Actual file size : %d\n",temp->ActualFileSize);
	printf("Link Count : %d\n",temp->LinkCount);
	printf("Reference count : %d\n",temp->ReferenceCount);
	
	if(temp->permission == 1)
	{
		printf("Permission is : Read only\n");
	}
	if(temp->permission == 2)
	{
		printf("Permission is : Write only\n");
	}
	if(temp->permission == 3)
	{
		printf("Permission is : Read and Write\n");
	}
	
	return 0;
}

//**********************************************************

int stat_file(char *name)
{
	PINODE temp = Head;
	int i=0;
	
	if(name == NULL)
	{
		return -1;
	}
	while(temp != NULL)
	{
		if(strcmp(name ,temp->FileName) == 0)
		{
			break;
		}
		temp=temp->next;
	}
	printf("\n--------- Statistic information --------\n");
	printf("File name : %s\n",temp->FileName);
	printf("File Size : %d\n",temp->FileSize);
	printf("Actual file Size : %d\n",temp->ActualFileSize);
	printf("Link Count : %d\n",temp->LinkCount);
	printf("Reference count : %d\n",temp->ReferenceCount);
	
	if(temp->permission == 1)
	{
		printf("Permission is : Read only \n");
	}
	if(temp->permission == 2)
	{
		printf("Permission is : Write only\n");
	}
	if(temp->permission == 3)
	{
		printf("Permission is : Read and write \n");
	}
	
	return 0;
}

//**********************************************************


int truncate_File(char *name)
{
	int fd=-1;
	fd=GetFDFromName(name);
	if(fd < 0)
	{
		return -1;
	}
	memset((UFDTArr[fd].ptrfiletable->ptrinode->Buffer),0,1024);
	
	UFDTArr[fd].ptrfiletable->readoffset=0;
	UFDTArr[fd].ptrfiletable->writeoffset=0;
	UFDTArr[fd].ptrfiletable->ptrinode->ActualFileSize=0;
}

//**********************************************************


int main()
{
	char *ptr=NULL;
	char str[80];
	char command[4][80],arr[1024];
	int count=0,ret=0,fd=0;
	
	InitialiseSuperBlock();
	CreateDILB();
	
	while(1)
	{
		fflush(stdin);
		strcpy(str,"");
		
		printf("\nCustomised VFS>");
		fgets(str,80,stdin);
		
		count=sscanf(str,"%s %s %s %s",command[0],command[1],command[2],command[3]);
		
		if(count==1)
		{
			if(strcmp(command[0],"ls")==0)
			{
				ls_file();
				
			}
			else if(strcmp(command[0],"closeall")==0)
			{
				CloseAllFile();
				printf("All file close successfully\n");
				continue;
			}
			else if(strcmp(command[0],"clear")==0)
			{
				system("cls");
				continue;
			}
			else if(strcmp(command[0],"help")==0)
			{
				DisplayHelp();
				continue;
			}
			else if(strcmp(command[0],"exit")==0)
			{
				printf("Thanks for using VFS\n");
				break;
			}
			else
			{
				printf("Command not  found\n");
			}
		}
		else if(count==2)
		{
			if(strcmp(command[0],"stat")==0)
			{
				ret=stat_file(command[1]);
				if(ret == -1)
				{
					printf("ERROR : Incorrect parameters\n");
				}
				if(ret == -2)
				{
					printf("ERROR : There is no such file\n");
				}
				continue;
			}
			else if(strcmp(command[0],"fstat")==0)
			{
				ret=fstat_file(atoi(command[1]));
				if(ret == -1)
				{
					printf("ERROR : Incorrect parameters\n");
				}
				if(ret == -2)
				{
					printf("ERROR : There is no such file\n");
				}
				continue;
			}
			else if(strcmp(command[0],"close")==0)
			{
				ret=CloseFileByName(command[1]);
				if(ret == -1)
				{
					printf("There is no such file\n");
				}
				continue;
			}
			else if(strcmp(command[0],"rm")==0)
			{
				ret=rm_file(command[1]);
				if(ret == -1)
				{
					printf("There is no such file\n");
				}
				continue;
			}
			else if(strcmp(command[0],"man")==0)
			{
				man(command[1]);
			}
			else if(strcmp(command[0],"write")==0)
			{
				fd=GetFDFromName(command[1]);
				if(fd == -1)
				{
					printf("file not exit \n");continue;
				}
				printf("Enter the data that you want to write in the file \n");
				fgets(arr,1024,stdin);
				//scanf("%s",arr);
				ret=strlen(arr);
				if(ret == 0)
				{
					printf("Incorrect parameter \n");continue;
				}
				ret=WriteFile(fd,arr,ret);
				if (ret >= 0)
				{ 
					printf("data written successfully of size %d \n",ret);
					continue;
				}
				if(ret == -1)
				{
					printf("Incorrect parameter\n"); 
				}
				if(ret == -2)
				{
					printf("ERROR : file is full\n");
				}
				if(ret == -3)
				{
					printf("ERROR : regular file required\n");
				}
			}
			else if(strcmp(command[0],"truncate")==0)
			{
				ret=truncate_File(command[1]);
				if(ret == -1)
				{
					printf("Icorrect parameter\n");
				}
			}
			else
			{
				printf("Command not found");
				continue;
			}
		}
		else if(count == 3)
		{
			if(strcmp(command[0],"create")==0)
			{
				ret=CreateFile(command[1],atoi(command[2]));
				if(ret>=0)
					printf("File is successfully created with file descriptor %d \n",ret);
				if(ret==-1)
					printf("Incorrect parameter\n");
				if(ret==-2)
					printf("There is no free inode \n");
				if(ret==-3)
					printf("File is already exist \n");
				if(ret==-4)
					printf("Memory allocation failure");
					
				continue;
			}
			else if(strcmp(command[1],"open")==0)
			{
				ret=OpenFile(command[1],atoi(command[2]));
				if(ret >= 0)
				printf("file open successfully with file descriptor %d\n",ret);
				if(ret == -1)
				printf("Incorrect parameter \n");
				if(ret == -2)
				printf("file not exist \n");
				if(ret == -3)
				printf("permission not allowed\n");
				if(ret == -4)
				printf("memory allocation failure\n");
				continue;
			}
			else if (strcmp(command[0],"read")==0)
			{
				int fd=0,size=0;
				fd=GetFDFromName(command[1]);
				if(fd ==-1)
				{
					printf("file not exist\n"); continue;
				}
				ptr=(char *)malloc(sizeof(atoi(command[2])));
				if(ptr ==  NULL)
				{
					printf("memory allocation failure");
				}
				ret=ReadFile(fd,ptr,atoi(command[2]));
				if(ret ==-1)
				{
					printf("file is not existing\n");
				}
				if(ret == -2)
				{
					printf(" permission failure\n");
				}
				if(ret == -3)
				{
					printf(" file is full \n");
				}
				if(ret == -4)
				{
					printf(" Regular file not found\n");
				}
				if(ret>0)
				{
					write(2,ptr,ret);
				}
				continue;
			}
			else
			{
				printf("\n ERROR : command not found");
			}
		}
		else if(count==4)
		{
			if(strcmp(command[1],"lseek")==0)
			{
				fd=GetFDFromName(command[1]);
				if(fd == -1)
				{
					printf("ERROR : Incorrect parameter\n"); continue;
				}
				ret=LseekFile(fd, atoi(command[2]),atoi(command[3]));
				if(ret == -1)
				{
					printf("ERROR : Unable to perform lseek \n");
				}
			}
			else 
			{
				printf("\n ERROR : command not found\n");
				continue;
			}
		}
		else
		{
			printf("ERROR : command not fount\n");\
			continue;
		}
	}
	return 0;
}
