# Excavator_Simulator-OpenGL

### 굴삭기 시뮬레이터

<br>

#### 프로젝트 소개

OpenGL을 활용하여 굴삭기를 모델링하고 동작시키는 시뮬레이터를 구현한다.



<br>

#### 구현 내용

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

#### 조작 방법

- 이동: W, A, S, D
- 시점 변경: 스페이스바
- 땅파기: 엔터
- 프로그램 종료: ESC
- 카메라 이동: 마우스 드래그



<br>

#### 프로그램 실행 방법

 "Excavator_Simulator" 폴더 내에 exe파일 실행

<br>

#### 프로젝트 설치 방법

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

