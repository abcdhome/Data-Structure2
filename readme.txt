해결하지 못한 문제점: 리눅스에서 검색중일때 esc로 취소를 하고 싶으면 esc를 두번눌러줘야 한다.
아무 텍스트도 안쓰고 엔터를 누를경우 다음에 텍스트가 써지기는 하나(기능은 작동한다.) 보이지가 않는다
리눅스에서는 아무 이름도 없는것을 저장할때 메세지바에 save:뒤에 글자를 적을때 텍스트가 안보여서 실제로 같은 공간에 저장은 되나 
저장을 하고 나서 글을 적으면 보이지가 않아 벡스페이스바로 글자가 있는 곳 까지 지우고 다시 적어야한다.

주의 사항: cmd에서는 .\viva라고 입력을 해야 실행이 된다 리눅스는 ./viva
실행된 프로그램에서 ./viva로 viva.c 파일을 만들고 ctrl+s로 저장을 해야 파일이 같은 공간에 생성이 되고
./viva viva.c를 이용해 파일을 불러올 수 있다

제 나름대로 최선을 다하여 오류도 찾아보고 수정 해보았습니다... .o파일은 makefile이 작동하는 확인을 위하여 빼고 보냈습니다

main.c:
1.윈도우 사용을 위해 window.h, conio.h 리눅스 사용을 위해 termios.h, unistd.h, sys/ioctl.h 사용
리눅스에서는 ctrl-q ctrl-s가 안되길래 system("stty -ixon")으로 설정하여 가능하게 만듬
2. ./viva를 입력하면 filenamed를 사용해 파일 이름을 변경하고 텍스트를 초기화 이때 파일 이름이 viva.c로 바뀌고 메세지는 filename change가 나온다
3. ./viva viva.c를 입력하면 같은 공간에 있는 viva.c 파일을 open_file로 불러옴 기존에 써둔 글자들은 지워지고 viva.c 파일의 텍스트를 불러온다
file open 이라는 메세지가 나온다
4. ctrl-s를 적으면 아무 이름도 없을경우 이름을 적으라고 메세지바에 나오고 메세지 바 옆에 적을 수 있다 이때 세이브를 하면서 바탕화면에
파일이 생성이 되고 저장이 된다. 파일 이름이 있을경우 기존 파일에 저장을 한다
5. ctrl-q를 누르면 저장이 안되어 있을경우 나갈거냐고 메세지바에서 물어보고 다시 ctrl-q를 누르면 나가진다
저장이 되어있을경우 ctrl-q를 누르면 바로 나간다.
6. ctrl-f를 누르면 검색을 시작한다 sdfs 라고 적혀 있다고 가정하에 s를 검색하면 맨앞 s가 하이라이트가 되고 
방향키 위 아래를 누르면 맨 마지막 s로 하이라이트랑 커서가 이동이 되고 기존에 있던 s의 하이라이트는 사라진다
search_text 검색을 하여 찾는 과정에서 wrap_around 방식을 사용하여 끝에서 다시 처음으로 이동할때 제대로 계산이 된다
제대로 설정을 안하여 맨마지막꺼만 검색이 안되는 오류가 있었다...
stop_search1 / stop_search2 엔터를 누르면 검색이 취소되고 마지막 검색 위치에서 종료가 되며 esc를 누르면 검색 이전의 위치에서 종료가 된다
7. escesc() 리눅스 전용 방향키로 getchar로 받고 상하좌우 pageup down home end를 구현
검색중잉일때 ch == '\033' esc를 누르면 special_key에 해당하는 상 하로 이동하여 검색을 할 수 있다
검색중일떄도 백스페이스를 사용하여 지울 수 있다.


text.c,h:
1. display() 화면 갱신 무슨 짓을 해보아도 system("cls")를 안 사용하면 오류가 나버림
unix/linux에도 system("clear") 하면 또 오류가 나서 ("\033[H\033[J") 사용
2. a_lines() 새로운 줄 추가 현재 줄 아래의 모든 줄을 한 칸씩 아래로 이동
3. insert() 문자 삽입 커서가 가리키는 위치에 문자를 삽입 중간에 삽입하면 memmove로 문자열을 오른쪽으로 밀기
4. backspace() 문자 삭제 커서가 중간에 있으면 왼쪽문자 삭제 memmove로 이동
커서가 줄의 시작에 있으면 strcat로 병합 이후 줄들을 한줄씩 위로 이동
5. home end 커서를 그 줄에 맨앞 맨뒤로 이동
6. page up, page down 커서를 모든 글 중에서 맨앞으로 이동 커서를 모든 글중에서 맨뒤로 이동
7. up down left right 커서를 위 아래 좌 우로 이동
8. char text[MAX_LINES][MAX_COLS]; 배열을 사용해 max lines = 1000 max_cols= 80으로 지정
9. h파일에 main.c에서 사용하기 위해 외부 선언이 존재함

terminal.c,h:
1. terminal1 행과 열을 포인터로 저장하고 CONSOLE_SCREEN_BUFFER_INFO라는 windows.h의 구조체를 사용 
csbi.srwindow.top,left,right,bottom등을 사용하여 화면의 위치를 조정
2. GetConsoleScreenBufferInfo std_output_handle을 사용해 출력 콘솔 핸들을 전달하고 csbi 행과 열의 수를 계산
3. 리눅스에서는 winsize 구조체를 사용하여 ws_row,col로 행과 열의 수를 정의
4. ioctl() stdout_fileno(표준 출력 핸들)과 tiocgwinsz터미널 크기 요청)을 전달 터미널 크기를 winsize 구조체에 저장

stbar.c,h:
1. s_stbar() 상태바를 초기화 status_bar.filename에 복사 처음에는 줄 수 0 커서 위치 0,0으로 조정 계산을 위해 co_li(filename)호출
2. d_stbar() 상태바 출력 terminal1으로 터미널 동적으로 위치 가져오기
3. \033[7m과 \033[0m 텍스트 하이라이트 표시
3. co_li 파일의 줄 수 계산 및 텍스트 배열 초기화 파일을 읽기 모드로 열고 fgets로 읽어 텍스트 배열에 저장
읽은 줄 수는 line_count로 계산하고 status_bar.total_lines에 저장
4. up(cursor) 커서 위치 업데이트  새로운 커서 위치를 status_bar.cursor,row,col에 반영
5. 현재 텍스트 배열의 줄 수 계산해 상태바에 반영


msgbar.c,h:
1. #define DEFAULT_MESSAGE "HELP: Ctrl-S = save | Ctrl-Q = quit | Ctrl-F = find" 메세지바에 나오는 초기화면 설정
2. s_msgbar() 메세지바 초기 상태를 설정하고 화면에 출력
3. DEFAULT_MESSAGE를 Mmsg_bar.message에 복사하고 strncpy를 사용해 버퍼 오버플로우 방지
4. d_msgbar() 메세지바 출력 메세지바를 화면 마지막줄에 표시 terminal1 함수로 동적으로 위치 가져오기
5. up_msgbar(const char *new_message) 특정 행동을 할때 발생되는 이벤트를 메세지로 적기