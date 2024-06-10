#include <Windows.h>
#include <atlimage.h>
#include <array>
#include <tchar.h>

#define ID_MAP_CHANGE 80
#define ID_CHANGE_FRAME 81

//맵 좌표

const int map1_x = 640;
const int map1_y = 3850;


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

	hWnd = CreateWindow(lpszClass, lpszWindowName, WS_OVERLAPPEDWINDOW | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_HSCROLL | WS_VSCROLL | WS_THICKFRAME, 0, 0, 1673, 956, NULL, (HMENU)NULL, hinstance, NULL);
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	while (GetMessage(&Message, 0, 0, 0)) {
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}
	return Message.wParam;
}

//충돌 체크
bool map_collision_check(int frisk_x, int frisk_y, int camera_x, int camera_y, RECT* collisionRects, int numRects) {

	int player_width = 60;
	int player_height = 90;
	int world_x = frisk_x + camera_x;
	int world_y = frisk_y + camera_y;

	for (int i = 0; i < numRects; ++i) {
		if (world_x  <= collisionRects[i].right && world_x + player_width >= collisionRects[i].left &&
			world_y  <= collisionRects[i].bottom && world_y + player_height >= collisionRects[i].top) {
			return true;
		}
	}
	return false;
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
void move_frisk(HWND hWnd, int& frisk_x, int& frisk_y, const int& frisk_move_distance, 
	const bool& check_walk, std::array<bool, 256>& keyState, int& move_direction, 
	int camera_x, int camera_y, RECT* collisionRects, int numRects) {

	int save_x = frisk_x;
	int save_y = frisk_y;

	
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
	// 충돌 검사
	/*if (map_collision_check(frisk_x, frisk_y, camera_x, camera_y, collisionRects, numRects)) {
		frisk_x = save_x;
		frisk_y = save_y;
	}*/
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM IParam)
{
	PAINTSTRUCT ps;
	HDC hDC, mDC;
	HPEN hPen, hOldPen;
	HBITMAP hBitmap;
	HBRUSH hBrush;
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
	
	//camera
	static int camera_x = 0;			// 카메라로 수정함
	static int camera_y = 0;
	static int camera_move_distance = 0;

	//map
	static int map_number = 33;
	static bool max_change = true;		//map 바뀌는지 확인

	//프리스크
	int frisk_img_width = 20;				//image_size	
	int frisk_img_height = 30;
	int frisk_game_width = 60;				//인게임 size	
	int frisk_game_height = 90;

	static int frisk_x = 0;			//frisk 위치
	static int frisk_y = 0;

	static int color = 0;

	int frisk_move_distance = 20;		//이동 거리
	static int move_direction = 2;		//움직이는 방향 (0: stop, 1: 상, 2: 하, 3: 좌, 4: 우)
	static int img_frame = 0;			//현재 이미지 프레임(이미지를 교차해서 나타내기 위함)
	static bool check_walk = true;		//걷는 이미지를 출력해도 되는지 확인

	//충돌 영역(좌, 상, 우, 하)
	RECT collisionRects[] = {
		//map1
		{2113, 3870, 2200, 4266},
		{2020, 4268, 2100, 4338},
		{1929, 4346, 2003, 4419},
		{1831, 4419, 1915, 4500},
		{1157, 4500, 1820, 4577},
		{1064, 4422, 1151, 4495},
		{705 , 4342, 1057, 4416},
		//{800 , 4111, 860, 4172}
	};

	//충돌 박스 개수
	static int numRects;	//나중에 전역 x로 바꾸삼(테스트용으로 바꿔둠

	//keyboard
	static std::array<bool, 256> keyState = {};		//대각 이동 처리를 위함

	//마우스
	static int mx, my;

	//(아직 처리 x)
	// static int color = 0;				//맵 이동시 이팩트로 사용(화면 검정색으로 되는거)

	// Battle
	static bool battle_mode = false;

	//메시지 처리
	switch (uMsg) {
	case WM_CREATE:
	{
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
		
		break;
	}
	case WM_KEYDOWN:
	{
		keyState[wParam] = true;

		if (keyState['Q']) {
			exit(0);
		}

		//충돌 박스 개수
		numRects = sizeof(collisionRects) / sizeof(RECT);

		//frisk move
		move_frisk(hWnd, frisk_x, frisk_y, frisk_move_distance, check_walk, keyState,
			move_direction, camera_x, camera_y, collisionRects, numRects);

		//움직일 수 있다면, 프레임 바꿔줌
		if (check_walk) {
			SetTimer(hWnd, 1, 200, NULL);
			check_walk = false;
		}

		InvalidateRect(hWnd, NULL, false);
	}
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

		// map
		switch (map_number)
		{
		case 1:
		{

			if (max_change) {
				frisk_x = 820;
				frisk_y = 400;
				camera_x = map1_x;
				camera_y = map1_y;
				max_change = false;
			}

			//카메라 이동
			if (frisk_x > window_Width / 2 - frisk_game_width / 2 && !(camera_x >= 640)) {
				camera_x += 3;
				camera_move_distance -= frisk_move_distance;
				frisk_x = window_Width / 2 - frisk_game_width / 2; // 중앙에 프리스크 고정
			}
			else if (frisk_x < window_Width / 2 - frisk_game_width / 2 && !(camera_x <= 207)) {
				camera_x -= 3;
				camera_move_distance += frisk_move_distance;
				frisk_x = window_Width / 2 - frisk_game_width / 2; // Keep Frisk in the center
			}
			

			//map change
			if (610 <= frisk_x && frisk_x <= 710 && frisk_y <= 255 &&
				camera_x == 205 && camera_y == 3850) {
				map_number = 2;
				max_change = true;
				InvalidateRect(hWnd, NULL, false);
			}
			break;
		}
		case 2:
		{
			if (max_change) {
				move_direction = 1;
				frisk_x = 720;
				frisk_y = 740;
				camera_x = 204;
				camera_y = 3625;
				max_change = false;
			}

			//frisk y축 이동
			if (frisk_y > window_Height / 2 - frisk_game_height / 2 && !(camera_y >= 3625)) {		//중앙보다 위에 있을때
				camera_y += 3;
				camera_move_distance -= frisk_move_distance;
				frisk_y = window_Height / 2 - frisk_game_height / 2; // 중앙에 프리스크 고정
			}
			else if (frisk_y < window_Height / 2 - frisk_game_height / 2 && !(camera_y <= 3553)) {	//중앙보다 아래에 있을때
				camera_y -= 3;
				camera_move_distance += frisk_move_distance;
				frisk_y = window_Height / 2 - frisk_game_height / 2; // 중앙에 프리스크 고정
			}

			//map change
			if (640 <= frisk_x && frisk_x <= 780 && frisk_y <= 265 &&
				camera_x == 204 && camera_y == 3553) {
				map_number = 3;
				max_change = true;
				InvalidateRect(hWnd, NULL, false);
			}
			break;
		}
		case 3:
		{
			if (max_change) {
				move_direction = 1;
				frisk_x = 710;
				frisk_y = 760;
				camera_x = 204;
				camera_y = 3319;
				max_change = false;
			}

			//frisk y축 이동
			if (frisk_y > window_Height / 2 - frisk_game_height / 2 && !(camera_y >= 3319)) {		//중앙보다 위에 있을때
				camera_y += 3;
				camera_move_distance -= frisk_move_distance;
				frisk_y = window_Height / 2 - frisk_game_height / 2; // 중앙에 프리스크 고정
			}
			else if (frisk_y < window_Height / 2 - frisk_game_height / 2 && !(camera_y <= 3091)) {	//중앙보다 아래에 있을때
				camera_y -= 3;
				camera_move_distance += frisk_move_distance;
				frisk_y = window_Height / 2 - frisk_game_height / 2; // 중앙에 프리스크 고정
			}

			//map change
			if (680 <= frisk_x && frisk_x <= 735 && frisk_y <= 325 &&
				camera_x == 204 && camera_y == 3091) {
				map_number = 4;
				max_change = true;
				InvalidateRect(hWnd, NULL, false);
			}
			break;
		}
		case 4:
		{
			if (max_change) {
				move_direction = 1;
				frisk_x = 720;
				frisk_y = 740;
				camera_x = 204;
				camera_y = 2853;
				max_change = false;
			}
			//카메라 이동 필요없는 맵

			//map change
			if (540 <= frisk_x && frisk_x <= 680 && frisk_y <= 260 &&
				camera_x == 204 && camera_y == 2853) {
				map_number = 5;
				max_change = true;
				InvalidateRect(hWnd, NULL, false);
			}
			break;
		}
		case 5:
		{
			if (max_change) {
				move_direction = 1;
				frisk_x = 610;
				frisk_y = 800;
				camera_x = 204;
				camera_y = 2621;
				max_change = false;
			}


			//카메라 이동x
			if (frisk_x > window_Width / 2 - frisk_game_width / 2 && !(camera_x >= 620)) {
				camera_x += 3;
				camera_move_distance -= frisk_move_distance;
				frisk_x = window_Width / 2 - frisk_game_width / 2; // 중앙에 프리스크 고정
			}
			else if (frisk_x < window_Width / 2 - frisk_game_width / 2 && !(camera_x <= 204)) {
				camera_x -= 3;
				camera_move_distance += frisk_move_distance;
				frisk_x = window_Width / 2 - frisk_game_width / 2; // Keep Frisk in the center
			}

			//map change
			if (470 <= frisk_y && frisk_y <= 540 && frisk_x >= 1470 &&
				camera_x == 621 && camera_y == 2621) {
				map_number = 6;
				max_change = true;
				InvalidateRect(hWnd, NULL, false);
			}
			break;
		}
		case 6:
		{
			if (max_change) {
				move_direction = 4;
				frisk_x = 100;
				frisk_y = 500;
				camera_x = 928;
				camera_y = 2630;
				max_change = false;
			}

			//map change
			if (710 <= frisk_x && frisk_x <= 840 && frisk_y <= 160 &&
				camera_x == 928 && camera_y == 2630) {
				map_number = 7;
				max_change = true;
				InvalidateRect(hWnd, NULL, false);
			}
			break;
		}
		case 7:
		{
			if (max_change) {
				move_direction = 1;
				frisk_x = 690;
				frisk_y = 740;
				camera_x = 945;
				camera_y = 2380;
				max_change = false;
			}

			//카메라 이동x
			if (frisk_x > window_Width / 2 - frisk_game_width / 2 && !(camera_x >= 1785)) {
				camera_x += 3;
				camera_move_distance -= frisk_move_distance;
				frisk_x = window_Width / 2 - frisk_game_width / 2; // 중앙에 프리스크 고정
			}
			else if (frisk_x < window_Width / 2 - frisk_game_width / 2 && !(camera_x <= 945)) {
				camera_x -= 3;
				camera_move_distance += frisk_move_distance;
				frisk_x = window_Width / 2 - frisk_game_width / 2; // Keep Frisk in thecenter
			}



			//map change
			if (360 <= frisk_y && frisk_y <= 420 && frisk_x >= 1470 &&
				camera_x == 1785 && camera_y == 2380) {
				map_number = 8;
				max_change = true;
				InvalidateRect(hWnd, NULL, false);
			}
			break;
		}
		case 8:
		{
			if (max_change) {
				move_direction = 4;
				frisk_x = 110;
				frisk_y = 440;
				camera_x = 2107;
				camera_y = 2365;
				max_change = false;
			}

			//카메라 이동x
			if (frisk_x > window_Width / 2 - frisk_game_width / 2 && !(camera_x >= 3160)) {
				camera_x += 3;
				camera_move_distance -= frisk_move_distance;
				frisk_x = window_Width / 2 - frisk_game_width / 2; // 중앙에 프리스크 고정
			}
			else if (frisk_x < window_Width / 2 - frisk_game_width / 2 && !(camera_x <= 2107)) {
				camera_x -= 3;
				camera_move_distance += frisk_move_distance;
				frisk_x = window_Width / 2 - frisk_game_width / 2; // Keep Frisk in thecenter
			}



			//map change
			if (410 <= frisk_y && frisk_y <= 480 && frisk_x >= 1470 &&
				camera_x == 3160 && camera_y == 2365) {
				map_number = 9;

				
				max_change = true;
				InvalidateRect(hWnd, NULL, false);
			}
			break;
		}
		case 9:
		{
			if (max_change) {
				move_direction = 4;
				frisk_x = 110;
				frisk_y = 410;
				camera_x = 3470;
				camera_y = 2375;
				max_change = false;
			}

			//frisk y축 이동
			if (frisk_y > window_Height / 2 - frisk_game_height / 2 && !(camera_y >= 2573)) {		//중앙보다 위에 있을때
				camera_y += 3;
				camera_move_distance -= frisk_move_distance;
				frisk_y = window_Height / 2 - frisk_game_height / 2; // 중앙에 프리스크 고정
			}
			else if (frisk_y < window_Height / 2 - frisk_game_height / 2 && !(camera_y <= 2375)) {	//중앙보다 아래에 있을때
				camera_y -= 3;
				camera_move_distance += frisk_move_distance;
				frisk_y = window_Height / 2 - frisk_game_height / 2; // 중앙에 프리스크 고정
			}

			//map change(윗문)
			if (760 <= frisk_x && frisk_y <= 900 && frisk_y <= 240 &&
				camera_x == 3470 && camera_y == 2375) {
				map_number = 10;
				max_change = true;
				InvalidateRect(hWnd, NULL, false);
			}

			//옆문
			if (545 <= frisk_y && frisk_y <= 605 && frisk_x >= 1470 &&
				camera_x == 3470 && camera_y == 2573) {
				map_number = 11;
				max_change = true;
				InvalidateRect(hWnd, NULL, false);
			}
			break;
		}
		case 10:
		{
			if (max_change) {
				move_direction = 1;
				frisk_x = 705;
				frisk_y = 720;
				camera_x = 3494;
				camera_y = 2141;
				max_change = false;
			}

			//map change(10->9)
			if (640 <= frisk_x && frisk_x <= 770 && frisk_y >= 780 &&
				camera_x == 3494 && camera_y == 2141) {
				map_number = 9;

				//
				move_direction = 2;
				frisk_x = 815;
				frisk_y = 285;
				camera_x = 3470;
				camera_y = 2375;
				max_change = false;
				InvalidateRect(hWnd, NULL, false);
			}
			break;
		}
		case 11:
		{
			if (max_change) {
				move_direction = 4;
				frisk_x = 110;
				frisk_y = 490;
				camera_x = 3778;
				camera_y = 2591;
				max_change = false;
			}

			// 아래로 떨어짐
			if (470 <= frisk_y && frisk_y <= 610 && frisk_x >= 710 && frisk_x <= 730&&
				camera_x == 3778 && camera_y == 2591) {
				map_number = 12;
				max_change = true;
				InvalidateRect(hWnd, NULL, false);
			}

			//옆문
			if (470 <= frisk_y && frisk_y <= 535 && frisk_x >= 1470 &&
				camera_x == 3778 && camera_y == 2591) {
				map_number = 13;
				max_change = true;
				InvalidateRect(hWnd, NULL, false);
			}
			break;
		}
		case 12:
		{
			//아래방
			if (max_change) {
				move_direction = 1;
				frisk_x = 670;
				frisk_y = 450;
				camera_x = 3790;
				camera_y = 2851;
				max_change = false;
			}

			// 좌
			if (295 <= frisk_x && frisk_x <= 390 && frisk_y <= 270 &&
				camera_x == 3790 && camera_y == 2851) {
				map_number = 11;

				move_direction = 4;
				frisk_x = 350;
				frisk_y = 490;
				camera_x = 3778;
				camera_y = 2591;
				max_change = false;
				InvalidateRect(hWnd, NULL, false);
			}

			// 우
			if (1070 <= frisk_x && frisk_x <= 1155 && frisk_y <= 270 &&
				camera_x == 3790 && camera_y == 2851) {
				map_number = 11;

				move_direction = 4;
				frisk_x = 1120;
				frisk_y = 490;
				camera_x = 3778;
				camera_y = 2591;
				max_change = false;
				InvalidateRect(hWnd, NULL, false);
			}
			break;
		}
		case 13:
		{
			if (max_change) {
				move_direction = 4;
				frisk_x = 110;
				frisk_y = 390;
				camera_x = 4108;
				camera_y = 2619;
				max_change = false;
			}

			//카메라 이동x
			if (frisk_x > window_Width / 2 - frisk_game_width / 2 && !(camera_x >= 4242)) {
				camera_x += 3;
				camera_move_distance -= frisk_move_distance;
				frisk_x = window_Width / 2 - frisk_game_width / 2; // 중앙에 프리스크 고정
			}
			else if (frisk_x < window_Width / 2 - frisk_game_width / 2 && !(camera_x <= 4108)) {
				camera_x -= 3;
				camera_move_distance += frisk_move_distance;
				frisk_x = window_Width / 2 - frisk_game_width / 2; // Keep Frisk in thecenter
			}

			//옆문
			if (360 <= frisk_y && frisk_y <= 430 && frisk_x >= 1470 &&
				camera_x == 4243 && camera_y == 2619) {
				map_number = 14;
				max_change = true;
				InvalidateRect(hWnd, NULL, false);
			}
			break;
		}
		case 14:
		{
			if (max_change) {
				move_direction = 4;
				frisk_x = 110;
				frisk_y = 390;
				camera_x = 4545;
				camera_y = 2619;
				max_change = false;
			}

			//카메라 이동x
			if (frisk_x > window_Width / 2 - frisk_game_width / 2 && !(camera_x >= 4821)) {
				camera_x += 3;
				camera_move_distance -= frisk_move_distance;
				frisk_x = window_Width / 2 - frisk_game_width / 2; // 중앙에 프리스크 고정
			}
			else if (frisk_x < window_Width / 2 - frisk_game_width / 2 && !(camera_x <= 4545)) {
				camera_x -= 3;
				camera_move_distance += frisk_move_distance;
				frisk_x = window_Width / 2 - frisk_game_width / 2; // Keep Frisk in thecenter
			}

			//frisk y축 이동
			if (frisk_y > window_Height / 2 - frisk_game_height / 2 && !(camera_y >= 2709)) {		//중앙보다 위에 있을때
				camera_y += 3;
				camera_move_distance -= frisk_move_distance;
				frisk_y = window_Height / 2 - frisk_game_height / 2; // 중앙에 프리스크 고정
			}
			else if (frisk_y < window_Height / 2 - frisk_game_height / 2 && !(camera_y <= 2619)) {	//중앙보다 아래에 있을때
				camera_y -= 3;
				camera_move_distance += frisk_move_distance;
				frisk_y = window_Height / 2 - frisk_game_height / 2; // 중앙에 프리스크 고정
			}

			//down 1
			if (frisk_y  == 405 && frisk_x == 690 &&
				4656 <= camera_x && camera_x <= 4722 && 
				2649 <= camera_y && camera_y <= 2709) {
				map_number = 15;

				//1
				move_direction = 1;
				frisk_x = 690;
				frisk_y = 425;
				camera_x = 4706;
				camera_y = 3095;
				max_change = false;
				InvalidateRect(hWnd, NULL, false);
			}

			//down 2
			if (frisk_y == 405 && frisk_x == 690 &&
				4779 <= camera_x && camera_x <= 4785 &&
				2688 <= camera_y && camera_y <= 2709) {
				map_number = 15;

				//2
				move_direction = 1;
				frisk_x = 690;
				frisk_y = 445;
				camera_x = 4796;
				camera_y = 3095;
				max_change = false;
				InvalidateRect(hWnd, NULL, false);
			}

			//down 3
			if (camera_y == 2709 && camera_x == 4821 &&
				770 <= frisk_x && frisk_x <= 800 &&
				485 <= frisk_y && frisk_y <= 700) {
				map_number = 15;

				//1
				move_direction = 1;
				frisk_x = 913;
				frisk_y = 625;
				camera_x = 4808;
				camera_y = 3095;
				max_change = false;
				InvalidateRect(hWnd, NULL, false);
			}

			// down 4
			if (camera_y == 2709 && camera_x == 4821 &&
				1070 <= frisk_x && frisk_x <= 1190 &&
				405 <= frisk_y && frisk_y <= 625) {
				map_number = 15;

				// 4
				move_direction = 1;
				frisk_x = 1270;
				frisk_y = 425;
				camera_x = 4808;
				camera_y = 3095;
				max_change = false;
				InvalidateRect(hWnd, NULL, false);
			}
			//옆문
			if (470 <= frisk_y && frisk_y <= 545 && frisk_x >= 1470 &&
				camera_x == 4821 && camera_y == 2709) {
				map_number = 16;		
				max_change = true;
				InvalidateRect(hWnd, NULL, false);
			}
			break;
		}
		case 15:
		{
			//아래
			//카메라 이동x
			if (frisk_x > window_Width / 2 - frisk_game_width / 2 && !(camera_x >= 4808)) {
				camera_x += 3;
				camera_move_distance -= frisk_move_distance;
				frisk_x = window_Width / 2 - frisk_game_width / 2; // 중앙에 프리스크 고정
			}
			else if (frisk_x < window_Width / 2 - frisk_game_width / 2 && !(camera_x <= 4586)) {
				camera_x -= 3;
				camera_move_distance += frisk_move_distance;
				frisk_x = window_Width / 2 - frisk_game_width / 2; // Keep Frisk in thecenter
			}

			//frisk y축 이동
			if (frisk_y > window_Height / 2 - frisk_game_height / 2 && !(camera_y >= 3095)) {		//중앙보다 위에 있을때
				camera_y += 3;
				camera_move_distance -= frisk_move_distance;
				frisk_y = window_Height / 2 - frisk_game_height / 2; // 중앙에 프리스크 고정
			}
			else if (frisk_y < window_Height / 2 - frisk_game_height / 2 && !(camera_y <= 3026)) {	//중앙보다 아래에 있을때
				camera_y -= 3;
				camera_move_distance += frisk_move_distance;
				frisk_y = window_Height / 2 - frisk_game_height / 2; // 중앙에 프리스크 고정
			}


			//up
			if (650 <= frisk_x && frisk_x <= 690 && frisk_y <= 225 &&
				camera_x == 4586 && camera_y == 3026) {
				map_number = 14;

				move_direction = 4;
				frisk_x = 690;
				frisk_y = 390;
				camera_x = 4572;
				camera_y = 2619;
				max_change = false;
				InvalidateRect(hWnd, NULL, false);
			}
			break;
		}
		case 16:
		{
			if (max_change) {
				move_direction = 4;
				frisk_x = 110;
				frisk_y = 440;
				camera_x = 5125;
				camera_y = 2725;
				max_change = false;
			}

			//카메라 이동x
			if (frisk_x > window_Width / 2 - frisk_game_width / 2 && !(camera_x >= 5377)) {
				camera_x += 3;
				camera_move_distance -= frisk_move_distance;
				frisk_x = window_Width / 2 - frisk_game_width / 2; // 중앙에 프리스크 고정
			}
			else if (frisk_x < window_Width / 2 - frisk_game_width / 2 && !(camera_x <= 5125)) {
				camera_x -= 3;
				camera_move_distance += frisk_move_distance;
				frisk_x = window_Width / 2 - frisk_game_width / 2; // Keep Frisk in thecenter
			}

			//map change
			if (420 <= frisk_y && frisk_y <= 480 && frisk_x >= 1470 &&
				camera_x == 5377 && camera_y == 2725) {
				map_number = 17;
				max_change = true;
				InvalidateRect(hWnd, NULL, false);
			}
			break;
		}
		case 17:
		{
			if (max_change) {
				move_direction = 4;
				frisk_x = 110;
				frisk_y = 405;
				camera_x = 5677;
				camera_y = 2735;
				max_change = false;
			}

			//map change
			if (370 <= frisk_y && frisk_y <= 445 && frisk_x >= 1470 &&
				camera_x == 5677 && camera_y == 2735) {
				map_number = 18;
				max_change = true;
				InvalidateRect(hWnd, NULL, false);
			}
			break;
		}
		case 18:
		{
			if (max_change) {
				move_direction = 4;
				frisk_x = 110;
				frisk_y = 465;
				camera_x = 6000;
				camera_y = 2721;
				max_change = false;
			}

			// right
			if (420 <= frisk_y && frisk_y <= 505 && frisk_x >= 1470 &&
				camera_x == 6000 && camera_y == 2721) {
				map_number = 19;
				max_change = true;
				InvalidateRect(hWnd, NULL, false);
			}

			// up
			if (1030 <= frisk_x && frisk_x <= 1170 && frisk_y <= 230 &&
				camera_x == 6000 && camera_y == 2721) {
				map_number = 20;
				max_change = true;
				InvalidateRect(hWnd, NULL, false);
			}
			break;
		}
		case 19:
		{
			if (max_change) {
				move_direction = 4;
				frisk_x = 110;
				frisk_y = 465;
				camera_x = 6315;
				camera_y = 2721;
				max_change = false;
			}

			//
			if (425 <= frisk_y && frisk_y <= 500 && frisk_x <= 100 &&
				camera_x == 6315 && camera_y == 2721) {
				map_number = 18;

				move_direction = 3;
				frisk_x = 1450;
				frisk_y = 465;
				camera_x = 6000;
				camera_y = 2721;
				max_change = false;
				InvalidateRect(hWnd, NULL, false);
			}
			break;
		}
		case 20:
		{
			if (max_change) {
				move_direction = 1;
				frisk_x = 410;
				frisk_y = 745;
				camera_x = 6144;
				camera_y = 2458;
				max_change = false;
			}

			//카메라 이동x
			if (frisk_x > window_Width / 2 - frisk_game_width / 2 && !(camera_x >= 6405)) {
				camera_x += 3;
				camera_move_distance -= frisk_move_distance;
				frisk_x = window_Width / 2 - frisk_game_width / 2; // 중앙에 프리스크 고정
			}
			else if (frisk_x < window_Width / 2 - frisk_game_width / 2 && !(camera_x <= 6144)) {
				camera_x -= 3;
				camera_move_distance += frisk_move_distance;
				frisk_x = window_Width / 2 - frisk_game_width / 2; // Keep Frisk in thecenter
			}

			//
			if (445	 <= frisk_y && frisk_y <= 505 && frisk_x >= 1470 &&
				camera_x == 6405 && camera_y == 2458) {
				map_number = 21;
				max_change = true;
				InvalidateRect(hWnd, NULL, false);
			}
			break;
		}
		case 21:
		{
			if (max_change) {
				move_direction = 4;
				frisk_x = 110;
				frisk_y = 305;
				camera_x = 6690;
				camera_y = 2502;
				max_change = false;
			}

			//카메라 이동x
			if (frisk_x > window_Width / 2 - frisk_game_width / 2 && !(camera_x >= 7038)) {
				camera_x += 3;
				camera_move_distance -= frisk_move_distance;
				frisk_x = window_Width / 2 - frisk_game_width / 2; // 중앙에 프리스크 고정
			}
			else if (frisk_x < window_Width / 2 - frisk_game_width / 2 && !(camera_x <= 6690)) {
				camera_x -= 3;
				camera_move_distance += frisk_move_distance;
				frisk_x = window_Width / 2 - frisk_game_width / 2; // Keep Frisk in thecenter
			}

			//frisk y축 이동
			if (frisk_y > window_Height / 2 - frisk_game_height / 2 && !(camera_y >= 2550)) {		//중앙보다 위에 있을때
				camera_y += 3;
				camera_move_distance -= frisk_move_distance;
				frisk_y = window_Height / 2 - frisk_game_height / 2; // 중앙에 프리스크 고정
			}
			else if (frisk_y < window_Height / 2 - frisk_game_height / 2 && !(camera_y <= 2502)) {	//중앙보다 아래에 있을때
				camera_y -= 3;
				camera_move_distance += frisk_move_distance;
				frisk_y = window_Height / 2 - frisk_game_height / 2; // 중앙에 프리스크 고정
			}

			//down 1-1
			if (320 <= frisk_y && frisk_y <= 345 &&	frisk_x == 690 &&
				6831 <= camera_x && camera_x <= 6840 &&
				2502 <= camera_y && camera_y <= 2502) {
				map_number = 22;
				max_change = true;
				InvalidateRect(hWnd, NULL, false);
			}

			//down 1-2
			if (605 <= frisk_y && frisk_y <= 645 && frisk_x == 690 &&
				6831 <= camera_x && camera_x <= 6840 &&
				 camera_y == 2550) {
				map_number = 23;
				max_change = true;
				InvalidateRect(hWnd, NULL, false);
			}

			//down 2-1
			if (320 <= frisk_y && frisk_y <= 345 && frisk_x == 690 &&
				6951 <= camera_x && camera_x <= 6960 &&
				camera_y == 2502) {
				map_number = 24;
				max_change = true;
				InvalidateRect(hWnd, NULL, false);
			}

			//down 2-2
			if (605 <= frisk_y && frisk_y <= 645 && frisk_x == 690 &&
				6951 <= camera_x && camera_x <= 6960 &&
				camera_y == 2550) {
				map_number = 25;
				max_change = true;
				InvalidateRect(hWnd, NULL, false);
			}

			//down 3-1
			if (325 <= frisk_y && frisk_y <= 365 &&
				850 <= frisk_x && frisk_x <= 890 &&
				camera_x == 7038 && camera_y == 2502) {
				map_number = 26;
				max_change = true;
				InvalidateRect(hWnd, NULL, false);
			}

			//down 3-2
			if (605 <= frisk_y && frisk_y <= 645 &&
				850 <= frisk_x && frisk_x <= 890 &&
				camera_x == 7038 && camera_y == 2550) {
				map_number = 27;
				max_change = true;
				InvalidateRect(hWnd, NULL, false);
			}

			//right
			if (frisk_y == 405 && frisk_x >= 1470 &&
				 camera_x == 7038 &&
				2526 <= camera_y && camera_y <= 2544) {
				map_number = 28;
				max_change = true;
				InvalidateRect(hWnd, NULL, false);
			}
			break;
		}
		case 22:
		{
			if (max_change) {
				move_direction = 1;
				frisk_x = 610;
				frisk_y = 225;
				camera_x = 6845;
				camera_y = 2864;
				max_change = false;
			}

			//
			if (210 <= frisk_x && frisk_x <= 230 && frisk_y <= 105 &&
				camera_x == 6845 && camera_y == 2864) {
				map_number = 21;

				move_direction = 4;
				frisk_x = 690;
				frisk_y = 305;
				camera_x = 6753;
				camera_y = 2502;
				max_change = false;
				InvalidateRect(hWnd, NULL, false);
			}
			break;
		}
		case 23:
		{
			if (max_change) {
				move_direction = 4;
				frisk_x = 610;
				frisk_y = 245;
				camera_x = 6845;
				camera_y = 2980;
				max_change = false;
			}

			//
			if (210 <= frisk_x && frisk_x <= 230 && frisk_y <= 125 &&
				camera_x == 6845 && camera_y == 2980) {
				map_number = 21;

				move_direction = 4;
				frisk_x = 690;
				frisk_y = 305;
				camera_x = 6753;
				camera_y = 2502;
				max_change = false;
				InvalidateRect(hWnd, NULL, false);
			}
			break;
		}
		case 24:
		{
			if (max_change) {
				move_direction = 1;
				frisk_x = 310;
				frisk_y = 225;
				camera_x = 7025;
				camera_y = 2864;
				max_change = false;
			}

			//
			if (490 <= frisk_x && frisk_x <= 530 && frisk_y <= 105 &&
				camera_x == 7025 && camera_y == 2864) {
				map_number = 21;

				move_direction = 4;
				frisk_x = 690;
				frisk_y = 305;
				camera_x = 6996;
				camera_y = 2502;
				max_change = false;
				InvalidateRect(hWnd, NULL, false);
			}
			break;
		}
		case 25:
		{
			if (max_change) {
				move_direction = 1;
				frisk_x = 310;
				frisk_y = 225;
				camera_x = 7025;
				camera_y = 2980;
				max_change = false;
			}

			//
			if (490 <= frisk_x && frisk_x <= 530 && frisk_y <= 105 &&
				camera_x == 7025 && camera_y == 2980) {
				map_number = 21;

				move_direction = 4;
				frisk_x = 690;
				frisk_y = 305;
				camera_x = 6996;
				camera_y = 2502;
				max_change = false;
				InvalidateRect(hWnd, NULL, false);
			}
			break;
		}
		case 26:
		{
			if (max_change) {
				move_direction = 1;
				frisk_x = 230;
				frisk_y = 225;
				camera_x = 7165;
				camera_y = 2864;
				max_change = false;
			}

			//
			if (490 <= frisk_x && frisk_x <= 530 && frisk_y <= 105 &&
				camera_x == 7165 && camera_y == 2864) {
				map_number = 21;

				move_direction = 2;
				frisk_x = 1150;
				frisk_y = 305;
				camera_x = 7038	;
				camera_y = 2502;
				max_change = false;
				InvalidateRect(hWnd, NULL, false);
			}
			break;
		}
		case 27:
		{
			if (max_change) {
				move_direction = 1;
				frisk_x = 230;
				frisk_y = 225;
				camera_x = 7165;
				camera_y = 2980;
				max_change = false;
			}

			//
			if (490 <= frisk_x && frisk_x <= 530 && frisk_y <= 105 &&
				camera_x == 7165 && camera_y == 2980) {
				map_number = 21;

				move_direction = 2;
				frisk_x = 1150;
				frisk_y = 305;
				camera_x = 7038;
				camera_y = 2502;
				max_change = false;
				InvalidateRect(hWnd, NULL, false);
			}
			break;
		}
		case 28:
		{
			if (max_change) {
				move_direction = 4;
				frisk_x = 110;
				frisk_y = 465;
				camera_x = 7330;
				camera_y = 2532;
				max_change = false;
			}

			//카메라 이동x
			if (frisk_x > window_Width / 2 - frisk_game_width / 2 && !(camera_x >= 7405)) {
				camera_x += 3;
				camera_move_distance -= frisk_move_distance;
				frisk_x = window_Width / 2 - frisk_game_width / 2; // 중앙에 프리스크 고정
			}
			else if (frisk_x < window_Width / 2 - frisk_game_width / 2 && !(camera_x <= 7330)) {
				camera_x -= 3;
				camera_move_distance += frisk_move_distance;
				frisk_x = window_Width / 2 - frisk_game_width / 2; // Keep Frisk in thecenter
			}

			//frisk y축 이동
			if (frisk_y > window_Height / 2 - frisk_game_height / 2 && !(camera_y >= 2682)) {		//중앙보다 위에 있을때
				camera_y += 3;
				camera_move_distance -= frisk_move_distance;
				frisk_y = window_Height / 2 - frisk_game_height / 2; // 중앙에 프리스크 고정
			}
			else if (frisk_y < window_Height / 2 - frisk_game_height / 2 && !(camera_y <= 2532)) {	//중앙보다 아래에 있을때
				camera_y -= 3;
				camera_move_distance += frisk_move_distance;
				frisk_y = window_Height / 2 - frisk_game_height / 2; // 중앙에 프리스크 고정
			}

			//
			if (850 <= frisk_x && frisk_x <= 990 && frisk_y >= 745 &&
				camera_x == 7405 && camera_y == 2682) {
				map_number = 29;
				max_change = true;
				InvalidateRect(hWnd, NULL, false);
			}
			break;
		}
		case 29:
		{
			if (max_change) {
				move_direction = 2;
				frisk_x = 1060;
				frisk_y = 305;
				camera_x = 8100;
				camera_y = 2950;
				max_change = false;
			}

			//카메라 이동x
			if (frisk_x > window_Width / 2 - frisk_game_width / 2 && !(camera_x >= 8100)) {
				camera_x += 3;
				camera_move_distance -= frisk_move_distance;
				frisk_x = window_Width / 2 - frisk_game_width / 2; // 중앙에 프리스크 고정
			}
			else if (frisk_x < window_Width / 2 - frisk_game_width / 2 && !(camera_x <= 8051)) {
				camera_x -= 3;
				camera_move_distance += frisk_move_distance;
				frisk_x = window_Width / 2 - frisk_game_width / 2; // Keep Frisk in thecenter
			}


			//frisk y축 이동
			if (frisk_y > window_Height / 2 - frisk_game_height / 2 && !(camera_y >= 3100)) {		//중앙보다 위에 있을때
				camera_y += 3;
				camera_move_distance -= frisk_move_distance;
				frisk_y = window_Height / 2 - frisk_game_height / 2; // 중앙에 프리스크 고정
			}
			else if (frisk_y < window_Height / 2 - frisk_game_height / 2 && !(camera_y <= 2950)) {	//중앙보다 아래에 있을때
				camera_y -= 3;
				camera_move_distance += frisk_move_distance;
				frisk_y = window_Height / 2 - frisk_game_height / 2; // 중앙에 프리스크 고정
			}


			//
			if (465 <= frisk_y && frisk_y <= 545 && frisk_x <= 110 &&
				camera_x == 8049 && camera_y == 3100) {
				map_number = 30;
				max_change = true;
				InvalidateRect(hWnd, NULL, false);
			}
			break;
		}
		case 30:
		{
			if (max_change) {
				move_direction = 3;
				frisk_x = 1440;
				frisk_y = 505;
				camera_x = 7760;
				camera_y = 3100;
				max_change = false;
			}

			//카메라 이동x
			if (frisk_x > window_Width / 2 - frisk_game_width / 2 && !(camera_x >= 7760)) {
				camera_x += 3;
				camera_move_distance -= frisk_move_distance;
				frisk_x = window_Width / 2 - frisk_game_width / 2; // 중앙에 프리스크 고정
			}
			else if (frisk_x < window_Width / 2 - frisk_game_width / 2 && !(camera_x <= 7733)) {
				camera_x -= 3;
				camera_move_distance += frisk_move_distance;
				frisk_x = window_Width / 2 - frisk_game_width / 2; // Keep Frisk in thecenter
			}
			

			//frisk y축 이동
			if (frisk_y > window_Height / 2 - frisk_game_height / 2 && !(camera_y >= 3100)) {		//중앙보다 위에 있을때
				camera_y += 3;
				camera_move_distance -= frisk_move_distance;
				frisk_y = window_Height / 2 - frisk_game_height / 2; // 중앙에 프리스크 고정
			}
			else if (frisk_y < window_Height / 2 - frisk_game_height / 2 && !(camera_y <= 2953)) {	//중앙보다 아래에 있을때
				camera_y -= 3;
				camera_move_distance += frisk_move_distance;
				frisk_y = window_Height / 2 - frisk_game_height / 2; // 중앙에 프리스크 고정
			}

			//right
			if (270 <= frisk_x && frisk_x <= 490 && frisk_y <= 225 &&
				camera_x == 7733 && camera_y == 2953) {
				map_number = 31;
				max_change = true;
				InvalidateRect(hWnd, NULL, false);
			}
			break;
		}
		case 31:
		{
			if (max_change) {
				move_direction = 1;
				frisk_x = 425;
				frisk_y = 705;
				camera_x = 7733;
				camera_y = 2681;
				max_change = false;
			}

			//카메라 이동x
			if (frisk_x > window_Width / 2 - frisk_game_width / 2 && !(camera_x >= 7766)) {
				camera_x += 3;
				camera_move_distance -= frisk_move_distance;
				frisk_x = window_Width / 2 - frisk_game_width / 2; // 중앙에 프리스크 고정
			}
			else if (frisk_x < window_Width / 2 - frisk_game_width / 2 && !(camera_x <= 7733)) {
				camera_x -= 3;
				camera_move_distance += frisk_move_distance;
				frisk_x = window_Width / 2 - frisk_game_width / 2; // Keep Frisk in thecenter
			}


			//frisk y축 이동
			if (frisk_y > window_Height / 2 - frisk_game_height / 2 && !(camera_y >= 2681)) {		//중앙보다 위에 있을때
				camera_y += 3;
				camera_move_distance -= frisk_move_distance;
				frisk_y = window_Height / 2 - frisk_game_height / 2; // 중앙에 프리스크 고정
			}
			else if (frisk_y < window_Height / 2 - frisk_game_height / 2 && !(camera_y <= 2534)) {	//중앙보다 아래에 있을때
				camera_y -= 3;
				camera_move_distance += frisk_move_distance;
				frisk_y = window_Height / 2 - frisk_game_height / 2; // 중앙에 프리스크 고정
			}

			//right
			if (385 <= frisk_y && frisk_y <= 405 && frisk_x >= 1470 &&
				camera_x == 7766 && 2534 <= camera_y && camera_y <= 2546) {
				map_number = 32;
				max_change = true;
				InvalidateRect(hWnd, NULL, false);
			}
			break;
		}
		case 32:
		{
			if (max_change) {
				move_direction = 1;
				frisk_x = 125;
				frisk_y = 445;
				camera_x = 8072;
				camera_y = 2523;
				max_change = false;
			}

			//카메라 이동x
			if (frisk_x > window_Width / 2 - frisk_game_width / 2 && !(camera_x >= 8579)) {
				camera_x += 3;
				camera_move_distance -= frisk_move_distance;
				frisk_x = window_Width / 2 - frisk_game_width / 2; // 중앙에 프리스크 고정
			}
			else if (frisk_x < window_Width / 2 - frisk_game_width / 2 && !(camera_x <= 8072)) {
				camera_x -= 3;
				camera_move_distance += frisk_move_distance;
				frisk_x = window_Width / 2 - frisk_game_width / 2; // Keep Frisk in thecenter
			}


			//frisk y축 이동
			if(camera_x != 8579){
				if (frisk_y > window_Height / 2 - frisk_game_height / 2 && !(camera_y >= 2523)) {		//중앙보다 위에 있을때
					camera_y += 3;
					camera_move_distance -= frisk_move_distance;
					frisk_y = window_Height / 2 - frisk_game_height / 2; // 중앙에 프리스크 고정
				}
				else if (frisk_y < window_Height / 2 - frisk_game_height / 2 && !(camera_y <= 2352)) {	//중앙보다 아래에 있을때
					camera_y -= 3;
					camera_move_distance += frisk_move_distance;
					frisk_y = window_Height / 2 - frisk_game_height / 2; // 중앙에 프리스크 고정
				}
			}

			//right
			if (1210 <= frisk_x && frisk_x <= 1290 && frisk_y <= 205 &&
				camera_x == 8579 && camera_y == 2523) {
				map_number = 33;
				max_change = true;
				InvalidateRect(hWnd, NULL, false);
			}

			//up
			if (690 == frisk_x && frisk_y <= 265 &&
				8225 <= camera_x && camera_x <= 8252 && camera_y == 2352) {
				map_number = 34;
				max_change = true;
				InvalidateRect(hWnd, NULL, false);
			}
			break;
		}
		case 33:
		{
			if (max_change) {
				move_direction = 1;
				frisk_x = 770;
				frisk_y = 725;
				camera_x = 8680;
				camera_y = 2292;
				max_change = false;
			}

			//right
			if (710 <= frisk_x && frisk_x <= 840 && frisk_y >= 745 &&
				camera_x == 8680 && camera_y == 2292) {
				map_number = 32;

				move_direction = 2;
				frisk_x = 1250;
				frisk_y = 215;
				camera_x = 8579;
				camera_y = 2523;
				max_change = false;
				InvalidateRect(hWnd, NULL, false);
			}
			break;
		}
		case 34:
		{
			if (max_change) {
				move_direction = 1;
				frisk_x = 770;
				frisk_y = 720;
				camera_x = 8220;
				camera_y = 2122;
				max_change = false;
			}


			//frisk y축 이동
			if (camera_x != 8579) {
				if (frisk_y > window_Height / 2 - frisk_game_height / 2 && !(camera_y >= 2122)) {		//중앙보다 위에 있을때
					camera_y += 3;
					camera_move_distance -= frisk_move_distance;
					frisk_y = window_Height / 2 - frisk_game_height / 2; // 중앙에 프리스크 고정
				}
				else if (frisk_y < window_Height / 2 - frisk_game_height / 2 && !(camera_y <= 1885)) {	//중앙보다 아래에 있을때
					camera_y -= 3;
					camera_move_distance += frisk_move_distance;
					frisk_y = window_Height / 2 - frisk_game_height / 2; // 중앙에 프리스크 고정
				}
			}

			//up
			if (730 <= frisk_x && frisk_x <= 820 && frisk_y <= 365 &&
				8220 == camera_x && camera_y == 1885) {
				map_number = 35;
				max_change = true;
				InvalidateRect(hWnd, NULL, false);
			}
			break;
		}
		case 35:
		{
			if (max_change) {
				move_direction = 1;
				frisk_x = 770;
				frisk_y = 800;
				camera_x = 8223;
				camera_y = 1642;
				max_change = false;
			}

			//left 36, 37
			if (580 <= frisk_y && frisk_y <= 720 && frisk_x <= 110 &&
				8223 == camera_x && camera_y == 1642) {
				map_number = 36;
				max_change = true;
				InvalidateRect(hWnd, NULL, false);
			}

			//right 38, 39, 40
			if (580 <= frisk_y && frisk_y <= 720 && frisk_x >= 1470 &&
				8223 == camera_x && camera_y == 1642) {
				map_number = 38;
				max_change = true;
				InvalidateRect(hWnd, NULL, false);
			}

			//down door
			if (790 <= frisk_x && frisk_x <= 870 &&
				400 <= frisk_y && frisk_y <= 440 &&
				8223 == camera_x && camera_y == 1642) {
				map_number = 41;
				max_change = true;
				InvalidateRect(hWnd, NULL, false);
			}
			break;
		}
		case 36:
		{
			// home left - 1
			if (max_change) {
				move_direction = 3;
				frisk_x = 1450;
				frisk_y = 620;
				camera_x = 7902;
				camera_y = 1648;
				max_change = false;
			}

			//right door
			if (560 <= frisk_y && frisk_y <= 700 && frisk_x >= 1470	 &&
				7902 == camera_x && camera_y == 1648) {
				map_number = 35;

				move_direction = 4;
				frisk_x = 150;
				frisk_y = 640;
				camera_x = 8223;
				camera_y = 1642;
				max_change = false;
				InvalidateRect(hWnd, NULL, false);
			}

			//up door
			if (270 <= frisk_x && frisk_x <= 410 && frisk_y <= 0 &&
				7902 == camera_x && camera_y == 1648) {
				map_number = 37;
				max_change = true;
				InvalidateRect(hWnd, NULL, false);
			}
			break;
		}
		case 37:
		{
			// home left - 1
			if (max_change) {
				move_direction = 1;
				frisk_x = 380;
				frisk_y = 800;
				camera_x = 7894;
				camera_y = 1414;
				max_change = false;
			}

			//down door
			if (310 <= frisk_x && frisk_x <= 450 && frisk_y >= 820 &&
				7894 == camera_x && camera_y == 1414) {
				map_number = 36;

				move_direction = 2;
				frisk_x = 340;
				frisk_y = 40;
				camera_x = 7902;
				camera_y = 1648;
				max_change = false;
				InvalidateRect(hWnd, NULL, false);
			}
			break;
		}
		case 38:
		{
			// home right - 1
			if (max_change) {
				move_direction = 4;
				frisk_x = 120;
				frisk_y = 660;
				camera_x = 8530;
				camera_y = 1640;
				max_change = false;
			}

			//카메라 이동x
			if (frisk_x > window_Width / 2 - frisk_game_width / 2 && !(camera_x >= 8965)) {
				camera_x += 3;
				camera_move_distance -= frisk_move_distance;
				frisk_x = window_Width / 2 - frisk_game_width / 2; // 중앙에 프리스크 고정
			}
			else if (frisk_x < window_Width / 2 - frisk_game_width / 2 && !(camera_x <= 8530)) {
				camera_x -= 3;
				camera_move_distance += frisk_move_distance;
				frisk_x = window_Width / 2 - frisk_game_width / 2; // Keep Frisk in thecenter
			}

			//up 1 door
			if (8554 <= camera_x && camera_x <= 8572 && frisk_y <= 540 &&
				690 == frisk_x && camera_y == 1640) {
				map_number = 39;
				max_change = true;
				InvalidateRect(hWnd, NULL, false);
			}

			//up 2 door
			if (8761 <= camera_x && camera_x <= 8782 && frisk_y <= 540 &&
				690 == frisk_x && camera_y == 1640) {
				map_number = 40;
				max_change = true;
				InvalidateRect(hWnd, NULL, false);
			}

			//left  door
			if (580 <= frisk_y && frisk_y <= 740 && frisk_x <= 110 &&
				8530 == camera_x && camera_y == 1640) {
				map_number = 35;

				move_direction = 3;
				frisk_x = 1450;
				frisk_y = 640;
				camera_x = 8223;
				camera_y = 1642;
				max_change = false;
				InvalidateRect(hWnd, NULL, false);
			}
			break;
		}
		case 39:
		{
			// home right - 2
			if (max_change) {
				move_direction = 1;
				frisk_x = 940;
				frisk_y = 780;
				camera_x = 8511;
				camera_y = 1483;
				max_change = false;
			}

			//down 1 door
			if (870 <= frisk_x && frisk_x <= 1000 && frisk_y >= 800 &&
				8511 == camera_x && camera_y == 1483) {
				map_number = 38;

				move_direction = 2;
				frisk_x = 690;
				frisk_y = 580;
				camera_x = 8563;
				camera_y = 1640;
				max_change = false;
				InvalidateRect(hWnd, NULL, false);
			}
			break;
		}
		case 40:
		{
			// home right - 3
			if (max_change) {
				move_direction = 1;
				frisk_x = 520;
				frisk_y = 780;
				camera_x = 8809;
				camera_y = 1481;
				max_change = false;
			}

			//down 1 door
			if (440 <= frisk_x && frisk_x <= 580 && frisk_y >= 800 &&
				8809 == camera_x && camera_y == 1481) {
				map_number = 38;

				move_direction = 2;
				frisk_x = 690;
				frisk_y = 580;
				camera_x = 8773;
				camera_y = 1640;
				max_change = false;
				InvalidateRect(hWnd, NULL, false);
			}
			break;
		}
		case 41:
		{
			// home right - 3
			if (max_change) {
				move_direction = 4;
				frisk_x = 380;
				frisk_y = 300;
				camera_x = 8530;
				camera_y = 1873;
				max_change = false;
			}

			//카메라 이동x
			if (frisk_x > window_Width / 2 - frisk_game_width / 2 && !(camera_x >= 8761)) {
				camera_x += 3;
				camera_move_distance -= frisk_move_distance;
				frisk_x = window_Width / 2 - frisk_game_width / 2; // 중앙에 프리스크 고정
			}
			else if (frisk_x < window_Width / 2 - frisk_game_width / 2 && !(camera_x <= 8530)) {
				camera_x -= 3;
				camera_move_distance += frisk_move_distance;
				frisk_x = window_Width / 2 - frisk_game_width / 2; // Keep Frisk in thecenter
			}

			//right door
			if (240 <= frisk_y && frisk_y <= 340 && frisk_x >= 1470 &&
				8761 == camera_x && camera_y == 1873) {
				map_number = 42;
				max_change = true;
				InvalidateRect(hWnd, NULL, false);
			}
			break;
		}
		case 42:
		{
			// home right - 3
			if (max_change) {
				move_direction = 4;
				frisk_x = 120;
				frisk_y = 300;
				camera_x = 8730;
				camera_y = 1873;
				max_change = false;
			}

			//right door
			if (240 <= frisk_y && frisk_y <= 340 && frisk_x >= 1470 &&
				8730 == camera_x && camera_y == 1873) {
				map_number = 43;
				max_change = true;
				InvalidateRect(hWnd, NULL, false);
			}
			break;
		}
		case 43:
		{
			// 코너
			if (max_change) {
				move_direction = 4;
				frisk_x = 120;
				frisk_y = 620;
				camera_x = 9473;
				camera_y = 1800;
				max_change = false;
			}

			//right door
			if (580 <= frisk_x && frisk_x <= 910 && frisk_y <= 0 &&
				9473 == camera_x && camera_y == 1800) {
				map_number = 44;
				max_change = true;
				InvalidateRect(hWnd, NULL, false);
			}
			break;
		}
		case 44:
		{
			//
			if (max_change) {
				move_direction = 1;
				frisk_x = 770;
				frisk_y = 800;
				camera_x = 9468;
				camera_y = 1581;
				max_change = false;
			}

			//
			if (680 <= frisk_x && frisk_x <= 810 && frisk_y <= 260 &&
				9468 == camera_x && camera_y == 1581) {
				
				//토리엘 다음 문
				map_number = 45;
				max_change = true;
				InvalidateRect(hWnd, NULL, false);
			}
			break;
		}
		}

		//카메라
		img_map.Draw(mDC, 100, 0, window_Width, window_Height, camera_x, camera_y, camera_width, camera_height);

		//충돌 박스
		for (int i{}; i < numRects; ++i) {

			hPen = CreatePen(PS_SOLID, 3, RGB(255, 0, 0));
			hOldPen = (HPEN)SelectObject(mDC, hPen);

			hBrush = (HBRUSH)SelectObject(mDC, GetStockObject(NULL_BRUSH));
			Rectangle(mDC, collisionRects[i].left - camera_x, collisionRects[i].top - camera_y, collisionRects[i].right - camera_x, collisionRects[i].bottom - camera_y);

			SelectObject(mDC, hOldPen);
			DeleteObject(hBrush);
			DeleteObject(hPen);
		}

		//캐릭터 박스
		{
			hPen = CreatePen(PS_SOLID, 3, RGB(0, 255, 0));
			hOldPen = (HPEN)SelectObject(mDC, hPen);

			hBrush = (HBRUSH)SelectObject(mDC, GetStockObject(NULL_BRUSH));
			Rectangle(mDC, frisk_x, frisk_y, frisk_x + 20 * 3, frisk_y + 30 * 3);

			SelectObject(mDC, hOldPen);
			DeleteObject(hBrush);
			DeleteObject(hPen);
		}

		// battle
		if (battle_mode)
		{

		}

		//draw frisk
		switch (move_direction) { // 0: 멈춤, 1: 위, 2: 아래, 3: 왼쪽, 4: 오른쪽
		case 1:
			img_frisk_back[img_frame].Draw(mDC, frisk_x, frisk_y, frisk_game_width, frisk_game_height, 0, 0, frisk_img_width, frisk_img_height);
			break;
		case 2:
			img_frisk_front[img_frame].Draw(mDC, frisk_x, frisk_y, frisk_game_width, frisk_game_height, 0, 0, frisk_img_width, frisk_img_height);
			break;
		case 3:
			img_frisk_left[img_frame].Draw(mDC, frisk_x, frisk_y, frisk_game_width, frisk_game_height, 0, 0, frisk_img_width, frisk_img_height);
			break;
		case 4:
			img_frisk_right[img_frame].Draw(mDC, frisk_x, frisk_y, frisk_game_width, frisk_game_height, 0, 0, frisk_img_width, frisk_img_height);
			break;
		}

		{
			//print frisk pos(frisk_width, frisk_width, frisk_x, frisk_y
			wsprintf(text, L"frisk x : %d, y : %d", frisk_x, frisk_y);
			TextOut(mDC, 0, 0, text, lstrlen(text));

			//map frisk pos
			wsprintf(text, L"map x : %d, y : %d", camera_x, camera_y);
			TextOut(mDC, 0, 15, text, lstrlen(text));

			//map frisk pos
			wsprintf(text, L"mouse x : %d, y : %d", mx+ camera_x, my+ camera_y);
			TextOut(mDC, 0, 30, text, lstrlen(text));

			//map위치에 적용 시킨 frisk 위치
			wsprintf(text, L"m_frisk x : %d, y : %d", frisk_x + camera_x, frisk_y + camera_y);
			TextOut(mDC, 0, 45, text, lstrlen(text));

			//map num
			wsprintf(text, L"map : %d", map_number);
			TextOut(mDC, 0, 60, text, lstrlen(text));
		}

		//마지막에 메모리 dc의 내용을 화면 dc로 복사한다.
		BitBlt(hDC, 0, 0, rt.right, rt.bottom, mDC, 0, 0, SRCCOPY);
		DeleteDC(mDC);
		DeleteObject(hBitmap);
		EndPaint(hWnd, &ps);
		break;
	case WM_MOUSEMOVE:
		mx = LOWORD(IParam);
		my = HIWORD(IParam);

		InvalidateRect(hWnd, NULL, false);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, IParam);
}
