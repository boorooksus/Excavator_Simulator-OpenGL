# Excavator_Simulator-OpenGL

### 굴삭기 시뮬레이터

<br>

### 프로젝트 소개

OpenGL을 활용하여 굴삭기를 모델링하고 동작시키는 시뮬레이터를 구현한다.



<br>

### 구현 내용

![ezgif com-gif-maker](https://user-images.githubusercontent.com/55964775/103000445-6e0d0c00-456e-11eb-9140-7b3191316830.gif)

굴살기 형태 및 이동 구현

<br>

![그림3](https://user-images.githubusercontent.com/55964775/102999293-1a012800-456c-11eb-9671-4da7e0bb4ac3.png)

1인칭 시점

<br>

![ezgif-3-62cef176b224](https://user-images.githubusercontent.com/55964775/102999845-276ae200-456d-11eb-86ec-0480cb489f7a.gif)

땅파기 동작

<br>

![ezgif com-gif-maker (1)](https://user-images.githubusercontent.com/55964775/103001264-f8a23b00-456f-11eb-9e91-4f1def3763df.gif)

카메라 이동



<br>

### 조작 방법

- 이동: W, A, S, D
- 시점 변경: 스페이스바
- 땅파기: 엔터
- 프로그램 종료: ESC
- 카메라 이동: 마우스 드래그



<br>

### 프로그램 실행 방법

 "Excavator_Simulator" 폴더 내에 exe파일 실행

<br>

### 프로젝트 설치 방법

1. openGL 튜토리얼 다운로드(http://www.opengl-tutorial.org/download/)
2. Cmake 설치(https://cmake.org/download/)
3. 소스 파일 빌드
   - cmake_gui.exe 실행 후 "Where is the source code" 항목에 openGL 튜토리얼 소스 폴더 지정
   -  openGL 튜토리얼 소스 폴더와 동일한 경로 내에 "opengl_project" 폴더 생성 후 Cmake Where is the build the binaries" 항목에 해당 경로 지정
4. 소스 파일 빌드
   - Cmake에서 Configure 버튼 클릭 후 Visual studio 버전 선택
   - Finish 버튼을 누른 후 메시지창에서 "Configuring done" 문구 확인
   - Generate 버튼 클릭 후 "generating done" 문구 확인
5. opengl_project 폴더 내에 생성된 "Tutorial.sln" 실행
6. 굴삭기 프로젝트 추가
   - 새로운 프로젝트를 추가하거나 Tutorial 프로젝트들 중 하나를 비워둔다.
   - 프로젝트 폴더와 "common" 폴더에 각각 깃 허브에 올린 "Excavator_Simulator" 폴더 내의 자료들과"common" 폴더 자료들을 옮긴다.

<br>

### 프로젝트 구현 방법

\-    **형태 구현**

굴삭기의 형태를 구현하기 위해 조종대, 바퀴 2개, 핸들 3개, 버킷을 만들었다. 이를 위하여 큐브 오브젝트인 ‘cube.obj’를 이용했으며, 총 10개의 큐브를 사용하였다. 굴삭기의 조종대는 실제감을 높이기 위해 4개의 큐브가 사용되었으며 각각 몸통, 운전석, 좌우 창문, 앞뒤 창문을 담당한다. 나머지 2개의 바퀴, 3개의 핸들, 버킷은 각각 큐브를 1개씩만 사용되었다. 또한 큐브 1개를 추가로 더 사용하여 땅을 구현하였다. 이는 1인칭 시점에서 굴착기의 이동을 잘 표현하기 위해서이다. 

각각의 큐브들은 main 함수 내에서 ‘gPosition1~12’ 변수를 통해 위치하는 좌표가 지정된다. 그 다음, 렌더링 루프를 돌며 각각의 큐브들의 model matrix를 변환, 회전 및 Scailing을 한 뒤 그려낸다. 그리고 큐브를 그릴 때 ‘ColorCheckID’와 ‘glUniform1i’ 함수를 이용하여 적절한 색상을 입힌다. 지정한 ‘ColorCheckID’ 값에 따른 색상은 ‘ColorFragmentShader.fragmentshader’에 저장되어있다. 그러나 땅을 표현한 큐브는 색상 대신 텍스쳐를 입혀 표현하였다. 땅 큐브에 매핑된 파일은 ‘ground.bmp’ 파일이며 main 함수 내에서 load되었다. ‘glUniform1i’ 함수를 실행할 때 ‘ColorCheck’ 값이 11로 지정된 경우 ‘ColorFragmentShader.fragmentshader’에서 색상 대신 ‘ground.bmp’파일 texture를 mapping하도록 하였다.

\-    **마우스 입력을 통한 카메라 이동**

 마우스 왼쪽 버튼클릭 후 드래그를 통해 카메라의 이동이 가능하다. 또한 키보드의 스페이스바를 누르면 1인칭으로 시점이 전환된다. 이를 구현하기 위하여 main함수 내의 while 루프에서 마우스 왼쪽 버튼 입력을 확인한다. 만약 마우스 왼쪽 버튼 입력이 확인되었다면 버튼을 클릭했을 때의 마우스 커서 좌표를 저장하고 ‘computeMouseRotates’ 함수를 실행시킨다. 이때, 현재 카메라가 전체 시점이 아니라면, 이 과정을 수행하지 않고 넘어간다. 1인칭 시점에서는 카메라가 마우스 입력에 따라 이동하지 않고 굴삭기에 종속되어 따라가기 때문이다. 

 ‘computeMouseRotates’ 함수에서는 프레임 간 시간을 계산하고, 커서 위치를 기반으로 회전 각도를 결정한 뒤, ‘eulerAngleYXZ’ 함수를 이용하여 ‘View’ matrix를 회전시켜 카메라의 위치를 이동시킨다. 마지막으로 현재 커서의 위치와 시간을 저장한 뒤 함수를 종료한다.

\-    **키보드 입력을 통한 굴삭기 이동**

 키보드의 ‘W’, ‘A’, ‘S’, ‘D’ 키를 눌러 굴삭기를 이동시킬 수 있게 하였다. 이를 구현하기 위하여 main 함수 내의 while 루프를 돌면서 ‘computeKeyboardTranslates’ 함수를 실행시켜 키보드의 ‘W’, ‘A’, ‘S’, ‘D’ 키가 눌렸는지 확인한다. 키를 누른것이 확인되었다면, 프레임 간 시간을 계산하고 방향 벡터를 이용하여 ‘translateFactor’ 변수에 이동 벡터를 계산한 뒤, ‘View’ matrix를 ‘translateFactor’ 만큼 translate 시킨다. 여기서 translate는 전체 시점인 경우에만 실행하고 1인칭 시점인 경우에는 실행하지 않는다. 그리고 ‘moving’ 변수에도 ‘translateFactor’ 값을 더한다. ‘moving’ 변수는 main 함수의 while 루프 내에서 땅의 model matrix를 translate할 때 사용된다. 이렇게 하면 실제로는 굴삭기가 가만히 있으나, 카메라와 땅이 움직여서 굴삭기가 움직이는 것처럼 보인다. 만약 이렇게 하지 않고 moving 변수에 ‘translateFactor’ 값을 더해주지 않는다면 카메라만 움직이고 굴삭기가 땅에 붙어서 안움직이는 것처럼 보이게 되므로 땅을 translate 하는 과정은 꼭 필요하다.

\-    **땅파기 동작**

키보드의 엔터 키를 누르면 굴삭기가 땅파기 동작을 수행한다. 3개의 핸들과 버킷을 일정 범위만큼 동시에 회전시켜 땅을 파낸다. 땅파기 동작 수행 중에는 굴삭기의 이동이 가능하지만 시점 전환이 불가능하다. 또한 전체 시점에서만 땅파기 동작이 수행 가능하도록 하였다. 

 땅파기 동작을 구현하기 위해 main 함수의 while 루프에서 ‘computeKeyboardTranslates’ 함수를 실행시켜 엔터 키 입력을 체크한다. 엔터 키가 입력되고, 현재 땅파기 동작을 수행하지 않고 있으며, 현재 시점이 전체시점일 때에만 ‘isDigging’ 변수 값을 1로 변경하고 ‘lastDiggingTime’ 변수에 현재 시각을 저장한다. ‘isDigginig’ 변수는 현재 땅파기 동작을 수행 중인지 알려주는 변수이다. 만약 수행 중이라면 1, 아니라면 0을 저장한다. ‘lastDiggingTime’ 변수는 가장 최근에 땅파기 동작을 시작한 시각을 저장한다. 

 while 루프에서 ‘computeKeyboardTranslates’ 수행한 뒤에는 ‘digging’ 함수를 실행시킨다. ‘digging’ 함수는 ‘isDigging’ 변수 값이 1일 때 땅파기 동작을 수행시키는 함수이다. 우선 ‘gOrientation’ 값이 0.5f보다 크면 ‘flag’ 변수를 1로 저장하고, ‘gOrientation’ 값이 -0.5f보다 작으면 ‘flag’ 변수를 0으로 저장한다. ‘gOrientation’ 변수는 main함수의 렌더링 루프에서 핸들과 버킷의 model matrix를 rotation할 때 사용되고, ‘flag’ 변수는 회전 방향을 결정하는 변수이다. 즉, 핸들과 버킷의 회전각이 경계 값을 넘어가면 회전 방향을 바꿔준다. 그 다음, 위치에 따른 속도 값을 달리하며 ‘gOrientation’ 값을 변화시켜 핸들과 버킷이 진자 운동을 수행하도록 해준다. 위 과정을 1.25초 동안 수행한 뒤, ‘gOrientation’와 ‘isDigging’ 변수를 0으로 초기화 시켜 땅파기 동작 수행을 마친다. 현재 시각 변수인 ‘currentTime’변수 값에 ‘lastDiggingTime’ 변수 값을 뺀 값이 1.25f를 초과하는지 확인하면 땅파기 동작 수행을 시작한지 1.25초가 지났는지 확인 가능하다.

\-    **시점 전환**

키보드의 스페이스바를 누르면 카메라 시점이 전환된다. 전체 시점에서는 1인칭 시점으로 바뀌고, 1인칭 시점에서는 다시 전체 시점으로 바뀐다. 1인칭 시점은 카메라가 굴삭기 앞에 종속된다. 1인칭 시점에서는 마우스를 통한 카메라 이동이 불가능하다. 전체 시점은 기본 카메라 시점으로, 마우스를 이용한 카메라 이동이 가능하다. 그리고 땅파기 동작 수행 중에는 시점 변환이 불가능하다.

 시점 변환을 구현하기 위해 main 함수의 while 루프 내에 ‘computeKeyboardTranslates’ 함수를 이용하여 가장 최근에 스페이스바를 누른 지 0.5초를 초과한 상태에서 새로운 스페이스바 입력이 들어왔는지 확인한다. 스페이스바 입력 간격이 0.5초를 초과하는지 확인하지 않으면 키보드의 스페이스바를 한 번 누르고 떼는 속도보다 main 함수의 while 루프를 도는 속도가 더 빠르기 때문에 불필요한 시점 전환이 많이 일어나게 된다. 이를 위하여 ‘lastSpaceTime’ 변수에 가장 최근에 시점 변환한 시각을 저장하여 현재 시각과 비교한다.

 위 조건을 만족한 상태에서 새로운 스페이스바 입력이 확인되었다면, ‘isFP’ 변수 값을 체크한다. ‘isFP’ 변수는 현재 시점이 1인칭인지 여부를 저장한 변수이다. 현재 시점이 전체 시점이라면 0, 1인칭 시점이라면 1이 저장된다. 만약 ‘isFP’ 값이 0이고 ‘isDigging’ 값이 0이라면 카메라를 전체 시점에서 1인칭 시점으로 변경한다. 우선 ‘lastTPView’ 변수에 현재 ‘View’ 값을 저장한다. ‘lastTPView’는 가장 최근 전체 시점에서의 ‘View’ matrix 값을 저장하는 변수이다. 그 다음 ‘isFP’ 값을 1로 바꾸고, ‘View’ matrix 값에 좌표를 지정하고 translate과 rotation을 통해 카메라가 굴삭기 앞에 위치하도록 만든다. 

 만약 ‘isFP’ 값이 1이라면 카메라를 1인칭 시점에서 전체 시점으로 변경한다. 우선 ‘isFP’ 값을 다시 0으로 바꾸고, ‘View’ matrix 값에 ‘lastTPView’ 값을 넣어 마지막 전체 시점에서의 카메라로 돌아간다.


<br>

### 구현시 문제점 및 아쉬운점

키보드 'A', 'D' 키를 통한 좌우 이동이 아쉽다. 

현재 프로그램에서 키보드 'A' 또는 'D' 키를 눌렀을 때, 다음과 같이 굴삭기가 바라보는 방향은 고정된 채로 좌우로 움직여 현실감이 떨어진다.

![좌우이동](https://user-images.githubusercontent.com/55964775/103473513-c0ef8a80-4ddc-11eb-988c-67310bc7470d.gif)

이러한 문제점을 해결하기 위해 바닥 큐브의 로테이션 매트릭스에서 회전을 담당하는 'direction' 변수를 추가하고, 키보드 'A' 또는 'D' 키를 입력 받았을 때 'direction' 변수의 값을 변화시켜 바닥 큐브를 회전시켜보았다. 이렇게 하면 마치 굴삭기가 좌우로 방향을 트는 것처럼 보인다.

- main 함수의 while 루프 내에 있는 바닥 object 

![direction1](https://user-images.githubusercontent.com/55964775/103473626-1c6e4800-4dde-11eb-959c-c1b299258b52.JPG)

- 키보드 입력 담당 함수

![direction2](https://user-images.githubusercontent.com/55964775/103473628-2132fc00-4dde-11eb-9c94-5f262fe3a96c.JPG)

하지만 다음과 같이 굴삭기가 좌우로 회전한 뒤에 앞뒤로 움직이면 자연스럽게 움직이지 않고 굴삭기와 바닥면이 따로 노는 것 같은 느낌이 든다. 

![회전](https://user-images.githubusercontent.com/55964775/103473510-bcc36d00-4ddc-11eb-9a06-fdaa4a90a9cf.gif)

이는 굴삭기가 방향을 바꿀 때에는 바닥면을 회전시켜 상대적으로 굴삭기의 방향이 바뀌는 것으로 구현하였으나, 굴삭기가 앞뒤로 움직일 때에는 좌표의 절대적인 위치에 따라 움직이기 때문이다.  따라서 굴삭기가 오른쪽 방향으로 90도 회전한 상태에서 키보드 'W' 키를 누르게 되면, 굴삭기가 앞으로 가지 않고 왼쪽으로 움직이는것처럼 보인다.

 이러한 문제점으로 인해 굴삭기의 좌우 이동 방식을 변경하기 위한 다른 해결책을 모색해야겠다.
 
 <br>
 ### 참고 강의
인하대학교 2020년 2학기 컴퓨터 그래픽스

