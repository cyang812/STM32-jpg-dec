# STM32 JPG DECODER

## 项目说明
- 解码器为 [ok-file-formats](https://github.com/cyang812/ok-file-formats)
- 测试平台：STM32F469NIH6
- 测试环境：IAR 7.80.4

## 特性
- 支持如下两种数据源方式，使用 `user_file_buf.h` 中的 `USE_FILE_BUFFER` 进行切换.
- 使用 fatfs 从文件中读取 jpg 数据解码, `dec_jpg()`.
- 从数据流中读取 jpg 数据解码, `dec_jpg_buf()`.
- 支持 RGBA8888(32 bit), RGB888(24 bit) 颜色模式, 使用 `ok_jpg.h` 中的 `JUST_USE_RGB` 进行切换.
