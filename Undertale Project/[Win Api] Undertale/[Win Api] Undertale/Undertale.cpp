#include <Windows.h>
#include <atlimage.h>
#include <array>
#include <tchar.h>

#define ID_MAP_CHANGE 80
#define ID_CHANGE_FRAME 81

HINSTANCE g_hinst;
LPCTSTR lpszClass = L"Window Class Name";
LPCTSTR lpszWindowName = L"windows program 2";

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM IParam);

int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE hPrevlnstance, LPSTR IpszCmdParam, int nCmdShow)
{
	HWND hWnd;
	MSG Message;
	WNDCLASSEX WndClass;
	g_hinst = hinstance;

	WndClass.cbSize = sizeof(WndClass);
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc = (WNDPROC)WndProc;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = hinstance;
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hCursor = LoadCursor(NULL, IDC_HAND);
	WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	WndClass.lpszMenuName = NULL;
	WndClass.lpszClassName = lpszClass;
	WndClass.hIconSm = LoadIcon(NULL, IDI_QUESTION);
	RegisterClassEx(&WndClass);

	hWnd = CreateWindow(lpszClass, lpszWindowName, WS_OVERLAPPEDWINDOW | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_HSCROLL | WS_VSCROLL | WS_THICKFRAME, 200, 0, 1673, 956, NULL, (HMENU)NULL, hinstance, NULL);
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	while (GetMessage(&Message, 0, 0, 0)) {
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}
	return Message.wParam;
}

//
void map_change(HWND hWnd, HDC hDC, int color) {
	HBRUSH hBrush, oldBrush;

	SetTimer(hWnd, ID_MAP_CHANGE, 10, NULL);

	hBrush = CreateSolidBrush(RGB(color, color, color));
	oldBrush = (HBRUSH)SelectObject(hDC, hBrush);
	Rectangle(hDC, 0, 0, 1673, 956);
	SelectObject(hDC, oldBrush);
	DeleteObject(hBrush);

}

//frisk 움직임 처리
void move_frisk(HWND hWnd, int &frisk_x, int &frisk_y, const int& frisk_move_distance, const bool& check_walk , std::array<bool, 256>&keyState, int &move_direction) {
	//대각 이동
	if (keyState[VK_UP] && keyState[VK_LEFT]) {
		frisk_x -= frisk_move_distance;
		frisk_y -= frisk_move_distance;
	}
	else if (keyState[VK_UP] && keyState[VK_RIGHT]) {
		frisk_x += frisk_move_distance;
		frisk_y -= frisk_move_distance;
	}
	else if (keyState[VK_DOWN] && keyState[VK_LEFT]) {
		frisk_x -= frisk_move_distance;
		frisk_y += frisk_move_distance;
	}
	else if (keyState[VK_DOWN] && keyState[VK_RIGHT]) {
		frisk_x += frisk_move_distance;
		frisk_y += frisk_move_distance;
	}
	else {
		// 기본 이동
		if (keyState[VK_UP]) {
			frisk_y -= frisk_move_distance;
			move_direction = 1;
		}
		if (keyState[VK_DOWN]) {
			frisk_y += frisk_move_distance;
			move_direction = 2;
		}
		if (keyState[VK_LEFT]) {
			frisk_x -= frisk_move_distance;
			move_direction = 3;
		}
		if (keyState[VK_RIGHT]) {
			frisk_x += frisk_move_distance;
			move_direction = 4;
		}
	}
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM IParam)
{
	PAINTSTRUCT ps;
	HDC hDC, mDC;
	HBITMAP hBitmap;

	TCHAR text[100];
	RECT rt;
	GetClientRect(hWnd, &rt);				//윈도우 크기 얻어오기
	int windowWidth = rt.right - rt.left;	//윈도우 너비
	int windowHeight = rt.bottom - rt.top;

	int window_Width = 1440;	//윈도우 너비
	int window_Height = 900;	//720

	//image
	static CImage img_map;		//전체 맵
	static CImage img_frisk_front[4], img_frisk_back[4], img_frisk_left[2], img_frisk_right[2];		//frisk

	static int camera_width = 300;
	static int camera_height = 230;

	//map
	static int map_x = 207;
	static int map_y = 2615;
	static int map_number = -1;

	//프리스크
	static int frisk_width;				//image_size	
	static int frisk_height;	

	static int frisk_x = 500;			//frisk 초기 위치
	static int frisk_y = 300;
	int saveX, saveY;					//이동전 좌표 저장(충돌시 이동전 좌표로 이동하기 위함)
	

	int frisk_move_distance = 5;		//이동 거리
	static int move_direction = 2;		//움직이는 방향 (0: stop, 1: 상, 2: 하, 3: 좌, 4: 우)
	static int img_frame = 0;			//현재 이미지 프레임(이미지를 교차해서 나타내기 위함)
	static bool check_walk = true;		//걷는 이미지를 출력해도 되는지 확인
	
	//keyboard
	static std::array<bool, 256> keyState = {};		//대각 이동 처리를 위함

	//(아직 처리 x)
	static int color = 0;				//맵 이동시 이팩트로 사용(화면 검정색으로 되는거)

	// Battle
	static bool battle_mode = false;

	//메시지 처리
	switch (uMsg) {
	case WM_CREATE:
		//map
		img_map.Load(L"map.png");

		//frisk
		img_frisk_front[0].Load(L"frisk_front_stop.png");		//front
		img_frisk_front[1].Load(L"frisk_front_walk_1.png");
		img_frisk_front[2].Load(L"frisk_front_stop.png");
		img_frisk_front[3].Load(L"frisk_front_walk_2.png");

		img_frisk_back[0].Load(L"frisk_back_stop.png");			//back
		img_frisk_back[1].Load(L"frisk_back_walk_1.png");
		img_frisk_back[2].Load(L"frisk_back_stop.png");
		img_frisk_back[3].Load(L"frisk_back_walk_2.png");

		img_frisk_left[0].Load(L"frisk_left_stop.png");			//left
		img_frisk_left[1].Load(L"frisk_left_walk.png");

		img_frisk_right[0].Load(L"frisk_right_stop.png");		//right
		img_frisk_right[1].Load(L"frisk_right_walk.png");

		frisk_width = img_frisk_front[0].GetWidth();			//size
		frisk_height = img_frisk_front[0].GetHeight();
		break;
	case WM_KEYDOWN:

		keyState[wParam] = true;
		saveX = frisk_x;			//이동전 좌표 저장(충돌시 이동전 좌표로 이동하기 위함)
		saveY = frisk_y;

		//frisk move
		move_frisk(hWnd, frisk_x, frisk_y, frisk_move_distance, check_walk, keyState, move_direction);		

		//움직일 수 있다면, 프레임 바꿔줌
		if (check_walk) {					
			SetTimer(hWnd, 1, 200, NULL);
			check_walk = false;
		}

		InvalidateRect(hWnd, NULL, false);
		break;
	case WM_KEYUP:
		KillTimer(hWnd, 1);
		img_frame = 0;				//stop 이미지 프레임
		check_walk = true;			//걷는 이미지 출력 가능
		keyState[wParam] = false;	//키를 눌렀던 부분 false로 바꿈

		InvalidateRect(hWnd, NULL, false);
		break;
	case WM_TIMER:
		switch (wParam)
		{
			//프레임 교차
		case 1:		
			if (move_direction == 1 || move_direction == 2) { // 상하 이동
				img_frame = (img_frame + 1) % 4; // 0 ~ 4 전환
			}
			else if (move_direction == 3 || move_direction == 4) { // 좌우 이동
				img_frame = (img_frame + 1) % 2; // 0 ~ 1 전환
			}
			break;
		case ID_MAP_CHANGE:
			color += 50;
			break;
		}

		InvalidateRect(hWnd, NULL, false);
		break;
	case WM_PAINT:
		hDC = BeginPaint(hWnd, &ps);
		mDC = CreateCompatibleDC(hDC);
		hBitmap = CreateCompatibleBitmap(hDC, rt.right, rt.bottom);
		SelectObject(mDC, (HBITMAP)hBitmap);
		//모든 그리기를 mDC에 한다

		map_number = 5;

		// map
		switch (map_number)
		{
		case 5:
			// Ruins_5
			// map_x = 207;
			// map_y = 2615;


			img_map.Draw(mDC, 100, 0, window_Width, window_Height, map_x, map_y, camera_width, camera_height);
			//frisk
			if (frisk_x > window_Width / 2 - frisk_width / 2 && !(map_x >= 630)) {
				map_x += 3;
				frisk_x = window_Width / 2 - frisk_width / 2; // 중앙에 프리스크 고정
			}
			else if (frisk_x < window_Width / 2 - frisk_width / 2 && !(map_x <= 207)) {
				map_x -= 3;
				frisk_x = window_Width / 2 - frisk_width / 2; // Keep Frisk in the center
			}

			if (frisk_x == 920) {
				if (!(color >= 255)) {
					map_change(hWnd, mDC, color);
					KillTimer(hWnd, ID_MAP_CHANGE);
					break;
				}

			}

		}

		
		// battle
		if (battle_mode) 
		{

		}


		//draw frisk
		switch (move_direction) { // 0: 멈춤, 1: 위, 2: 아래, 3: 왼쪽, 4: 오른쪽
		case 1:
			img_frisk_back[img_frame].Draw(mDC, frisk_x, frisk_y, frisk_width * 3, frisk_height * 3, 0, 0, frisk_width, frisk_height);
			break;
		case 2:
			img_frisk_front[img_frame].Draw(mDC, frisk_x, frisk_y, frisk_width * 3, frisk_height * 3, 0, 0, frisk_width, frisk_height);
			break;
		case 3:
			img_frisk_left[img_frame].Draw(mDC, frisk_x, frisk_y, frisk_width * 3, frisk_height * 3, 0, 0, frisk_width, frisk_height);
			break;
		case 4:
			img_frisk_right[img_frame].Draw(mDC, frisk_x, frisk_y, frisk_width * 3, frisk_height * 3, 0, 0, frisk_width, frisk_height);
			break;
		}


		
		//마지막에 메모리 dc의 내용을 화면 dc로 복사한다.
		BitBlt(hDC, 0, 0, rt.right, rt.bottom, mDC, 0, 0, SRCCOPY);
		DeleteDC(mDC);
		DeleteObject(hBitmap);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, IParam);
}