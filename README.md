# SPC-1000 & Raspberry Pi Extension Box 프로젝트

SPC-1000은 1982년 삼성전자에서 출시한 최초의 8비트 개인용 컴퓨터입니다. 본 저장소는 SPC-1000 컴퓨터와 이를 확장 및 현대화하기 위한 **라즈베리 파이 기반 확장 박스(Extension Box/SPCBox)** 하드웨어/소프트웨어 프로젝트, 그리고 각종 주변 에뮬레이션 및 데이터 변환 도구들을 통합하여 관리하는 프로젝트입니다.

---

## 📋 핵심 사양 (SPC-1000 Specification)
- **CPU**: Z80A @ 4MHz, 64KB RAM (페이지 메모리 없음)
- **VDP (Video)**: MC6847 (16KB VRAM 사용)
- **PSG (Sound)**: AY-8910 (3채널 사운드)
- **Storage**: 내장형 카세트 데크 기본 탑재

---

## 🚀 고완성도 완료 프로젝트 요약 (High-Completeness Subprojects)

현재 본 저장소에서 구현 완료 및 실사용 검증을 마친 핵심 프로젝트들의 목록과 상세 기능입니다.

### 1. SPC-1000 에뮬레이터 제품군
*   **💻 [spcemul](file:///home/msx/spc1000/spcemul) - Windows/Linux 네이티브 에뮬레이터**:
    *   Z80 CPU, MC6847 VDP, AY-8910 PSG 사운드 에뮬레이션 완벽 동작
    *   카세트 테이프 읽기/쓰기 모터 제어 및 GUI 메뉴 지원
    *   두 번째 VDP 확장 카드(TMS9918 VDP Unit/Soft Box) 지원
*   **🌐 [sdl2](file:///home/msx/spc1000/sdl2) - SDL2 & WebAssembly 웹 에뮬레이터**:
    *   최신 SDL2 라이브러리로 이식되어 리눅스 및 임베디드 디바이스 빌드 지원
    *   **Emscripten(WebAssembly)** 컴파일을 완벽히 지원하여, 웹 브라우저에서 플러그인 없이 실행 가능한 파일(`index.js`, `index.wasm`, `index.data`) 및 전용 가상 키보드 인터페이스([keyboard.html](file:///home/msx/spc1000/sdl2/keyboard.html)) 구현
*   **🔌 [spc1000_circle](file:///home/msx/spc1000/spc1000_circle) & [baremetal](file:///home/msx/spc1000/baremetal) - 라즈베리 파이 베어메탈 에뮬레이터**:
    *   OS 없이 라즈베리 파이(RPi 1, Zero 등)에서 직접 부팅하는 **Bare-metal** 펌웨어(`kernel.img`) 빌드 지원
    *   Circle 프레임워크 기반으로 USB 키보드 입력, MC6847 비디오 렌더링, PWM/I2S 사운드(3.5mm 잭 또는 external DAC) 고속 출력 지원

### 2. 라즈베리 파이 확장 박스 (Extension Box) & FDD 에뮬레이션
*   **📦 [spcbox_bm](file:///home/msx/spc1000/spcbox_bm) / [rpibox](file:///home/msx/spc1000/rpibox) - SPCBox GPIO 베어메탈 드라이버**:
    *   라즈베리 파이의 GPIO 핀을 SPC-1000의 확장 슬롯 버스 신호와 물리적으로 직접 결합하여 구동하는 저수준 베어메탈 제어 소프트웨어
*   **💾 [spcbios](file:///home/msx/spc1000/spcbios) - 확장 박스 가상 디스크/BIOS**:
    *   가상 SD-725 인텔리전트 플로피 디스크 드라이브(FDD) 에뮬레이터 탑재
    *   D88 플로피 디스크 포맷(`.d88` / `system.d88`) 이미지 마운트, 파일 시스템 읽기/쓰기 서비스 및 가상 바이오스 제공
*   **🔌 [spcfloppy](file:///home/msx/spc1000/spcfloppy) - PC-FDD 시리얼 통신 유틸리티**:
    *   Windows 환경에서 아두이노 등을 통해 SPC-1000 본체와 직렬 연결하여 가상 SD-725 플로피 드라이브를 모방 및 제어하는 프로그램

### 3. CP/M 2.2 운영체제 포트 ([cpm](file:///home/msx/spc1000/cpm))
*   8비트 표준 운영체제인 **CP/M 2.2**를 SPC-1000 환경에 이식 완료
*   Z80 어셈블리어로 제작된 CP/M 부트로더(`spc-1000_cpm_boot.asm`) 및 BIOS(`spc-1000_cpm_bios.asm`) 소스코드 포함
*   플로피 디스크 이미지 생성, 파일 읽기/쓰기 및 관리용 툴킷([/cpm/bin](file:///home/msx/spc1000/cpm/bin)) 완비

### 4. 카세트 테이프 & 데이터 분석 유틸리티
*   **📼 [tape](file:///home/msx/spc1000/tape) - 테이프 오디오 분석 및 변환**:
    *   실제 카세트테이프 녹음 데이터(WAV)를 주파수 피크 추출을 통해 디지털 `.TAP` 에뮬레이터 이미지로 디코딩 (`wav2tap.py`, `detect_peaks.py`)
    *   TAP 파일의 구조를 진단하고 내부 기계어 바이너리를 덤프하는 유틸리티 완비
*   **🔄 [txt2bas](file:///home/msx/spc1000/txt2bas) & [bas2txt](file:///home/msx/spc1000/bas2txt) - BASIC 코드 컴파일러/역컴파일러**:
    *   **bas2txt**: Hu-BASIC 토큰 바이너리 파일(`.tap` 등)을 해독하여 사람이 읽을 수 있는 BASIC 소스코드 텍스트 파일(`.txt`)로 역변환 (`bas2txt.py`)
    *   **txt2bas**: PC 텍스트 파일로 코딩한 BASIC 소스를 SPC-1000에서 바로 로드 가능한 토큰 형태의 테이프 바이너리 및 TAP 파일로 변환

### 5. 공식 MAME 에뮬레이터 통합 패치 ([mamesrc](file:///home/msx/spc1000/mamesrc) / [mame](file:///home/msx/spc1000/mame))
*   공식 멀티 에뮬레이터 프로젝트 MAME(구 MESS) 소스에 SPC-1000 하드웨어 지원 코드를 기여하기 위한 드라이버 및 패치 관리 스크립트 제공

---

## 📂 디렉토리 구조 요약 (Directory Map)

*   `spcemul/` - SDL1 기반 Windows/Linux 네이티브 에뮬레이터
*   `sdl2/` - SDL2 & WebAssembly(Emscripten) 지원 현대식 에뮬레이터
*   `baremetal/` - 라즈베리 파이 하드웨어 독립형(Bare-metal) 에뮬레이터
*   `spc1000_circle/` - Circle 라이브러리 탑재 RPi 베어메탈 소스코드
*   `spcbox_bm/` & `rpibox/` - 라즈베리 파이 확장 박스 드라이버 소프트웨어
*   `spcbios/` - 확장 박스 탑재 가상 드라이브 펌웨어 및 BIOS
*   `spcfloppy/` - PC-FDD 직렬연결 통신 유틸리티 (Windows용)
*   `cpm/` - SPC-1000용 CP/M 2.2 운영체제 파일 및 디스크 유틸리티
*   `tape/` - 실물 카세트테이프 WAV 파일 디코딩 및 TAP 가공 스크립트 모음
*   `txt2bas/` & `bas2txt/` - BASIC 토큰화 인코더 및 디코더
*   `mamesrc/` & `mame/` - MAME 드라이버 패치 및 리소스
*   `docs/` - 회로도, 확장기 커넥터 연결도, 관련 개발 문서 (주로 한국어)
*   `tools/` - Z80 바이너리 변환 및 가상 디스크 생성 유틸리티
