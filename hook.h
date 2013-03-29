#pragma once
#include "ntddk.h"

typedef NTSTATUS (*OB_SECURITY_METHOD)(
	IN PVOID Object,
	IN SECURITY_OPERATION_CODE OperationCode,
	IN PSECURITY_INFORMATION SecurityInformation,
	IN OUT PSECURITY_DESCRIPTOR SecurityDescriptor,
	IN OUT PULONG CapturedLength,
	IN OUT PSECURITY_DESCRIPTOR *ObjectsSecurityDescriptor,
	IN POOL_TYPE PoolType,
	IN PGENERIC_MAPPING GenericMapping,
	IN ULONG unk
	);



typedef NTSTATUS (__stdcall* ObOpenObjectByPointer)(
	IN   PVOID Object,
	IN      ULONG HandleAttributes,
	IN  PACCESS_STATE PassedAccessState,
	IN     ACCESS_MASK DesiredAccess,
	IN  POBJECT_TYPE ObjectType,
	IN     KPROCESSOR_MODE AccessMode,
	OUT    PHANDLE Handle
	);

////ȫ�ֱ�����
//typedef struct _Gloabal_Var {
//	ULONG addrNtReadVirtualMemory;
//	ULONG addrNtWriteVirtualMemory;
//	ULONG addrNtReadVirtualMemoryOffset;
//	ULONG addrNtWriteVirtualMemoryOffset;
//} global_Var;


 POBJECT_TYPE DbgkDebugObjectType;

void handleObjectHook(BOOLEAN bHook);

//����image�Ļص�����
void LoadImageNotifyRoutine(IN PUNICODE_STRING FullName,
	IN HANDLE ProcessId,
	IN PIMAGE_INFO ImageInfo);


//������TPǰ����ʼ��(��ȡδ��hook��ϵͳ��Ϣ)
void InitBeforeTP();
//������TP������(��ȡTPԭʼ����)
void InitAfterTp();
//����ж��
void TP_DDK_Unload();

//�ָ�TP��KiAttachProcess //XP3
void ReSumeKiAttachProcess();

//�ƹ�TP��NtOpenProcess
void FkNtOpenProcss();

//TP NtOpenProcess��ת�����㺯��
void __stdcall JmpNtOpenProcess(BOOLEAN bFk);

//�ƹ�TP��NtOpenThread;
void FkNtOpenThread();

//TP NtOpenThread��ת�����㺯��
void __stdcall JmpNtOpenThread(BOOLEAN bFk);

//�ƹ�TP��NtReadVirtualMemory
void FkNtReadVirtualMemory(BOOLEAN bFk);

void __stdcall FakeNtReadVirtualMemory();


//�ƹ�TP��NtWriteVirtualMemory
void FkNtWriteVirtualMemory(BOOLEAN bFk);

void __stdcall FakeNtWriteVirtualMemory();

//#define crc_jmp 0xcf6f6 //VM���crc��⣬jmpƫ�ƣ�
#define AddrCRC1 0x1630
#define AddrCRC2 0x4082
#define AddrCRC3 0xd1a85
#define DebugPortReset1 0x2228
#define DebugPortReset2 0x6EA8
#define DebugPortCheck1 0xba4ca
#define DebugPortPop 0xbb0f0
#define NtGetContextThread_SSDT_INDEX 85 
//CRCУ��
void FkCRC(BOOLEAN bFk);
void __stdcall FakeCRC3();

//Debug����
void FkDebugReset(BOOLEAN bFk);
void __stdcall FakeDebugPort();
void __stdcall FakeDebugPortCheck();
void __stdcall FakeDebugPortPop();

//Ӳ���ϵ�
//��Context��Ӳ���ϵ�Ĵ�������
void FkHardBreakPoint(BOOLEAN bFk);
void ZeroHardBreakPoint(PCONTEXT pContext,KPROCESSOR_MODE Mode);
void _stdcall FakeNtGetContextThread();
