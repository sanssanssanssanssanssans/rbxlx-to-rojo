# rbx-to-lua

RBXML / RBMX 파일에서 Lua 스크립트를 추출해서 Rojo 스타일로 변환합니다.

## 빌드

make

## 사용법

```
./rbx-to-lua input.rbxlx -o out
```

## 특징

- 자체 XML 파서를 내장합니다.
- script / localscript / modulescript를 추출합니다.