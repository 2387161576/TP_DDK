#include "hook.h"
#include "util.h"


OB_SECURITY_METHOD g_fnSecurityProcedure=NULL;


 NTSTATUS MySecurityProcedure(
	IN PVOID Object,
	IN SECURITY_OPERATION_CODE OperationCode,
	IN PSECURITY_INFORMATION SecurityInformation,
	IN OUT PSECURITY_DESCRIPTOR SecurityDescriptor,
	IN OUT PULONG CapturedLength,
	IN OUT PSECURITY_DESCRIPTOR *ObjectsSecurityDescriptor,
	IN POOL_TYPE PoolType,
	IN PGENERIC_MAPPING GenericMapping,
	IN ULONG unk
	)
 {
	 PCHAR pTargetName;
	 //KdPrint(("++Current Eprocess:%x  Objcet:%x++\n",IoGetCurrentProcess(),Object));
	 KdPrint(("++Current Eprocess:%s  Objcet:%s++\n",
		 PsGetProcessImageFileName(IoGetCurrentProcess()),
		 PsGetProcessImageFileName((PEPROCESS)Object)));
	 pTargetName=PsGetProcessImageFileName((PEPROCESS)Object);
	 if(_stricmp(pTargetName,"calc.exe")==0)
		 return STATUS_INVALID_PARAMETER;

	 return g_fnSecurityProcedure(
		 Object,
		 OperationCode,
		 SecurityInformation,
		 SecurityDescriptor,
		 CapturedLength,
		 ObjectsSecurityDescriptor,
		 PoolType,GenericMapping,
		 unk);
 }

void handleObjectHook(BOOLEAN bHook)
{
	PVOID TypeInfo=(PVOID)((ULONG)*PsProcessType+0x60);
	//PVOID TypeInfo=(PVOID)((ULONG)DbgkDebugObjectType+0x60);
	if(bHook)
	{
		g_fnSecurityProcedure=(OB_SECURITY_METHOD)*(PULONG)((ULONG)TypeInfo+0x40);
		*(PULONG)((ULONG)TypeInfo+0x40)=(ULONG)MySecurityProcedure;
	}
	else
	{
		*(PULONG)((ULONG)TypeInfo+0x40)=(ULONG)g_fnSecurityProcedure;
	}
}

void LoadImageNotifyRoutine(IN PUNICODE_STRING FullName,
							IN HANDLE ProcessId,
							IN PIMAGE_INFO ImageInfo)
{
	KIRQL Irql;
	ULONG uImageBase;
	if(wcsstr(FullName->Buffer,L"TesSafe.sys")!=0)
	{
		__asm //ȥ��ҳ�汣��
		{
			push eax
			cli
				mov eax,cr0
				and eax,not 10000h //and eax,0FFFEFFFFh
				mov cr0,eax
				pop eax
		}
		//����IRQL�жϼ�
		Irql=KeRaiseIrqlToDpcLevel();

		*(UCHAR*)KdDisableDebugger=0xC3;
		uImageBase=ImageInfo->ImageBase;
		*(PUCHAR)(uImageBase+0x7031)=0x74;
		*(PUCHAR)(uImageBase+0x715f)=0xEB;

		KeLowerIrql(Irql);
		__asm  //�ָ�ҳ�汣��
		{ 
			push eax
			mov eax,cr0
				or eax,10000h
				mov cr0,eax
				sti
			pop eax
		}
		KdPrint(("TesSafe.sys ȥ��KdDisableDebugger���\n"));
	}

	return;
}


//ȫ�ֱ���
PSYSTEM_DESCRIPTOR_TABLE KeServiceDescriptorTableShadow; //sstd Shadow
ULONG g_uNtReadVirtualMemoryAddr;
ULONG g_uNtReadVirtualMemoryAddr_offest3;
ULONG g_uNtWriteVirtualMemoryAddr;
ULONG g_uNtWriteVirtualMemoryAddr_offset3;
UCHAR g_KiAttachProcess_10[10]={0};

UCHAR JmpBackCRC3[25]={0};
ULONG g_uCRC3Ret=0; //CRC3 TPԭʼjmp
ULONG g_uDebugPort_Reset1=0; //debugPort���㺯��1
ULONG g_uDebugPort_Reset_4; //debugPort���㺯��1 ԭǰ�Ľ�����,����ƭ��CRC���
//������TPǰ����ʼ��
void InitBeforeTP()
{
	ULONG addrNtReadMemory,addrNtWriteMemory,
		_KeStackAttachProcess,_Cur_KeStackAttachProcess,KiAttachProcess_bytes,_KiAttachProcess;
	addrNtReadMemory=(ULONG)KeServiceDescriptorTable->ServiceTableBase+186*4;
	addrNtWriteMemory=(ULONG)KeServiceDescriptorTable->ServiceTableBase+277*4;
	addrNtReadMemory=*(PULONG)addrNtReadMemory;
	addrNtWriteMemory=*(PULONG)addrNtWriteMemory;

	g_uNtReadVirtualMemoryAddr=addrNtReadMemory;
	g_uNtWriteVirtualMemoryAddr=addrNtWriteMemory;
	g_uNtReadVirtualMemoryAddr_offest3=*(PULONG)(addrNtReadMemory+3);
	g_uNtWriteVirtualMemoryAddr_offset3=*(PULONG)(addrNtWriteMemory+3);

	//����KiAttachProcessǰ10�ֽ�
	 _KeStackAttachProcess=MyGetFunAddress(L"KeStackAttachProcess");
	 _Cur_KeStackAttachProcess=_KeStackAttachProcess+0x65;
	 KiAttachProcess_bytes=*(PULONG)(_Cur_KeStackAttachProcess+1);
	 _KiAttachProcess=KiAttachProcess_bytes+_Cur_KeStackAttachProcess+5;
	RtlCopyMemory(g_KiAttachProcess_10,_KiAttachProcess,10);
	KeServiceDescriptorTableShadow=(ULONG)KeServiceDescriptorTable-0x40;
}

//����ж��
void TP_DDK_Unload()
{
	FkNtOpenThread(FALSE);
	FkNtOpenProcss(FALSE);
	FkNtReadVirtualMemory(FALSE);
	FkNtWriteVirtualMemory(FALSE);
	
}

void ReSumeKiAttachProcess()
{
	KIRQL Irql;
	ULONG _KeStackAttachProcess, _Cur_KeStackAttachProcess, _KiAttachProcess;
	ULONG bytes;
	//UCHAR orgBytes[10]={0x8B,0xff,0x55,0x8b,0xec,0x53,0x56,0x8b,0x75,0x8};

	_KeStackAttachProcess=MyGetFunAddress(L"KeStackAttachProcess");
	_Cur_KeStackAttachProcess=_KeStackAttachProcess+0x65;
	bytes=*(PULONG)(_Cur_KeStackAttachProcess+1);
	//KiAttachProcess-Cur_KeStackAttachProcess-5=bytes;
	_KiAttachProcess=bytes+_Cur_KeStackAttachProcess+5;

		DisableWP();
		Irql=KeRaiseIrqlToDpcLevel();
		RtlCopyMemory((PVOID)_KiAttachProcess,(PVOID)g_KiAttachProcess_10,10);

		KeLowerIrql(Irql);
		EnableWP();
		KdPrint(("TP_DDK: ���KiAttachProcess\n"));
}

/************************************************************************/
/*TP_NtOpenProcess                                                              */
/************************************************************************/
ULONG NtOpenProcessCodeLen=0; //NtOpenProcess ��ת�޸ĵĴ��볤��
ULONG g_Process_fnObOpenObjectByPointer=0; //ObOpenObjectByPointer ������ַ
ULONG g_Process_HookObOpenObjectByPointer; //д��jmp�ĵ�ַ
void FkNtOpenProcss(BOOLEAN bFk)
{
	KIRQL Irql;
	BOOLEAN bFound;
	PUCHAR p;
	ULONG i;
	bFound=FALSE;
	p=(PUCHAR)NtOpenProcess;;
	i=0;
	g_Process_fnObOpenObjectByPointer=MyGetFunAddress(L"ObOpenObjectByPointer");

	if(bFk)
	{
		ULONG bytes=0;
		while(i<0x300)
		{
			if (*p==0x50 && 
				*(p+1)==0xff &&
				*(p+4)==0xff &&
				*(p+7)==0xE8)
			{
				bFound=TRUE;
				break;
			}
			i++;
			p++;
		}
		if(!bFound)
		{
			KdPrint(("TP_DDK: �Ҳ��� ObOpenObjectByPointer ������ַ\n"));
			return;
		}
		g_Process_HookObOpenObjectByPointer=p+1;


		//����ɵ�buffer
		NtOpenProcessCodeLen=ade32_get_code_length(g_Process_HookObOpenObjectByPointer,5);
		RtlCopyMemory((PVOID)JmpNtOpenProcess,(PVOID)g_Process_HookObOpenObjectByPointer,NtOpenProcessCodeLen);
	

		//д����ת bytes=JmpNtOpenProcess-g_HookObOpenObjectByPointer-5;
		bytes=(ULONG)JmpNtOpenProcess-g_Process_HookObOpenObjectByPointer-5;
		DisableWP();
		Irql=KeRaiseIrqlToDpcLevel();
		*(PUCHAR)g_Process_HookObOpenObjectByPointer=0xE9;
		RtlCopyMemory((PVOID)(g_Process_HookObOpenObjectByPointer+1),&bytes,sizeof(ULONG));
		KeLowerIrql(Irql);
		EnableWP();

		KdPrint(("TP_DDK: ���NtOpenProcess����!\n"));
	}
	else
	{
		if(*(PUCHAR)JmpNtOpenProcess==0 || *(PUCHAR)JmpNtOpenProcess == 0x90)
			return;
		DisableWP();
		Irql=KeRaiseIrqlToDpcLevel();
		RtlCopyMemory((PVOID)g_Process_HookObOpenObjectByPointer,JmpNtOpenProcess,NtOpenProcessCodeLen);
		*(PUCHAR)JmpNtOpenProcess=0;
		KeLowerIrql(Irql);
		EnableWP();

		KdPrint(("TP_DDK: ��ԭNtOpenProcess����!\n"));
	}
	

}

BOOLEAN  g_NtOpenProcess_bIsDnfProcess=FALSE; //ȫ�ֱ������Ƿ���dnf����
 __declspec(naked) void __stdcall JmpNtOpenProcess()
 {
	 __asm
	 {
		 nop
		 nop
		 nop
		 nop
		 nop
		 nop
		 nop
		 nop
		 nop
		 nop
		 nop
		 nop
		 pushad
		 pushfd
		 call IsDnfProcess
		 mov g_NtOpenProcess_bIsDnfProcess,al
		 popfd
		 popad
		 cmp g_NtOpenProcess_bIsDnfProcess,0
		 je NotNDF
		 mov eax,g_Process_HookObOpenObjectByPointer;
		 add eax,NtOpenProcessCodeLen;
		 jmp eax //DNF���̣�ִ��TP������
NotNDF:	
		 mov eax,g_Process_fnObOpenObjectByPointer
		 call eax //����ԭ��ObOpenObjectByPointer
		 mov edi, g_Process_HookObOpenObjectByPointer
		 add edi,NtOpenProcessCodeLen
		 add edi,5
		 jmp edi	
	 }
 }



 /************************************************************************/
 /*TP_NtOpenThread                                                                      */
 /************************************************************************/
 ULONG NtOpenThreadCodeLen=0; //NtOpenProcess ��ת�޸ĵĴ��볤��
 ULONG g_Thread_fnObOpenObjectByPointer=0; //ObOpenObjectByPointer ������ַ
 ULONG g_Thread_HookObOpenObjectByPointer; //д��jmp�ĵ�ַ
 void FkNtOpenThread(BOOLEAN bFk)
 {
	 KIRQL Irql;
	 BOOLEAN bFound;
	 PUCHAR p;
	 ULONG i;
	 bFound=FALSE;
	 p=(PUCHAR)NtOpenThread;;
	 i=0;
	 g_Thread_fnObOpenObjectByPointer=MyGetFunAddress(L"ObOpenObjectByPointer");

	 if(bFk)
	 {
		 ULONG bytes=0;
		 while(i<0x300)
		 {
			 if (*p==0x50 && 
				 *(p+1)==0xff &&
				 *(p+4)==0xff &&
				 *(p+7)==0xE8)
			 {
				 bFound=TRUE;
				 break;
			 }
			 i++;
			 p++;
		 }
		 if(!bFound)
		 {
			 KdPrint(("TP_DDK: �Ҳ��� ObOpenObjectByPointer ������ַ\n"));
			 return;
		 }
		 g_Thread_HookObOpenObjectByPointer=p+1;


		 //����ɵ�buffer
		 NtOpenThreadCodeLen=ade32_get_code_length(g_Thread_HookObOpenObjectByPointer,5);
		 RtlCopyMemory((PVOID)JmpNtOpenThread,(PVOID)g_Thread_HookObOpenObjectByPointer,NtOpenThreadCodeLen);

		 
		 //д����ת bytes=JmpNtOpenProcess-g_HookObOpenObjectByPointer-5;
		 bytes=(ULONG)JmpNtOpenThread-g_Thread_HookObOpenObjectByPointer-5;
		 DisableWP();
		 Irql=KeRaiseIrqlToDpcLevel();

		 *(PUCHAR)g_Thread_HookObOpenObjectByPointer=0xE9;
		 RtlCopyMemory((PVOID)(g_Thread_HookObOpenObjectByPointer+1),&bytes,sizeof(ULONG));

		 KeLowerIrql(Irql);
		 EnableWP();

		 KdPrint(("TP_DDK: ���NtOpenThread����!\n"));
	 }
	 else
	 {
		 if(*(PUCHAR)JmpNtOpenThread==0 || *(PUCHAR)JmpNtOpenThread == 0x90)
			 return;
		 DisableWP();
		 Irql=KeRaiseIrqlToDpcLevel();
		 RtlCopyMemory((PVOID)g_Thread_HookObOpenObjectByPointer,JmpNtOpenThread,NtOpenThreadCodeLen);
		 *(PUCHAR)JmpNtOpenThread=0;
		 KeLowerIrql(Irql);
		 EnableWP();

		 KdPrint(("TP_DDK: ��ԭNtOpenThread����!\n"));
	 }


 }

 BOOLEAN  g_NtOpenThread_bIsDnfProcess=FALSE; //ȫ�ֱ������Ƿ���dnf����
 __declspec(naked) void __stdcall JmpNtOpenThread()
 {
	 __asm
	 {
		 nop
			 nop
			 nop
			 nop
			 nop
			 nop
			 nop
			 nop
			 nop
			 nop
			 nop
			 nop
			 pushad
			 pushfd
			 call IsDnfProcess
			 mov g_NtOpenThread_bIsDnfProcess,al
			 popfd
			 popad
			 cmp g_NtOpenThread_bIsDnfProcess,0
			 je NotNDF
			 mov eax,g_Thread_HookObOpenObjectByPointer;
		 add eax,NtOpenThreadCodeLen;
		 jmp eax //DNF���̣�ִ��TP������
NotNDF:	
		 mov eax,g_Thread_fnObOpenObjectByPointer
			 call eax //����ԭ��ObOpenObjectByPointer
			 mov edi, g_Thread_HookObOpenObjectByPointer
			 add edi,NtOpenThreadCodeLen
			 add edi,5
			 jmp edi	
	 }
 }


 /************************************************************************/
 /*TP_NtReadVirtualMemory                                                        */
 /************************************************************************/
 void FkNtReadVirtualMemory( BOOLEAN bFk )
 {
	 KIRQL Irql;
	 PULONG pNtReadMemory;
	 pNtReadMemory=(PULONG)((ULONG)KeServiceDescriptorTable->ServiceTableBase+186*4);
	 //addrNtWriteMemory=(ULONG)KeServiceDescriptorTable->ServiceTableBase+277*4;

	 if(bFk)
	 {
		 //if ( *(PUCHAR)g_uNtReadVirtualMemoryAddr !=0x6A)
		 //{
			// KdPrint(("TP_DDK:SSDT���Ѿ���hook,�޷���λNtReadVirtualMemory\n"));
			// return;
		 //}

		 DisableWP();
		 Irql=KeRaiseIrqlToDpcLevel();
		 *pNtReadMemory=FakeNtReadVirtualMemory;
		 KeLowerIrql(Irql);
		 EnableWP();
		 KdPrint(("TP_DDK: ���TP��NtReadVirtualMemory����\n"));
	 }
	 else
	 {
		 DisableWP();
		 Irql=KeRaiseIrqlToDpcLevel();
		 *pNtReadMemory=g_uNtReadVirtualMemoryAddr;
		 KeLowerIrql(Irql);
		 EnableWP();
		 KdPrint(("TP_DDK: �ָ�NtReadVirtualMemory SSDT��\n"));
	 }

 }


__declspec(naked)  void __stdcall FakeNtReadVirtualMemory()
 {
	 if(IsDnfProcess())
	 {
		 __asm
		 {
			 jmp [g_uNtReadVirtualMemoryAddr]
		 }
	 }
	 else
	 {
		 __asm
		 {
			 push 0x1C
			 push g_uNtReadVirtualMemoryAddr_offest3
			 mov eax,g_uNtReadVirtualMemoryAddr
			 add eax,7
			 jmp eax
		 }
	 }
 }

/************************************************************************/
/*TP_NtWriteVirtualMemory                                                        */
/************************************************************************/
void FkNtWriteVirtualMemory( BOOLEAN bFk )
{
	KIRQL Irql;
	PULONG pNtWriteMemory;
	pNtWriteMemory=(PULONG)((ULONG)KeServiceDescriptorTable->ServiceTableBase+277*4);
	//addrNtWriteMemory=(ULONG)KeServiceDescriptorTable->ServiceTableBase+277*4;

	if(bFk)
	{
		//if ( *(PUCHAR)g_uNtReadVirtualMemoryAddr !=0x6A)
		//{
		// KdPrint(("TP_DDK:SSDT���Ѿ���hook,�޷���λNtReadVirtualMemory\n"));
		// return;
		//}

		DisableWP();
		Irql=KeRaiseIrqlToDpcLevel();
		*pNtWriteMemory=FakeNtWriteVirtualMemory;
		KeLowerIrql(Irql);
		EnableWP();

		KdPrint(("TP_DDK: ���TP��NtWriteVirtualMemory����\n"));
	}
	else
	{
		DisableWP();
		Irql=KeRaiseIrqlToDpcLevel();
		*pNtWriteMemory=g_uNtWriteVirtualMemoryAddr;
		KeLowerIrql(Irql);
		EnableWP();
		KdPrint(("TP_DDK: �ָ�NtWriteVirtualMemory SSDT��\n"));
	}

}


__declspec(naked)  void __stdcall FakeNtWriteVirtualMemory()
{
	if(IsDnfProcess())
	{
		__asm
		{
			jmp [g_uNtWriteVirtualMemoryAddr]
		}
	}
	else
	{
		__asm
		{
			push 0x1C
				push g_uNtWriteVirtualMemoryAddr_offset3
				mov eax,g_uNtWriteVirtualMemoryAddr
				add eax,7
				jmp eax
		}
	}
}


void FkCRC( BOOLEAN bFk )
{
	KIRQL Irql;
	ULONG uImageBase,crc1,crc2,crc3=0;
	uImageBase=GetModuleBase("TesSafe.sys");
	if(uImageBase==0)
	{
		KdPrint(("TP_DDK:��ȡTesSafeģ��ʧ��,���CRCʧ��\n"));
		return;
	}

	crc1=uImageBase+AddrCRC1;
	crc2=uImageBase+AddrCRC2;
	crc3=uImageBase+AddrCRC3;
	if(bFk)
	{
		ULONG bytes_jmp;
		g_uCRC3Ret= uImageBase+0xcf6f6;
		g_uDebugPort_Reset1=uImageBase+DebugPortReset1; //��ʼ�����㺯��1��ַ
		g_uDebugPort_Reset_4=*(PULONG)g_uDebugPort_Reset1; //�������㺯��1ǰ�Ľ�
		
		DisableWP();
		Irql=KeRaiseIrqlToDpcLevel();

		//crc1
		*(PUCHAR)(crc1+0)=0x33;
		*(PUCHAR)(crc1+1)=0xC0;
		*(PUCHAR)(crc1+2)=0xC2;
		*(PUCHAR)(crc1+3)=0x0C;
		*(PUCHAR)(crc1+4)=0x00;

		//crc2
		*(PUCHAR)(crc2+0)=0xC3;
		KeLowerIrql(Irql);
		EnableWP();

		KdPrint(("TP_DDK:���CRC1\n"));
		KdPrint(("TP_DDK:���CRC2\n"));

		//CRC3
		WriteJmp((PVOID)crc3,FakeCRC3,JmpBackCRC3);

		KdPrint(("TP_DDK:���CRC3\n"));
	}
	else
	{
		UnhookFunction((PVOID)crc3,JmpBackCRC3);
		KdPrint(("TP_DDK:�ָ�CRC3\n"));
	}
}

__declspec(naked) void __stdcall FakeCRC3()
{
	__asm
	{
		pushfd
		cmp edx,g_uDebugPort_Reset1
		je g_uDebugPort_Reset1_4

		popfd
		push [edx]
		jmp [g_uCRC3Ret]

g_uDebugPort_Reset1_4:
		popfd
		push [g_uDebugPort_Reset_4]
		jmp [g_uCRC3Ret]


	}
}

__declspec(naked) void __stdcall FakeCRC2()
{
	__asm
	{
		push eax
		mov eax,[esp+4]
		cmp eax,g_uDebugPort_Reset1
		jne notTarget
		add eax,4
		mov [esp+4],eax
notTarget:
		pop eax
		jmp [g_uCRC3Ret]
	}
}
