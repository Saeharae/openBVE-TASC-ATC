
#include "Plugin.h"

int g_iReverser = 1;
int g_iPowerNotch = 0;
int g_iBrakeNotch = 0;

//TASC 변수
int stopPosition = 0; //정차위치 (M)
bool tasc = false; //tasc 동작 여부
bool tascEnabled = false; //tasc 동작 조건 여부
bool tascKeyCode = false; //tasc 동작 키 입력 여부
const double DECELERATION_RATE = 4.5 / 3.6; // km/h/s을 m/s^2로 변환

//ATC 변수
float atcLimitSpeed = 130; //atc 최고속도 지정
int atcCode = 0; //atc 속도코드

ATS_API void WINAPI Load()
{
	AllocConsole();
	freopen("CONOUT$", "wt", stdout);
	printf("플러그인을 불러왔습니다.\n"); //console only
}

ATS_API void WINAPI Dispose()
{
	printf("플러그인이 Unload 되었습니다.\n"); //console only
}

ATS_API int WINAPI GetPluginVersion()
{
	return ATS_VERSION;
}

ATS_API void WINAPI SetVehicleSpec(ATS_VEHICLESPEC VehicleSpec)
{
	printf("열차 정보를 불러왔습니다.\n"); //console only
}

ATS_API void WINAPI Initialize(int iHandle)
{
	printf("Initialize : %d\n", iHandle); //console only
}

//ATC 감속 변수
bool brake7 = false;
bool brakeN = false;

ATS_API ATS_HANDLES WINAPI Elapse(ATS_VEHICLESTATE VehicleState, int* pPanel, int* pSound)
{
	ATS_HANDLES Output;
	Output.Reverser = g_iReverser;
	Output.Power = g_iPowerNotch;
	Output.Brake = g_iBrakeNotch;

	//tasc 동작조건 여부
	//tasc 동작조건을 충족하면, tasc 키가 활성화 되어있으면 tasc를 작동, 비활성화 되어있으면 tasc 미작동.
	if (tascEnabled) {
		if (tascKeyCode) {
			tasc = true;
			pPanel[0] = 1;
		} else if (!tascKeyCode) {
			tasc = false;
			pPanel[0] = 0;
		}
	}

	//tasc 로직
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
			g_iBrakeNotch = 5; //5단 제동
			tasc = false;
			tascEnabled = false;
			pPanel[0] = 0;
			printf("열차가 정차하여 TASC가 해제 됩니다.\n"); //console only
		}
	}

	//atc 로직
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
	//사동역
	if (BeaconData.Optional == 11101) {
		stopPosition = 2550;
		printf("다음 정차 위치가 %d로 변경 되었습니다.\n", stopPosition); //console only
	}

	//상록수역
	if (BeaconData.Optional == 10102) {
		tascEnabled = true;
	} else if (BeaconData.Optional == 11102) {
		stopPosition = 3425;
		printf("다음 정차 위치가 %d로 변경 되었습니다.\n", stopPosition); //console only
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
			printf("TASC가 비활성화 되었습니다.\n수동으로 정차해야 합니다.\n"); //console only
		} else if (!tascKeyCode) {
			tascKeyCode = true;
			printf("TASC가 활성화 되었습니다.\n역에 정차하기 500m 이전에 별도의 조작 없이 TASC가 동작합니다.\n"); //console only
		}
	}
}

ATS_API void WINAPI KeyUp(){}

ATS_API void WINAPI HornBlow(){}

ATS_API void WINAPI DoorOpen(){}

ATS_API void WINAPI DoorClose(){}

ATS_API void WINAPI SetSignal(int Signal, int* pSound) {
	//ATC 로직
	if (Signal == 0) {
		atcLimitSpeed = 0.0f;
		printf("ATC 최고속도: %.1f\n", atcLimitSpeed);
		atcCode = 0;
		pSound[0];
	} else if (Signal == 1) {
		atcLimitSpeed = 15.0f;
		printf("ATC 최고속도: %.1f\n", atcLimitSpeed);
		atcCode = 15;
		pSound[0];
	} else if (Signal == 2) {
		atcLimitSpeed = 25.0f;
		printf("ATC 최고속도: %.1f\n", atcLimitSpeed);
		atcCode = 25;
		pSound[0];
	} else if (Signal == 3) {
		atcLimitSpeed = 45.0f;
		printf("ATC 최고속도: %.1f\n", atcLimitSpeed);
		atcCode = 45;
		pSound[0];
	} else if (Signal == 4) {
		atcLimitSpeed = 60.0f;
		printf("ATC 최고속도: %.1f\n", atcLimitSpeed);
		atcCode = 60;
		pSound[0];
	} else if (Signal == 5) {
		atcLimitSpeed = 80.0f;
		printf("ATC 최고속도: %.1f\n", atcLimitSpeed);
		atcCode = 80;
		pSound[0];
	} else if (Signal == 6) {
		atcLimitSpeed = 110.0f;
		printf("ATC 최고속도: %.1f\n", atcLimitSpeed);
		atcCode = 110;
		pSound[0];
	} else if (Signal == 7) {
		atcLimitSpeed = 130.0f;
		printf("ATC 최고속도: %.1f\n", atcLimitSpeed);
		atcCode = 130;
		pSound[0];
	} else if (Signal == 8) {
	}
}