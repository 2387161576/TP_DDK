#pragma once

#include "ntddk.h"
#include <WINDEF.H>
#include "ntimage.h"
#include "ADE32.H"
#include "ctl_code.h"

#define GameProcessName "DNF.EXE"
//ϵͳbuild
#define  BuildWin2000 2195
#define  BuildWin2003 3790
#define  BuildXp3 2600
#define  BuildWin7 7600
#define  LoadBase 0x400000

//SSDT
#define SSDT_INDEX_NtReadVirtualMemory 186
#define SSDT_INDEX_NtWriteVirtualMemory 277

#define TP_DeviceName L"\\Device\\TP_DDK"
#define TP_symLinkName L"\\??\\TP_DDK"
//////////////////////////////////////////////////////////////////////////
#define PAGECODE    code_seg("PAGE")   //��ҳ
#define LOCKEDCODE   code_seg()
#define INITCODE     code_seg("INIT")   //ִ�о����ڴ��б�ж��

#define PAGEDDATA    data_seg("PAGE")   //��ҳ
#define LOCKEDDATA   data_seg()
#define INITDATA     data_seg("INIT")   //ִ�о����ڴ��б�ж��

#define		SYSTEMSERVICE(ID)  KeServiceDescriptorTable->ServiceTableBase[ID]
#define        SYSTEM_SERVICE(_Func)        KeServiceDescriptorTable.ServiceTableBase[*(PULONG)((PUCHAR)_Func + 1)]
#define        SYSTEM_INDEX(_Func)                (*(PULONG)((PUCHAR)_Func + 1))
#define        IOCTL_START_PROTECTION        CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define        C_MAXPROCNUMS                        12

#define CHECK_IRQL KdPrint(("DEBUG: ---->>>>> IRQL:%X <<<<---- \n",KeGetCurrentIrql ()));

//////////////////////////////////////////////////////////////////////////


/************************************************************************/
/* TypeDef                                                                     */
/************************************************************************/
typedef ULONG   DWORD;

typedef struct _SYSTEM_SERVICE_TABLE
{
	PULONG  ServiceTableBase;  // array of entry points
	PVOID  CounterTable;  // array of usage counters
	ULONG  ServiceLimit;    // number of table entries
	UCHAR*  ArgumentTable;  // array of byte counts
}SYSTEM_SERVICE_TABLE,*PSYSTEM_SERVICE_TABLE,**PPSYSTEM_SERVICE_TABLE;

typedef struct _SERVICE_DESCRIPTOR_TABLE
{
	SYSTEM_SERVICE_TABLE ntoskrnl;  // ntoskrnl.exe ( native api )
	SYSTEM_SERVICE_TABLE win32k;    // win32k.sys (gdi/user support)
	SYSTEM_SERVICE_TABLE Table3;    // not used
	SYSTEM_SERVICE_TABLE Table4;    // not used
}
SYSTEM_DESCRIPTOR_TABLE,*PSYSTEM_DESCRIPTOR_TABLE,**PPSYSTEM_DESCRIPTOR_TABLE;

//typedef struct _ServiceDescriptorTable
//{
//	PVOID ServiceTableBase; //System Service Dispatch Table �Ļ���ַ 
//	PVOID ServiceCounterTable ;//������ SSDT ��ÿ�����񱻵��ô����ļ����������������һ����sysenter ���¡�
//	unsigned int NumberOfServices;//�� ServiceTableBase �����ķ������Ŀ�� 
//	PVOID ParamTableBase;//����ÿ��ϵͳ��������ֽ�����Ļ���ַ-ϵͳ��������� 
//} *PServiceDescriptorTable;


typedef enum _SYSTEM_INFORMATION_CLASS {  
	SystemBasicInformation,  
	SystemProcessorInformation,  
	SystemPerformanceInformation,  
	SystemTimeOfDayInformation,  
	SystemPathInformation,  
	SystemProcessInformation, //5  
	SystemCallCountInformation,  
	SystemDeviceInformation,  
	SystemProcessorPerformanceInformation,  
	SystemFlagsInformation,  
	SystemCallTimeInformation,  
	SystemModuleInformation,  
	SystemLocksInformation,  
	SystemStackTraceInformation,  
	SystemPagedPoolInformation,  
	SystemNonPagedPoolInformation,  
	SystemHandleInformation,  
	SystemObjectInformation,  
	SystemPageFileInformation,  
	SystemVdmInstemulInformation,  
	SystemVdmBopInformation,  
	SystemFileCacheInformation,  
	SystemPoolTagInformation,  
	SystemInterruptInformation,  
	SystemDpcBehaviorInformation,  
	SystemFullMemoryInformation,  
	SystemLoadGdiDriverInformation,  
	SystemUnloadGdiDriverInformation,  
	SystemTimeAdjustmentInformation,  
	SystemSummaryMemoryInformation,  
	SystemNextEventIdInformation,  
	SystemEventIdsInformation,  
	SystemCrashDumpInformation,  
	SystemExceptionInformation,  
	SystemCrashDumpStateInformation,  
	SystemKernelDebuggerInformation,  
	SystemContextSwitchInformation,  
	SystemRegistryQuotaInformation,  
	SystemExtendServiceTableInformation,  
	SystemPrioritySeperation,  
	SystemPlugPlayBusInformation,  
	SystemDockInformation,  
	SystemPowerInformation2,  
	SystemProcessorSpeedInformation,  
	SystemCurrentTimeZoneInformation,  
	SystemLookasideInformation  
} SYSTEM_INFORMATION_CLASS, *PSYSTEM_INFORMATION_CLASS;  

typedef struct _SYSTEM_THREAD_INFORMATION {  
	LARGE_INTEGER           KernelTime;  
	LARGE_INTEGER           UserTime;  
	LARGE_INTEGER           CreateTime;  
	ULONG                   WaitTime;  
	PVOID                   StartAddress;  
	CLIENT_ID               ClientId;  
	KPRIORITY               Priority;  
	LONG                    BasePriority;  
	ULONG                   ContextSwitchCount;  
	ULONG                   State;  
	KWAIT_REASON            WaitReason;  
}SYSTEM_THREAD_INFORMATION, *PSYSTEM_THREAD_INFORMATION; 

typedef struct _SYSTEM_PROCESS_INFORMATION {  
	ULONG                   NextEntryOffset;  
	ULONG                   NumberOfThreads;  
	LARGE_INTEGER           Reserved[3];  
	LARGE_INTEGER           CreateTime;  
	LARGE_INTEGER           UserTime;  
	LARGE_INTEGER           KernelTime;  
	UNICODE_STRING          ImageName;  
	KPRIORITY               BasePriority;  
	HANDLE                  ProcessId;  
	HANDLE                  InheritedFromProcessId;  
	ULONG                   HandleCount;  
	ULONG                   Reserved2[2];  
	ULONG                   PrivatePageCount;  
	VM_COUNTERS             VirtualMemoryCounters;  
	IO_COUNTERS             IoCounters;  
	SYSTEM_THREAD_INFORMATION           Threads[0];  
} SYSTEM_PROCESS_INFORMATION, *PSYSTEM_PROCESS_INFORMATION;  

typedef struct _SYSTEM_MODULE {
	ULONG                Reserved1;
	ULONG                Reserved2;
	PVOID                ImageBaseAddress;
	ULONG                ImageSize;
	ULONG                Flags;
	WORD                 Id;
	WORD                 Rank;
	WORD                 w018;
	WORD                 NameOffset;
	BYTE                 Name[MAXIMUM_FILENAME_LENGTH];
} SYSTEM_MODULE, *PSYSTEM_MODULE;

typedef struct _SYSTEM_MODULE_INFORMATION {
	ULONG                ModulesCount;
	SYSTEM_MODULE        Modules[];
} SYSTEM_MODULE_INFORMATION, *PSYSTEM_MODULE_INFORMATION;


typedef struct _LOADED_KERNEL_INFO
{
	PVOID OriginalKernelBase;
	PVOID NewKernelBase;
	PVOID NewPsdt;
	PKEVENT NotifyEvent;
	LONG LoadedStatus;
} LOADED_KERNEL_INFO, *PLOADED_KERNEL_INFO;

typedef
	NTSTATUS 
	(NTAPI*	_QuerySystemInformation)( 
	IN SYSTEM_INFORMATION_CLASS SystemInformationClass, 
	IN OUT PVOID SystemInformation, 
	IN ULONG SystemInformationLength, 
	OUT PULONG ReturnLength OPTIONAL 
	); 

//////////////////////////////Global variable ///////////////////////////////////////////////
extern PSYSTEM_SERVICE_TABLE KeServiceDescriptorTable;
extern PSYSTEM_DESCRIPTOR_TABLE KeServiceDescriptorTableShadow;

 //����NtOpenProcess��ԭ��
typedef NTSTATUS (__stdcall *NTOPENPROCESS)(
	OUT		PHANDLE ProcessHandle,
	IN      ACCESS_MASK DesiredAccess,
	IN      POBJECT_ATTRIBUTES ObjectAttributes,
	IN		PCLIENT_ID ClientId
	);

EXTERN_C __declspec(dllimport) NTSTATUS NtOpenThread(
	OUT  PHANDLE ThreadHandle,
	IN   ACCESS_MASK DesiredAccess,
	IN   POBJECT_ATTRIBUTES ObjectAttributes,
	IN   PCLIENT_ID ClientId
	);

 
EXTERN_C NTKERNELAPI
	BOOLEAN
	KeAddSystemServiceTable(
	IN PULONG_PTR Base,
	IN PULONG Count OPTIONAL,
	IN ULONG Limit,
	IN PUCHAR Number,
	IN ULONG Index
	);

EXTERN_C NTSYSAPI NTSTATUS NTAPI ZwQuerySystemInformation(   
	IN ULONG SystemInformationClass,   
	IN PVOID SystemInformation,   
	IN ULONG SystemInformationLength,   
	OUT PULONG ReturnLength); 

//NTKERNELAPI
//	VOID
//	KeStackAttachProcess (
//	IN PRKPROCESS PROCESS,
//	OUT PRKAPC_STATE ApcState
//	);

NTSYSAPI 
	PIMAGE_NT_HEADERS
	NTAPI
	RtlImageNtHeader(IN PVOID ModuleAddress );


UCHAR *
	PsGetProcessImageFileName(
	__in PEPROCESS Process
	);
  
 //PVOID NTAPI RtlImageDirectoryEntryToData(
	//PVOID 	BaseAddress,
	//BOOLEAN MappedAsImage,
	//USHORT 	Directory,
	//PULONG 	Size 
	//);	

typedef struct _PROCESS_INFO {   
	DWORD   dwProcessId ;   
	PUCHAR  pImageFileName ;   
} PROCESS_INFO, *PPROCESS_INFO ;

#define EPROCESS_SIZE     1  
#define PEB_OFFSET          2  
#define FILE_NAME_OFFSET        3  
#define PROCESS_LINK_OFFSET     4  
#define PROCESS_ID_OFFSET       5  
#define EXIT_TIME_OFFSET        6 
#define DebugPort_OFFSET    7
#define PROCESS_ObjectTable_OFFSET 8
ULONG GetPlantformDependentInfo ( ULONG dwFlag );

//����5�ֽڴ���Ľṹ 
#pragma pack(1) 
typedef struct _TOP5CODE 
{ 
	UCHAR instruction; //ָ��
	ULONG address; //��ַ 
}TOP5CODE,*PTOP5CODE; 
#pragma pack()

//////////////////////////////////////////////////////////////////////
//  ����:  MyGetFunAddress
//  ����:  ��ȡ������ַ
//  ����:  ���������ַ���ָ��
//  ����:  ������ַ
//////////////////////////////////////////////////////////////////////
ULONG MyGetFunAddress( IN PCWSTR FunctionName);


//ring0 ��������
NTSTATUS Ring0EnumProcess();

//�Ƿ���PAE
BOOLEAN isPaeOpened();


//��ȡģ�����ַ
ULONG GetModuleBase(PUCHAR moduleName);

///////////////////////////////////////////////////////////////////////////////   
//  ö�ٽ��̡�������ͨ��EPROCESS�ṹ��ActiveProcessLinks����<BR>// ���������ʵ����ȫ�ֱ���PsActiveProcessHead��ָʾ������    
///////////////////////////////////////////////////////////////////////////////   
void EnumProcessList ();

//ͨ���������ƻ�ȡ_eprocess�ṹָ��
ULONG GetProcessByName(PUCHAR pName);


// Qualifier:��ȡSSDT������ǰ��ַ
ULONG* GetSSDT_CurrAddr(void* func);

//��ȡԭʼSSDT���ϵ�ԭʼ����RVAֵ
ULONG GetOrgSSdtFuncRVA(ULONG index,PLOADED_KERNEL_INFO plki);

PSYSTEM_DESCRIPTOR_TABLE GetShadowTable();

//��ȡϵͳ�汾��
ULONG GetVersion();

//��EAT�ж�λ��ָ������,MmGetSystemRoutineAddressʵ�ʵ��õ�MiFindExportedRoutineByName
PVOID MiFindExportedRoutineByName (IN PVOID DllBase,IN PANSI_STRING AnsiImageRoutineName);

ULONG  GetOriginalKernelBase();

//�жϵ�ǰ�����Ƿ�ΪDNF.exe
BOOLEAN  IsDnfProcess();

//ȥ��ҳ�汣��
void DisableWP();

 //�ָ�ҳ�汣��
void EnableWP();

//************************************
// Method:    CreateMyDevice
// FullName:  CreateMyDevice
// Access:    public 
// Returns:   NTSTATUS
// Qualifier:�����豸
// Parameter: IN PDRIVER_OBJECT pDrvierObj
//************************************
NTSTATUS CreateMyDevice(IN PDRIVER_OBJECT pDrvierObj);

//����hook����
ULONG GetCodeLength(IN PVOID desFunc,IN ULONG NeedLengh);

//�ҹ�Function������FakeFunction
//JmpBuffer��FakeFunction��ת��Function����ת������
BOOLEAN HookFunction(PVOID Function, PVOID FakeFunction, PUCHAR JmpBuffer);

BOOLEAN UnhookFunction(PVOID Function, PUCHAR JmpBuffer);

//����ʵ�ֵ���ת
void WriteJmp( PVOID Function,PVOID fakeFunction,PUCHAR JmpBuffer );


NTSTATUS GetModuleInfo(
	IN char*	chModName,
	OUT PSYSTEM_MODULE_INFORMATION	psmi);

NTSTATUS LoadKernelFile(OUT PLOADED_KERNEL_INFO plki);

//���ڴ���뷽ʽ��pFileBuffer���ص��ڴ���
BOOL ImageFile(PBYTE pFileBuffer,BYTE **ImageModuleBase);
