
#include <ntddk.h>
#include "util.h"
#include "Inline_Hook_NtOpenProcess.h"
#include "hook.h"


void DDK_Unload(IN PDRIVER_OBJECT pDRIVER_OBJECT);
NTSTATUS ddk_DispatchRoutine_CONTROL(IN PDEVICE_OBJECT pDevobj,IN PIRP pIrp	);
NTSTATUS DispatchClose(PDEVICE_OBJECT pDevObj, PIRP pIrp);

#pragma INITCODE
NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObj,PUNICODE_STRING B)
{

	KdPrint(("TP_DDK Entry:....\n"));
	pDriverObj->DriverUnload=DDK_Unload;
	//ע����ǲ����
	pDriverObj->MajorFunction[IRP_MJ_CREATE]=ddk_DispatchRoutine_CONTROL; //IRP_MJ_CREATE���IRP������
	pDriverObj->MajorFunction[IRP_MJ_CLOSE]=DispatchClose; //IRP_MJ_CREATE���IRP������
	pDriverObj->MajorFunction[IRP_MJ_READ]=ddk_DispatchRoutine_CONTROL; //IRP_MJ_CREATE���IRP������
	pDriverObj->MajorFunction[IRP_MJ_CLOSE]=ddk_DispatchRoutine_CONTROL; //IRP_MJ_CREATE���IRP������
	pDriverObj->MajorFunction[IRP_MJ_DEVICE_CONTROL]=ddk_DispatchRoutine_CONTROL; //IRP_MJ_CREATE���		IRP������
	
	CreateMyDevice(pDriverObj);//������Ӧ���豸

	
	//Inline_Hook_NtOpenProcess();
	//handleObjectHook(TRUE);
	//PsSetLoadImageNotifyRoutine(LoadImageNotifyRoutine); //��������ӳ��ص�
	InitBeforeTP();
 	KdPrint(("TesSafe: %x\n",GetModuleBase("tessafe.sys")));
	FkCRC(TRUE);
	return STATUS_SUCCESS;
}


void DDK_Unload(IN PDRIVER_OBJECT pDRIVER_OBJECT)
{
	PDEVICE_OBJECT pDev;
	UNICODE_STRING symLinkName;
	FkCRC(FALSE);
	//TP_DDK_Unload();
	//Inline_unHook_NtOpenProcess();
	//handleObjectHook(FALSE);
	//PsRemoveLoadImageNotifyRoutine(LoadImageNotifyRoutine); //ж��ӳ��ص�
	

	pDev=pDRIVER_OBJECT->DeviceObject;
	IoDeleteDevice(pDev); //ɾ���豸
	RtlInitUnicodeString(&symLinkName,TP_symLinkName);	//ɾ��������������
	IoDeleteSymbolicLink(&symLinkName);
	KdPrint(("�豸ж�سɹ�...\n"));
	KdPrint(("TP_DDK ����ж�سɹ�...\n"));
}

NTSTATUS ddk_DispatchRoutine_CONTROL(IN PDEVICE_OBJECT pDevobj,IN PIRP pIrp	)
{
	ULONG info,mf;
	PIO_STACK_LOCATION stack;
	info=0;
	stack=IoGetCurrentIrpStackLocation(pIrp); //�õ���ǰջָ��
	mf=stack->MajorFunction;
	switch(mf)
	{
	case IRP_MJ_DEVICE_CONTROL:
		{
			NTSTATUS status=STATUS_SUCCESS;
			//�õ����뻺������С
			ULONG cbin=stack->Parameters.DeviceIoControl.InputBufferLength;
			//�õ������������С
			ULONG cbout=stack->Parameters.DeviceIoControl.OutputBufferLength;
			//�õ�IOCTL��
			ULONG code=stack->Parameters.DeviceIoControl.IoControlCode;

			switch(code)
			{
			case TEST_1:
				{
					int x,y;
					int *inBuffer=(int*)pIrp->AssociatedIrp.SystemBuffer;
					int *outBuffer=(int*)MmGetSystemAddressForMdlSafe(
						pIrp->MdlAddress,NormalPagePriority);
					//����Ƿ�ɶ��쳣
					//ProbeForRead(inBuffer,cbin,__alignof(int));
					//��ȡ����buffer
					x=inBuffer[0];
					y=inBuffer[1];

					//����Ƿ��д�쳣
					//ProbeForWrite(OutBuffer,cbout,__alignof(int));
					//��������û���	
					outBuffer[0]=x+y;
					KdPrint(("Call->add\n"));
					KdPrint(("x=%d,y=%d \n",x,y));
					break;
				}
			case Inline_NtOpenProcess_Hook_Code:
				{
					int pid=0;
					int* inBuffer=(int*)pIrp->AssociatedIrp.SystemBuffer;
					
					KdPrint(("PID:%d",*inBuffer));
					Inline_NtOpenProcess_Id=NULL;
					Inline_NtOpenProcess_Id=(HANDLE)(*inBuffer);
					break;
				}
			case TP_DDK_Enable_Code:
				{
					KdPrint(("TP_DDK_Enable_Code:\n"));
					ReSumeKiAttachProcess();
					FkNtOpenProcss(TRUE);
					FkNtOpenThread(TRUE);
					FkNtReadVirtualMemory(TRUE);
					FkNtWriteVirtualMemory(TRUE);
					break;
				}
			}

			break;
		}
	case IRP_MJ_CREATE:
		{
			break;
		}
	case  IRP_MJ_CLOSE:
		{
			KdPrint(("CLose Device...++++\n"));
			break;
		}
	case IRP_MJ_READ:
		{
			break;
		}
	}

	//����Ӧ��IPR���д���
	pIrp->IoStatus.Information=info; //���ò������ֽ���Ϊ0��������ʵ������
	pIrp->IoStatus.Status=STATUS_SUCCESS;//���سɹ�
	IoCompleteRequest(pIrp,IO_NO_INCREMENT);//ָʾ��ɴ�IRP
	return STATUS_SUCCESS; 
}

NTSTATUS DispatchClose(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	KdPrint (("DispatchClose \n"));
	return STATUS_SUCCESS;
}