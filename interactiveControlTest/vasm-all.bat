for %%f in (*.asm) do vasm -Fbin -pad=255 "%%f" -o "%%~nf.bin" -L "%%~nf.lst"
