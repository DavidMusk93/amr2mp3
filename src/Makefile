all: main

TEST_MODE := 1

.PHONY: main

main:
	ndk-build NDK_PROJECT_PATH=. NDK_APPLICATION_MK=./Application.mk TEST=$(TEST_MODE)
	bash helper.sh finish

.PHONY: data

data:
	g++ -std=c++17 ./SafeString.cc ./Logger.cc ./StringConverter.cc ./Reader.cc && ./a.out && rm ./a.out

.PHONY: clean

clean:
	rm -rf ./data.h ./obj ./libs ./a.out
