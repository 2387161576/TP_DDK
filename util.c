#include "util.h"


#pragma PAGECODE
ULONG MyGetFunAddress( IN PCWSTR FunctionName)
{
	UNICODE_STRING UniCodeFunctionName;
	RtlInitUnicodeString( &UniCodeFunctionName, FunctionName );
	return (ULONG)MmGetSystemRoutineAddress( &UniCodeFunctionName );   
}



#pragma PAGECODE
ULONG* GetSSDT_CurrAddr( void* func )
{
	ULONG SSDT_NtOpenProcess_Cur_Addr,index;
	index=SYSTEM_INDEX(func);
	SSDT_NtOpenProcess_Cur_Addr=(ULONG)KeServiceDescriptorTable->ServiceTableBase+0x4*index;
	SSDT_NtOpenProcess_Cur_Addr=(ULONG*)SSDT_NtOpenProcess_Cur_Addr;
	return SSDT_NtOpenProcess_Cur_Addr;
}

#pragma PAGECODE
ULONG GetVersion()
{
	ULONG rtn;
	ULONG MajorVersion,MinorVersion,BuildNumber;
	PsGetVersion(&MajorVersion,&MinorVersion,&BuildNumber,NULL);//ϵͳ�汾.����1���汾,����2���汾,����3ʱ�����,����4�ִ�
	rtn=MajorVersion;
	rtn=rtn *10;     
	rtn+=MinorVersion;   //���汾+���汾
	return rtn;
}


#pragma PAGECODE
NTSTATUS CreateMyDevice( IN PDRIVER_OBJECT pDrvierObj )
{
	NTSTATUS status;
	PDEVICE_OBJECT pDevObj;

	//�����豸����
	UNICODE_STRING devName;
	UNICODE_STRING symLinkName;
	RtlInitUnicodeString(&devName,TP_DeviceName);

	//�����豸
	status=IoCreateDevice(pDrvierObj,0,&devName,FILE_DEVICE_UNKNOWN,0,TRUE,&pDevObj);
	if(!NT_SUCCESS(status))
	{
		switch(status)
		{
		case STATUS_INSUFFICIENT_RESOURCES:
			KdPrint(("��Դ���� STATUS_INSUFFICIENT_RESOURCES"));
			break;
		case STATUS_OBJECT_NAME_EXISTS:
			KdPrint(("ָ������������"));
			break;
		case STATUS_OBJECT_NAME_COLLISION:
			KdPrint(("//�������г�ͻ"));
			break;
		}
		KdPrint(("\n"));
		KdPrint(("�豸����ʧ��...++++++++\n"));
		return status;
	}

	KdPrint(("�豸�����ɹ�...++++++++\n"));
	pDevObj->Flags |= DO_BUFFERED_IO;
	//������������
	RtlInitUnicodeString(&symLinkName,TP_symLinkName);
	status=IoCreateSymbolicLink(&symLinkName,&devName);
	if (!NT_SUCCESS(status)) /*status����0*/
	{
		IoDeleteDevice( pDevObj );
		return status;
	}

	return STATUS_SUCCESS;
}

void DisableWP()
{
	__asm //ȥ��ҳ�汣��
	{
		cli
			push eax
			mov eax,cr0
			and eax,not 10000h //and eax,0FFFEFFFFh
			mov cr0,eax
			pop eax
	}
}

void EnableWP()
{
	__asm  //�ָ�ҳ�汣��
	{ 
		push eax
			mov eax,cr0
			or eax,10000h
			mov cr0,eax
			pop eax
			sti
	}
}

ULONG GetCodeLength( IN PVOID desFunc,IN ULONG NeedLengh )
{
	ULONG offset,codeLen;
	offset=0;
	codeLen=0;
	do 
	{
		codeLen=ade32_disasm((PVOID)((ULONG)desFunc+offset));
		if(codeLen==0)
			return 0;
		offset+=codeLen;
	} while (offset<NeedLengh);

	return offset;
}

//�ҹ�Function������FakeFunction
//JmpBuffer��FakeFunction��ת��Function����ת������
BOOLEAN HookFunction(PVOID Function, PVOID FakeFunction, PUCHAR JmpBuffer)
{
	ULONG length;
	UCHAR jmpCode[5];
	PUCHAR temp;
	KIRQL Irql;

	length = ade32_get_code_length(Function, 5);
	if(length == 0)
		return FALSE;

	temp = (PUCHAR)Function + length;
	RtlCopyMemory(JmpBuffer, Function, length);

	JmpBuffer[length] = 0xe9;
	*(PULONG)(JmpBuffer + length + 1) = ((PUCHAR)Function + length - (JmpBuffer + length) - 5);

	jmpCode[0] = 0xe9;
	*(PULONG)(&jmpCode[1]) = (ULONG)((PUCHAR)FakeFunction - (PUCHAR)Function - 5);

	DisableWP();
	Irql=KeRaiseIrqlToDpcLevel();
	RtlCopyMemory(Function, jmpCode, 5);
	KeLowerIrql(Irql);
	EnableWP();

	return TRUE;
}

BOOLEAN UnhookFunction(PVOID Function, PUCHAR JmpBuffer)
{
	ULONG length;

	if(JmpBuffer[0] == 0)
		return TRUE;

	length = ade32_get_code_length(JmpBuffer, 5);
	if(length == 0)
		return FALSE;
	__asm int 3
	DisableWP();
	RtlCopyMemory(Function, JmpBuffer, length);
	EnableWP();

	RtlZeroMemory(JmpBuffer, length);

	return TRUE;
}

void WriteJmp( PVOID Function,PVOID fakeFunction,PUCHAR JmpBuffer )
{
	ULONG length;
	UCHAR jmpCode[5];
	PUCHAR temp;
	KIRQL Irql;

	length = ade32_get_code_length(Function, 5);
	if(length == 0)
		return FALSE;
	RtlCopyMemory(JmpBuffer, Function, length);

	jmpCode[0] = 0xe9;
	*(PULONG)(&jmpCode[1]) = (ULONG)((PUCHAR)fakeFunction - (PUCHAR)Function - 5);

	DisableWP();
	Irql=KeRaiseIrqlToDpcLevel();
	RtlCopyMemory(Function, jmpCode, 5);
	KeLowerIrql(Irql);
	EnableWP();
}


BOOLEAN  IsDnfProcess()
{
	PEPROCESS curProcess= PsGetCurrentProcess();
	PUCHAR pszCurName=PsGetProcessImageFileName(curProcess);
	if(_stricmp("dnf.exe",pszCurName)==0)
	{
		//KdPrint(("��ǰ����:%s\n",pszCurName));
		return TRUE;
	}
	return FALSE;
}

PSYSTEM_DESCRIPTOR_TABLE GetShadowTable()
{
	PUCHAR p;
	ULONG i,curAddr;
	PSYSTEM_DESCRIPTOR_TABLE rc;
	p=(PUCHAR)KeAddSystemServiceTable;
	for(i=0;i<100;i++)
	{
		curAddr=*(PULONG)(p+i);
		__try
		{
			if(MmIsAddressValid(curAddr) && MmIsAddressValid(curAddr+sizeof(SYSTEM_SERVICE_TABLE)-1))
			{
				if(memcmp(curAddr,KeServiceDescriptorTable,sizeof(SYSTEM_SERVICE_TABLE))==0)
				{
					if(curAddr==(ULONG)KeServiceDescriptorTable)
						continue;
					return curAddr;
				}
			}
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{

		}
	}

	return NULL;
}

ULONG GetSysImageBase( PUCHAR moduleName )
{
	ULONG uImageBase;

}

NTSTATUS Ring0EnumProcess()
{
	ULONG   cbBuffer = 0x8000; //32k  
	PVOID   pSystemInfo;  
	NTSTATUS status;  
	PSYSTEM_PROCESS_INFORMATION pInfo;  

	//Ϊ���ҽ��̷����㹻�Ŀռ�  
	do   
	{  
		pSystemInfo = ExAllocatePool(NonPagedPool, cbBuffer);  
		if (pSystemInfo == NULL)    //����ռ�ʧ�ܣ�����  
		{  
			return 1;  
		}  
		status = ZwQuerySystemInformation(SystemProcessInformation, pSystemInfo, cbBuffer, NULL );  
		if (status == STATUS_INFO_LENGTH_MISMATCH) //�ռ䲻��  
		{  
			ExFreePool(pSystemInfo);  
			cbBuffer *= 2;  
		}  
		else if(!NT_SUCCESS(status))  
		{  
			ExFreePool(pSystemInfo);  
			return 1;  
		}  

	} while(status == STATUS_INFO_LENGTH_MISMATCH); //����ǿռ䲻�㣬��һֱѭ��  

	pInfo = (PSYSTEM_PROCESS_INFORMATION)pSystemInfo; //�ѵõ�����Ϣ�ŵ�pInfo��  

	for (;;)  
	{  
		LPWSTR pszProcessName = pInfo->ImageName.Buffer;  
		if (pszProcessName == NULL)  
		{  
			pszProcessName = L"NULL";  
		}  
		KdPrint(("PID:%d, process name:%S\n", pInfo->ProcessId, pszProcessName)); 

		if (pInfo->NextEntryOffset == 0) //==0��˵�������������β����  
		{  
			break;  
		}  
		pInfo = (PSYSTEM_PROCESS_INFORMATION)(((PUCHAR)pInfo) + pInfo->NextEntryOffset); //����  

	}  
	
	return STATUS_SUCCESS;  
}


ULONG GetModuleBase(PUCHAR moduleName)
{
	ULONG uSize=0x10000;
	ULONG ModulesCount=0,uImageBase=0,i;
	NTSTATUS status;
	PSYSTEM_MODULE_INFORMATION pModuleInfo;

	pModuleInfo=ExAllocatePool(NonPagedPool,uSize);
	if(pModuleInfo==NULL)
		return 0;
	status=ZwQuerySystemInformation(SystemModuleInformation,pModuleInfo,uSize,NULL);
	if(!NT_SUCCESS(status))
	{
		ExFreePool(pModuleInfo);
		return 0;
	}

	ModulesCount=pModuleInfo->ModulesCount;
	for(i=0;i<ModulesCount;i++)
	{
		PUCHAR fullName,fileName;
		fullName=pModuleInfo->Modules[i].Name;
		fileName=fullName+pModuleInfo->Modules[i].NameOffset;
		if(_stricmp(fileName,moduleName)==0)
		{
			uImageBase=pModuleInfo->Modules[i].ImageBaseAddress;
			break;
		}
	}

	ExFreePool(pModuleInfo);
	return uImageBase;
}

