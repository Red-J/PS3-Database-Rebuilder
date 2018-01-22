#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ppu-lv2.h>
#include <io/pad.h>
#include <fcntl.h>

#include <sys/file.h>


#include <sysutil/msg.h>
#include <sysutil/sysutil.h>

#include "rsxutil.h"

#define SUCCESS 0
#define FAILED -1

#define SC_SYS_POWER 					(379)
#define SYS_REBOOT				 		0x8201

static vs32 dialog_action = 0;

static void do_flip()
{
	sysUtilCheckCallback();
	flip();
}
int sys_fs_mount(char const* deviceName, char const* deviceFileSystem, char const* devicePath, int writeProt)
{
	lv2syscall8(837, (u64) deviceName, (u64) deviceFileSystem, (u64) devicePath, 0, (u64) writeProt, 0, 0, 0 );
	return_to_user_prog(int);
}
int CopyFile(char* path, char* path2)
{
	int ret = 0;
	s32 fd = -1;
	s32 fd2 = -1;
	u64 lenght = 0LL;

	u64 pos = 0ULL;
	u64 readed = 0, writed = 0;

	char *mem = NULL;

	sysFSStat stat;

	ret= sysLv2FsStat(path, &stat);
	lenght = stat.st_size;

	if(ret) goto skip;

	if(strstr(path, "/dev_hdd0/") != NULL && strstr(path2, "/dev_hdd0/") != NULL)
	{
		if(strcmp(path, path2)==0) return ret;

		sysLv2FsUnlink(path2);
		sysLv2FsLink(path, path2);

		if (sysLv2FsStat(path2, &stat) == 0) return 0;
	}

	ret = sysLv2FsOpen(path, 0, &fd, S_IRWXU | S_IRWXG | S_IRWXO, NULL, 0);
	if(ret) goto skip;

	ret = sysLv2FsOpen(path2, SYS_O_WRONLY | SYS_O_CREAT | SYS_O_TRUNC, &fd2, 0777, NULL, 0);
	if(ret) {sysLv2FsClose(fd);goto skip;}

	mem = malloc(0x100000);
	if (mem == NULL) return FAILED;

	while(pos < lenght)
	{
		readed = lenght - pos; if(readed > 0x100000ULL) readed = 0x100000ULL;
		ret=sysLv2FsRead(fd, mem, readed, &writed);
		if(ret<0) goto skip;
		if(readed != writed) {ret = 0x8001000C; goto skip;}

		ret=sysLv2FsWrite(fd2, mem, readed, &writed);
		if(ret<0) goto skip;
		if(readed != writed) {ret = 0x8001000C; goto skip;}

		pos += readed;
	}

skip:

	if(mem) free(mem);
	if(fd >=0) sysLv2FsClose(fd);
	if(fd2>=0) sysLv2FsClose(fd2);
	if(ret) return ret;

	ret = sysLv2FsStat(path2, &stat);
	if((ret == SUCCESS) && (stat.st_size == lenght)) ret = SUCCESS; else ret = FAILED;

	return ret;
}
static void dialog_handler(msgButton button,void *usrData)
{
	switch(button) {
		case MSG_DIALOG_BTN_OK:
			dialog_action = 1;
			break;
		case MSG_DIALOG_BTN_NO:
		case MSG_DIALOG_BTN_ESCAPE:
			dialog_action = 2;
			break;
		case MSG_DIALOG_BTN_NONE:
			dialog_action = -1;
			break;
		default:
			break;
	}
}

void program_exit_callback()
{
	gcmSetWaitFlip(context);
	rsxFinish(context,1);
}

void sysutil_exit_callback(u64 status,u64 param,void *usrdata)
{
	switch(status) {
		case SYSUTIL_EXIT_GAME:
			break;
		case SYSUTIL_DRAW_BEGIN:
		case SYSUTIL_DRAW_END:
			break;
		default:
			break;
	}
}

int main(int argc,char *argv[])
{
	s32 ret;
	msgType dialogType;
	void *host_addr = memalign(1024*1024,HOST_SIZE);

	printf("msgdialog test...\n");

	init_screen(host_addr,HOST_SIZE);
	ioPadInit(7);

	ret = atexit(program_exit_callback);
	ret = sysUtilRegisterCallback(SYSUTIL_EVENT_SLOT0,sysutil_exit_callback,NULL);

	msgDialogOpenErrorCode(0xBADC0FFE,dialog_handler,NULL,NULL);
	msgDialogClose(3000.0f);

	dialog_action = 0;
	while(dialog_action!=-1)
		do_flip();

	msgDialogAbort();
	
	// yes/no dialog type
	dialogType = (MSG_DIALOG_NORMAL | MSG_DIALOG_BTN_TYPE_YESNO | MSG_DIALOG_DISABLE_CANCEL_ON | MSG_DIALOG_DEFAULT_CURSOR_NO);
	msgDialogOpen2(dialogType,"                  -= DB Rebuilder =-                  \n \n Pour lancer faites 'oui' pour quitter faites 'non'",dialog_handler,NULL,NULL);

	dialog_action = 0;
	while(!dialog_action)
		do_flip();

	msgDialogAbort();

	// OK dialog type
	dialogType = (MSG_DIALOG_NORMAL | MSG_DIALOG_BTN_OK);
	if(dialog_action==1)
		{
		msgDialogOpen2(dialogType,"Installaion effectuée",dialog_handler,NULL,NULL);
		{goto install;}
		
install:
{		sysFSStat stat;

	// copy files to dev_flash
	if(sysLv2FsStat("/dev_blind", &stat) != SUCCESS)
		sys_fs_mount("CELL_FS_IOS:BUILTIN_FLSH1", "CELL_FS_FAT", "/dev_blind", 0);

	if(sysLv2FsStat("/dev_blind", &stat) == SUCCESS)
		{ lv2syscall3(392, 0x1004, 0x4, 0x6); } 
	  CopyFile("/dev_hdd0/game/DBREBUILD/USRDIR/db.err","/dev_hdd0/mms/db.err");

	// reboot
	sysLv2FsUnlink("/dev_hdd0/tmp/turnoff");
	//{ lv2syscall3(392, 0x1004, 0x4, 0x6); } //1 Beep
    //{ lv2syscall3(392, 0x1004, 0xa, 0x1b6); } //3 beep
	
	dialog_action = 0;
	while(!dialog_action)
		do_flip();

	msgDialogAbort();
	}	
	{ lv2syscall3(392, 0x1004, 0xa, 0x1b6); } 
    {lv2syscall3(SC_SYS_POWER, SYS_REBOOT, 0, 0); return_to_user_prog(int);}

	return 0;
}
		
		else
		{msgDialogOpen2(dialogType,"Appuyer sur O pour quitter",dialog_handler,NULL,NULL);

	
	dialog_action = 0;
	while(!dialog_action)
		do_flip();

	msgDialogAbort();
	sysLv2FsUnlink("/dev_hdd0/tmp/turnoff");	
	{ lv2syscall3(392, 0x1004, 0xa, 0x1b6); } 
    {lv2syscall3(SC_SYS_POWER, SYS_REBOOT, 0, 0); return_to_user_prog(int);}
	return 0;}
}