python .\utils\version_get.py guard.bin version.bin
python .\utils\version_hw_get.py guard.bin version_hw.bin
python .\utils\genimg_single.py guard.bin guard_s.bin
python .\utils\version_add.py guard_s.bin version_hw.bin
python .\utils\version_add.py guard_s.bin version.bin
pause