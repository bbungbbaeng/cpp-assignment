<h2> 컴퓨터 프로그래밍 기초 C++ 과제</h2>  

<h4>실행 방법 (Homebrew 및 M1 Mac 기준)</h4>
먼저, SFML 2.6.2 버전을 깔아주어야 합니다.  

터미널에 아래의 명령어를 입력해줍니다.  

```bash
$ brew install sfml@2
```

그 다음, cmake 또한 깔아줍니다.

```bash
brew install cmake
```

<br>

그 후, 내려받은 해당 파일의 압축을 해제하고, vscode에서 폴더를 열어줍니다.  

이 때, cpp 파일들을 보면 SFML 헤더를 인식하지 못하고 있는 것을 볼 수 있을 것입니다.

이 문제를 해결해주기 위해서, vscode에서 `Ctrl + Shift + B`를 눌러 검색 창에 `c_cpp_properties.json`과 `tasks.json`을 각각 클릭해줍니다. 

이후에는 해당 폴더에 `.vscode`라는 새로운 폴더가 생기게 되는데, 이 폴더 안에 `c_cpp_properties.json`과 `tasks.json`가 위치하고 있을 것입니다.  

`c_cpp_properties.json`에는 아래의 내용을 그대로 `Ctrl + C, Ctrl + V` 해줍니다.

```json
{
    "configurations": [
        {
            "name": "Mac",
            "includePath": [
                "${workspaceFolder}/**",
                "/opt/homebrew/Cellar/sfml@2/2.6.2_1/include/"
            ],
            "defines": [],
            "macFrameworkPath": [
                "/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/System/Library/Frameworks"
            ],
            "compilerPath": "/usr/bin/clang",
            "cStandard": "c17",
            "cppStandard": "c++17",
            "intelliSenseMode": "macos-clang-arm64"
        }
    ],
    "version": 4
}
```

`tasks.json`에도 아래의 내용을 그대로 `Ctrl + C, Ctrl + V` 해줍니다.

```json
{
    "version": "2.0.0",
    "tasks": [
        {
            "type": "shell",
            "label": "clang++ build active file",
            "command": "/usr/bin/clang++",
            "args": [
                "-std=c++17",
                "-stdlib=libc++",
                "-g",
                "${file}",
                // SFML dependency including begin
                "-I/opt/homebrew/Cellar/sfml@2/2.6.2_1/include",
                "-L/opt/homebrew/Cellar/sfml@2/2.6.2_1/lib",
                "-lsfml-window",
                "-lsfml-system",
                "-lsfml-graphics",
                // SFML dependency including end
                "-o",
                "${fileDirname}/${fileBasenameNoExtension}",
            ],
            "options": {
                "cwd": "${workspaceFolder}"
            },
            "problemMatcher": [
                "$gcc"
            ],
            "group": "build"
        },
		{
			"label": "Open Terminal",
			"type": "shell",
			"command": "osascript -e 'tell application \"Terminal\"\ndo script \"echo hello\"\nend tell'",
			"problemMatcher": []
		}
    ]
}
```

<br>

그 다음, vscode 내에서 터미널을 열어준 후에, 아래의 명령어들을 순서대로 입력해줍니다.  

```bash
$ mkdir build
$ cd build
$ cmake ..
$ cmake --build .
```

<br>

위의 작업들이 완료되고 나면, 아래의 명령어를 터미널에 입력함으로써 프로그램을 실행할 수 있습니다. (단, 터미널에서 현재 위치가 build 폴더여야 합니다.)

```bash
$ ./iaps
```