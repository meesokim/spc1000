# Circle-to-SDL2 호환성 레이어 (Linux Native)

본 문서는 PC(Linux Native) 환경에서 SPC-1000 베어메탈 에뮬레이터 코드([kernel.cpp](file:///home/msx/spc1000/baremetal/kernel.cpp))를 별도의 큰 수정 없이 그대로 컴파일하고 실행 및 디버깅할 수 있도록 구현한 **Circle-to-SDL2 호환성 레이어**에 대해 설명합니다.

---

## 1. 아키텍처 및 원리

베어메탈 라즈베리 파이 라이브러리인 **Circle**의 API 및 하드웨어 연동 구조를 데스크톱 환경의 **SDL2** 라이브러리로 투명하게 매핑합니다.

```
       +--------------------------------------------+
       |   spc1000/baremetal/kernel.cpp (에뮬레이터) |
       +--------------------+-----------------------+
                            |
                     (HOST_COMPILE 정의)
                            |
                            v
       +--------------------+-----------------------+
       |     baremetal/host_include/ (모킹 헤더)    |
       +--------------------+-----------------------+
                            |
                            v
       +--------------------+-----------------------+
       |      baremetal/host_circle/ (SDL2 C++)     |
       +--------------------+-----------------------+
                            |
                   (SDL2 / POSIX API 사용)
                            |
                            v
       +--------------------+-----------------------+
       |            Linux 호스트 (화면/소리)         |
       +--------------------------------------------+
```

호스트 빌드 컴파일 시 `-DHOST_COMPILE` 매크로를 정의하고 포함 헤더 경로의 최우선순위를 `host_include/`로 잡아주면, 에뮬레이터 내에서 호출되는 `#include <circle/...>` 및 `#include <fatfs/...>` 코드들이 SDL2 모킹 스텁으로 자연스럽게 전환됩니다.

---

## 2. 주요 연동 컴포넌트

### A. 비디오 및 프레임버퍼 (`host_circle/screen.cpp`)
*   **CScreenDevice** 및 **CBcmFrameBuffer**:
    *   320x240 해상도의 16bpp (RGB565) 프레임버퍼 메모리를 가상으로 할당합니다.
    *   `SDL_CreateWindow`, `SDL_CreateRenderer`, `SDL_CreateTexture`를 통해 SDL2 창(2배율 확대 윈도우)을 띄우고, 가상 프레임버퍼 데이터를 실시간으로 비디오 카드에 로드 및 드로우(60 FPS 제한)합니다.

### B. 사운드 및 PSG (`host_circle/sound.cpp`)
*   **CVCHIQSoundBaseDevice**:
    *   SDL2 Audio spec (`AUDIO_S16SYS`, 1 채널 모노, 44100Hz)을 활성화합니다.
    *   백그라운드 SDL2 오디오 콜백 스레드(`HostAudioCallback`)가 주기적으로 구동되어 에뮬레이터 PSG 칩셋으로부터 음원을 생성하는 `GetChunk()`를 직접 호출하고 이를 오디오 디바이스로 송출합니다.

### C. 키보드 입력 (`host_circle/screen.cpp`)
*   **CUSBKeyboardDevice**:
    *   SDL2 이벤트 폴링 과정에서 탐지된 키 코드를 USB HID 키 표준 스캔코드로 실시간 매핑합니다.
    *   Shift, Ctrl, Alt 등의 Modifier 키 정보와 동시에 눌린 최대 6개 키의 USB 값을 조합하여 `CKernel::KeyStatusHandlerRaw` 콜백 함수로 인젝션합니다.
    *   `F12`(시스템 콜드리셋), `F10`(시스템 웜리셋) 핫키 인젝션을 에뮬레이션합니다.

### D. 대기 및 동기화 스케줄러 (`host_circle/scheduler.cpp`)
*   **CScheduler**:
    *   에뮬레이션 루프의 `MsSleep()` 및 `Yield()` 호출 시, 내부적으로 `SDL_PollEvent()`를 수행하여 윈도우가 굳지 않게 이벤트를 처리하고 CPU 점유를 일시 양보(`SDL_Delay`)합니다.

### E. 파일 입출력 (`host_circle/fatfs.cpp`)
*   **fatfs/ff.h**:
    *   FatFS 인터페이스 구조체(`FIL`, `FATFS`)와 작동 규격(`f_open`, `f_read`, `f_write` 등)을 POSIX 파일 입출력(`fopen`, `fread` 등)으로 변환해 줍니다.
    *   메모리 카드 탐색 경로인 `SD:/spcconfig.ini` 경로 요청이 들어오면 로컬 디렉토리인 `./sdcard/spcconfig.ini`로 경로를 자동 매핑합니다.

---

## 3. 디렉토리 구조

```
baremetal/
  ├── Makefile.host             # 호스트 컴파일 Makefile
  ├── HOST_CIRCLE.md            # 본 설명 문서
  ├── host_include/             # 모킹 헤더 폴더
  │   ├── circle/
  │   │   ├── types.h           # 에뮬레이터용 정밀 정수 자료형 및 boolean
  │   │   ├── screen.h          # CScreenDevice 선언
  │   │   ├── timer.h           # CTimer 스텁 선언
  │   │   ├── logger.h          # CLogger 디버깅 로그 출력 스텁
  │   │   ├── startup.h         # halt(), reboot() 등의 시스템 API 에뮬레이션
  │   │   ├── util.h            # memory/string 래핑
  │   │   ├── sched/
  │   │   │   └── scheduler.h   # CScheduler 스텁 선언
  │   │   └── usb/
  │   │       ├── usbhcidevice.h
  │   │       └── usbkeyboard.h # CUSBKeyboardDevice 선언
  │   ├── vc4/
  │   │   ├── vchiq/
  │   │   │   └── vchiqdevice.h
  │   │   └── sound/
  │   │       └── vchiqsoundbasedevice.h # CVCHIQSoundBaseDevice 선언
  │   ├── SDCard/
  │   │   └── emmc.h
  │   └── fatfs/
  │       └── ff.h              # FatFS POSIX 파일 입출력 매핑 헤더
  └── host_circle/              # 호스트 연동 구현 소스 파일 (.cpp)
      ├── screen.cpp            # SDL2 비디오 윈도우, 그래픽스 및 키보드 입력 구현
      ├── sound.cpp             # SDL2 Audio 스레드 및 콜백 연동 구현
      ├── scheduler.cpp         # 호스트 Yield 및 MsSleep 동작 연동 구현
      └── fatfs.cpp             # FatFS API의 POSIX 파일 시스템 오버라이딩
```

---

## 4. 빌드 및 테스트 방법

### 필수 패키지 설치
호스트 환경에 SDL2 개발 라이브러리가 미리 설치되어 있어야 합니다. (Debian/Ubuntu 기준)
```bash
sudo apt-get install libsdl2-dev
```

### 컴파일
베어메탈 디렉토리(`/home/msx/spc1000/baremetal/`)에서 아래 명령어를 실행하여 호스트 컴파일 버전을 빌드합니다.
```bash
make -f Makefile.host
```
*   크로스 컴파일된 기존 ARM 임베디드 파일과 오브젝트 꼬임이 없도록 전용 오브젝트 파일인 `*.host.o`를 빌드 타겟으로 활용합니다.
*   빌드가 정상적으로 완료되면 로컬 디렉토리에 **`spc1000.host`** 바이너리 파일이 생성됩니다.

### 실행
로컬 가상 드라이브 경로(`sdcard/`)에 `spcconfig.ini` 및 구동 설정 파일들이 있는지 확인하고 에뮬레이터를 실행합니다.
```bash
./spc1000.host
```

### 소스코드 정리 (Clean)
생성된 호스트 바이너리 및 호스트용 오브젝트 파일을 제거할 수 있습니다.
```bash
make -f Makefile.host clean
```
*   해당 클린 타겟은 임베디드용 베어메탈 파일 빌드 정보(`.o` 등)에는 아무런 영향을 주지 않습니다.
