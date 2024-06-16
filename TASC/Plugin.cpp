
#include "Plugin.h"

int g_iReverser = 1;
int g_iPowerNotch = 0;
int g_iBrakeNotch = 0;

//TASC ����
int stopPosition = 0; //������ġ (M)
bool tasc = false; //tasc ���� ����
bool tascEnabled = false; //tasc ���� ���� ����
bool tascKeyCode = false; //tasc ���� Ű �Է� ����
const double DECELERATION_RATE = 4.5 / 3.6; // km/h/s�� m/s^2�� ��ȯ

//ATC ����
float atcLimitSpeed = 130; //atc �ְ�ӵ� ����
int atcCode = 0; //atc �ӵ��ڵ�

ATS_API void WINAPI Load()
{
	AllocConsole();
	freopen("CONOUT$", "wt", stdout);
	printf("�÷������� �ҷ��Խ��ϴ�.\n"); //console only
}

ATS_API void WINAPI Dispose()
{
	printf("�÷������� Unload �Ǿ����ϴ�.\n"); //console only
}

ATS_API int WINAPI GetPluginVersion()
{
	return ATS_VERSION;
}

ATS_API void WINAPI SetVehicleSpec(ATS_VEHICLESPEC VehicleSpec)
{
	printf("���� ������ �ҷ��Խ��ϴ�.\n"); //console only
}

ATS_API void WINAPI Initialize(int iHandle)
{
	printf("Initialize : %d\n", iHandle); //console only
}

//ATC ���� ����
bool brake7 = false;
bool brakeN = false;

ATS_API ATS_HANDLES WINAPI Elapse(ATS_VEHICLESTATE VehicleState, int* pPanel, int* pSound)
{
	ATS_HANDLES Output;
	Output.Reverser = g_iReverser;
	Output.Power = g_iPowerNotch;
	Output.Brake = g_iBrakeNotch;

	//tasc �������� ����
	//tasc ���������� �����ϸ�, tasc Ű�� Ȱ��ȭ �Ǿ������� tasc�� �۵�, ��Ȱ��ȭ �Ǿ������� tasc ���۵�.
	if (tascEnabled) {
		if (tascKeyCode) {
			tasc = true;
			pPanel[0] = 1;
		} else if (!tascKeyCode) {
			tasc = false;
			pPanel[0] = 0;
		}
	}

	//tasc ����
	if (tasc) {
		double currentSpeed = VehicleState.Speed / 3.6;
		double currentPosition = VehicleState.Location;
		double distanceToStop = stopPosition - currentPosition;

		if (distanceToStop <= 0) {
			g_iPowerNotch = 0;
			g_iBrakeNotch = 7;
		}
		else {
			double requiredDeceleration = (currentSpeed * currentSpeed) / (2 * distanceToStop);

			if (requiredDeceleration >= DECELERATION_RATE * 0.875) {
				g_iBrakeNotch = 7;
			}
			else if (requiredDeceleration >= DECELERATION_RATE * 0.75) {
				g_iBrakeNotch = 6;
			}
			else if (requiredDeceleration >= DECELERATION_RATE * 0.625) {
				g_iBrakeNotch = 5;
			}
			else if (requiredDeceleration >= DECELERATION_RATE * 0.5) {
				g_iBrakeNotch = 4;
			}
			else if (requiredDeceleration >= DECELERATION_RATE * 0.375) {
				g_iBrakeNotch = 3;
			}
			else if (requiredDeceleration >= DECELERATION_RATE * 0.25) {
				g_iBrakeNotch = 2;
			}
			else if (requiredDeceleration >= DECELERATION_RATE * 0.125) {
				g_iBrakeNotch = 1;
			}
			else {
				g_iBrakeNotch = 0;
			}
			g_iPowerNotch = 0;
		}
		Output.Power = g_iPowerNotch;
		Output.Brake = g_iBrakeNotch;

		if (VehicleState.Speed == 0.0f) {
			g_iBrakeNotch = 5; //5�� ����
			tasc = false;
			tascEnabled = false;
			pPanel[0] = 0;
			printf("������ �����Ͽ� TASC�� ���� �˴ϴ�.\n"); //console only
		}
	}

	//atc ����
	if (VehicleState.Speed > atcLimitSpeed) {
		brake7 = true;
		if (brake7) {
			g_iBrakeNotch = 7;
		}
	} else {
		if (brake7) {
			brake7 = false;
			brakeN = true;
		}
		if (brakeN) {
			g_iBrakeNotch = 0;
			brakeN = false;
		}
	}

	pPanel[1] = atcCode;

	return Output;
}

ATS_API void WINAPI SetBeaconData(ATS_BEACONDATA BeaconData) {
	//�絿��
	if (BeaconData.Optional == 11101) {
		stopPosition = 2550;
		printf("���� ���� ��ġ�� %d�� ���� �Ǿ����ϴ�.\n", stopPosition); //console only
	}

	//��ϼ���
	if (BeaconData.Optional == 10102) {
		tascEnabled = true;
	} else if (BeaconData.Optional == 11102) {
		stopPosition = 3425;
		printf("���� ���� ��ġ�� %d�� ���� �Ǿ����ϴ�.\n", stopPosition); //console only
	}
}

ATS_API void WINAPI SetPower(int iNotch){
	g_iPowerNotch = iNotch;
}

ATS_API void WINAPI SetBrake(int iNotch){
	g_iBrakeNotch = iNotch;
}

ATS_API void WINAPI SetReverser(int iReverser){
	g_iReverser = iReverser;
}

ATS_API void WINAPI KeyDown(int keyCode) {
	//KeyCode 2 is Delete Key
	if (keyCode == 2) {
		if (tascKeyCode) {
			tascKeyCode = false;
			printf("TASC�� ��Ȱ��ȭ �Ǿ����ϴ�.\n�������� �����ؾ� �մϴ�.\n"); //console only
		} else if (!tascKeyCode) {
			tascKeyCode = true;
			printf("TASC�� Ȱ��ȭ �Ǿ����ϴ�.\n���� �����ϱ� 500m ������ ������ ���� ���� TASC�� �����մϴ�.\n"); //console only
		}
	}
}

ATS_API void WINAPI KeyUp(){}

ATS_API void WINAPI HornBlow(){}

ATS_API void WINAPI DoorOpen(){}

ATS_API void WINAPI DoorClose(){}

ATS_API void WINAPI SetSignal(int Signal, int* pSound) {
	//ATC ����
	if (Signal == 0) {
		atcLimitSpeed = 0.0f;
		printf("ATC �ְ�ӵ�: %.1f\n", atcLimitSpeed);
		atcCode = 0;
		pSound[0];
	} else if (Signal == 1) {
		atcLimitSpeed = 15.0f;
		printf("ATC �ְ�ӵ�: %.1f\n", atcLimitSpeed);
		atcCode = 15;
		pSound[0];
	} else if (Signal == 2) {
		atcLimitSpeed = 25.0f;
		printf("ATC �ְ�ӵ�: %.1f\n", atcLimitSpeed);
		atcCode = 25;
		pSound[0];
	} else if (Signal == 3) {
		atcLimitSpeed = 45.0f;
		printf("ATC �ְ�ӵ�: %.1f\n", atcLimitSpeed);
		atcCode = 45;
		pSound[0];
	} else if (Signal == 4) {
		atcLimitSpeed = 60.0f;
		printf("ATC �ְ�ӵ�: %.1f\n", atcLimitSpeed);
		atcCode = 60;
		pSound[0];
	} else if (Signal == 5) {
		atcLimitSpeed = 80.0f;
		printf("ATC �ְ�ӵ�: %.1f\n", atcLimitSpeed);
		atcCode = 80;
		pSound[0];
	} else if (Signal == 6) {
		atcLimitSpeed = 110.0f;
		printf("ATC �ְ�ӵ�: %.1f\n", atcLimitSpeed);
		atcCode = 110;
		pSound[0];
	} else if (Signal == 7) {
		atcLimitSpeed = 130.0f;
		printf("ATC �ְ�ӵ�: %.1f\n", atcLimitSpeed);
		atcCode = 130;
		pSound[0];
	} else if (Signal == 8) {
	}
}