
#include <ntddk.h>
#include "util.h"
#include "hook.h"


void DDK_Unload(IN PDRIVER_OBJECT pDRIVER_OBJECT);
NTSTATUS  Comm_Create(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS  Comm_Close(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS Comm_Default(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp);
NTSTATUS Comm_IoControl(PDEVICE_OBJECT DeviceObject, PIRP Irp);
NTSTATUS COMM_DirectOutIo(PIRP Irp, PIO_STACK_LOCATION pIoStackIrp, UINT *sizeofWrite);
NTSTATUS COMM_DirectInIo(PIRP Irp, PIO_STACK_LOCATION pIoStackIrp, UINT *sizeofWrite);
NTSTATUS COMM_BufferedIo(PIRP Irp, PIO_STACK_LOCATION pIoStackIrp, UINT *sizeofWrite);
NTSTATUS COMM_NeitherIo(PIRP Irp, PIO_STACK_LOCATION pIoStackIrp, UINT *sizeofWrite);
NTSTATUS COMM_TP_DDK(PIRP Irp, PIO_STACK_LOCATION pIoStackIrp, UINT *sizeofWrite);


NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObj,PUNICODE_STRING B)
{

	KdPrint(("TP_DDK Entry:....\n"));


	pDriverObj->DriverUnload=DDK_Unload;
	//ע����ǲ����
	pDriverObj->MajorFunction[IRP_MJ_CREATE]=Comm_Create; 
	pDriverObj->MajorFunction[IRP_MJ_CLOSE]=Comm_Close;
	pDriverObj->MajorFunction[IRP_MJ_READ]=Comm_Default;
	pDriverObj->MajorFunction[IRP_MJ_DEVICE_CONTROL]=Comm_IoControl;

	if(!NT_SUCCESS(CreateMyDevice(pDriverObj))) //������Ӧ���豸
		KdPrint(("�����豸ʧ��..\n"));
	//Inline_Hook_NtOpenProcess();
	//handleObjectHook(TRUE);
	//PsSetLoadImageNotifyRoutine(LoadImageNotifyRoutine); //��������ӳ��ص�
	InitAfterTp();

	return STATUS_SUCCESS;
}


void DDK_Unload(IN PDRIVER_OBJECT pDRIVER_OBJECT)
{
	UNICODE_STRING symLinkName;
	if(pDRIVER_OBJECT->DeviceObject!=NULL)
		IoDeleteDevice(pDRIVER_OBJECT->DeviceObject); //ɾ���豸
	RtlInitUnicodeString(&symLinkName,L"\\??\\TP_DDK");	//ɾ��������������
	IoDeleteSymbolicLink(&symLinkName);
	KdPrint(("�豸ж�سɹ�...\n"));
	KdPrint(("TP_DDK ����ж�سɹ�...\n"));
}

NTSTATUS Comm_IoControl(IN PDEVICE_OBJECT pDevobj,IN PIRP pIrp	)
{
	NTSTATUS status = STATUS_NOT_SUPPORTED;
	PIO_STACK_LOCATION irpStack = NULL;
	UINT sizeofWrite = 0;

	//KdPrint(("Comm_IoControl\n"));
	irpStack=IoGetCurrentIrpStackLocation(pIrp); //�õ���ǰջָ��

	if(irpStack)
	{
		switch(irpStack->Parameters.DeviceIoControl.IoControlCode)
		{
			case IOCTL_COMM_DIRECT_IN_IO:	//ֱ�����뻺�����I/O(METHOD_IN_DIRECT)
				status=COMM_DirectInIo(pIrp,irpStack,&sizeofWrite);
				break;

			case  IOCTL_COMM_DIRECT_OUT_IO:	//��������ֱ�����I/O(METHOD_OUT_DIRECT)
				status=COMM_DirectOutIo(pIrp,irpStack,&sizeofWrite);
				break;

			case  IOCTL_COMM_BUFFERED_IO:	//�����������I/O(METHOD_BUFFERED)
				status=COMM_BufferedIo(pIrp,irpStack,&sizeofWrite);
				break;

			case IOCTL_COMM_NEITHER_IO:		//�������ַ���������(METHOD_NEITHER)
				status=COMM_NeitherIo(pIrp,irpStack,&sizeofWrite);
				break;

			case IOCTL_TP_DDK_ENABLE:
				status=COMM_TP_DDK(pIrp,irpStack,&sizeofWrite);
				break;
		}
	}

	/*
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
					//__asm int 3
					//KdPrint(("TP_DDK_Enable_Code:\n"));
					
					InitAfterTp();
					FkNtOpenProcss(TRUE);
					FkNtOpenThread(TRUE);
					FkCRC(TRUE);
					//FkDebugReset(TRUE);
					ReSumeKiAttachProcess();

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
	*/
	//����Ӧ��IPR���д���

	pIrp->IoStatus.Information=sizeofWrite;
	pIrp->IoStatus.Status=status;
	IoCompleteRequest(pIrp,IO_NO_INCREMENT);//ָʾ��ɴ�IRP
	return status; 
}

NTSTATUS Comm_Close(PDEVICE_OBJECT pDevObj, PIRP pIrp)
{
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS Comm_Create(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp)
{
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	return pIrp->IoStatus.Status;
}

NTSTATUS Comm_Default(IN PDEVICE_OBJECT pDevObj, IN PIRP pIrp)
{
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);

	return pIrp->IoStatus.Status;
}


NTSTATUS COMM_DirectInIo(PIRP Irp, PIO_STACK_LOCATION pIoStackIrp, UINT *sizeofWrite)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PVOID pInputBuffer, pOutputBuffer;
	ULONG  outputLength, inputLength;

	outputLength=pIoStackIrp->Parameters.DeviceIoControl.OutputBufferLength;
	inputLength=pIoStackIrp->Parameters.DeviceIoControl.InputBufferLength;
	pInputBuffer=Irp->AssociatedIrp.SystemBuffer;
	pOutputBuffer = NULL;

	if(Irp->MdlAddress)
		pOutputBuffer=MmGetSystemAddressForMdlSafe(Irp->MdlAddress,NormalPagePriority);
	
	if(pOutputBuffer && pOutputBuffer)
	{
		//KdPrint(("IOCTL_COMM_DIRECT_IN_IO-> UserModeMessage = %s \n", pInputBuffer));
		//RtlCopyMemory(pOutputBuffer,pInputBuffer,outputLength);
		*sizeofWrite = outputLength;

		InitAfterTp();
		status = STATUS_SUCCESS;
	}

	return status;
}

NTSTATUS COMM_DirectOutIo(PIRP Irp, PIO_STACK_LOCATION pIoStackIrp, UINT *sizeofWrite)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PVOID pInputBuffer, pOutputBuffer;
	ULONG  outputLength, inputLength;

	//KdPrint(("COMM_DirectOutIo\r\n"));

	outputLength = pIoStackIrp->Parameters.DeviceIoControl.OutputBufferLength;
	inputLength  = pIoStackIrp->Parameters.DeviceIoControl.InputBufferLength;
	pInputBuffer = Irp->AssociatedIrp.SystemBuffer;
	pOutputBuffer = NULL;

	if(Irp->MdlAddress)
		pOutputBuffer = MmGetSystemAddressForMdlSafe(Irp->MdlAddress, NormalPagePriority);

	if(pInputBuffer && pOutputBuffer)
	{                                                          
		KdPrint(("COMM_DirectOutIo UserModeMessage = '%s'", pInputBuffer));
		RtlCopyMemory(pOutputBuffer, pInputBuffer, outputLength);
		*sizeofWrite = outputLength;
		status = STATUS_SUCCESS;
	}
	return status;
}

NTSTATUS COMM_BufferedIo(PIRP Irp, PIO_STACK_LOCATION pIoStackIrp, UINT *sizeofWrite)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PVOID pInputBuffer, pOutputBuffer;
	ULONG  outputLength, inputLength;

	KdPrint(("COMM_BufferedIo\r\n"));

	outputLength = pIoStackIrp->Parameters.DeviceIoControl.OutputBufferLength;
	inputLength  = pIoStackIrp->Parameters.DeviceIoControl.InputBufferLength;
	pInputBuffer = Irp->AssociatedIrp.SystemBuffer;
	pOutputBuffer = Irp->AssociatedIrp.SystemBuffer;

	if(pInputBuffer && pOutputBuffer)
	{              
		KdPrint(("COMM_BufferedIo UserModeMessage = '%s'", pInputBuffer));
		//RtlCopyMemory(pOutputBuffer, pInputBuffer, outputLength);
		*sizeofWrite = outputLength;


		status = STATUS_SUCCESS;
	}
	return status;
}

NTSTATUS COMM_NeitherIo(PIRP Irp, PIO_STACK_LOCATION pIoStackIrp, UINT *sizeofWrite)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PVOID pInputBuffer, pOutputBuffer;
	ULONG  outputLength, inputLength;

	KdPrint(("COMM_NeitherIo\r\n"));

	outputLength  = pIoStackIrp->Parameters.DeviceIoControl.OutputBufferLength;
	inputLength   = pIoStackIrp->Parameters.DeviceIoControl.InputBufferLength;
	pInputBuffer  = pIoStackIrp->Parameters.DeviceIoControl.Type3InputBuffer;
	pOutputBuffer = Irp->UserBuffer;

	if(pInputBuffer && pOutputBuffer)
	{              
		KdPrint(("COMM_NeitherIo UserModeMessage = '%s'", pInputBuffer));
		RtlCopyMemory(pOutputBuffer, pInputBuffer, outputLength);
		*sizeofWrite = outputLength;
		status = STATUS_SUCCESS;
	}
	return status;
}

NTSTATUS COMM_TP_DDK(PIRP Irp, PIO_STACK_LOCATION pIoStackIrp, UINT *sizeofWrite)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PVOID pInputBuffer, pOutputBuffer;
	ULONG  outputLength, inputLength;

	//KdPrint(("COMM_TP_DDK\r\n"));

	status = STATUS_SUCCESS;
	
	return status;
}