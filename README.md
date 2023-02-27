MAIX Loader Build
======

## Debug
```
mkdir build && cd build
cmake -D CMAKE_BUILD_TYPE=Debug .. &&  make
```

## Release
```
mkdir build && cd build
cmake .. && make
```

## Flash Map (6MB)

Name|Size|Address Range
:-|:-:|:--
 JPEG data       | 192 KB | 0x00450000 ~ 0x005F,FFFF ( 1728 KB )   
 Alive model     | 239 KB | 0x0035,0000 ~ 0x0044,FFFF ( 1024 KB ） 
 Feature model   | 960 KB | 0x0025,0000 ~ 0x0034,FFFF ( 1024 KB)   
 Detect model    | 271 KB | 0x0020,0000 ~ 0x0024,FFFF ( 320 KB)    
 Keypoint model  | 156 KB | 0x001D,0000 ~ 0x001F,FFFF ( 192 KB)    
 aligorithm data | 516 KB | 0x000D,0000 ~ 0x001C,FFFF  ( 1024 KB)  
 BAK             | 320 KB | 0x0008,0000 ~ 0x000C,FFFF (320 KB)     
 APP             | 320 KB | 0x0003,0000 ~ 0x0007,FFFF (320 KB)     
 Stage2\_BAK     | 64 KB  | 0x0002,0000 ~ 0x0002,FFFF ( 64 KB)     
 Stage2\_APP     | 64 KB  | 0x0001,0000 ~ 0x0001,FFFF  (64 KB)     
Stage1|64 KB|0x0000,0000 ~ 0x0000,FFFF  (64 KB）

