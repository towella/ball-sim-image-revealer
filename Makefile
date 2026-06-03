# https://opensource.com/article/18/8/what-how-makefile
# https://www3.ntu.edu.sg/home/ehchua/programming/cpp/gcc_make.html
# phony target all can run multiple other targets
all: build generate

build:
# https://medium.com/@edkins.sarah/set-up-sdl2-on-your-mac-without-xcode-6b0c33b723f7
	@echo "Building..."
	@g++ -std=c++17 -Wall -Werror -O0 source/*.cpp -I"source/*.hpp" -I"include" -L"lib" -l SDL2-2.0.0 -l SDL2_image-2.0.0 -o ballSim
	
run:
	@echo "Running..."
	@./ballSim

clean:
	@rm ballSim
	@rm *.mov
	@echo "Clean complete"

generate:
# https://ffmpeg.org/ffmpeg.html#Main-options
	@echo "Generating video..."
# pipe output into ffmpef
# -y    overwrite any existing files without asking
# -f    force the output to be raw video
# -i    input url ('-' since piping in byte stream)
# -c:v  set video codec to h264
	@./ballSim | ./include/ffmpeg -y -f rawvideo -pixel_format rgb24 -video_size 720x1280 -i - -c:v h264 -pix_fmt yuv420p video.mov
	@echo "Generation complete"
