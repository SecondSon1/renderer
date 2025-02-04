# renderer

### Чтобы запустить:

Поменять PATH_TO_PROJECT_ROOT в main.cpp. Так как я пишу и запускаю код
в Visual Studio, оно запускает его глубоко, и то, как выйти в корневую директорию,
подобрано эмпирически. Если надо узнать CWD (current working directory),
поставьте туда "." и запустите код, в логах первой строчкой выведет CWD.

Далее:

```bash
mkdir build
cd build
cmake ..
cmake --build . --parallel 16
```
