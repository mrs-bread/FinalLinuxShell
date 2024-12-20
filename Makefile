SRC=main.cpp
TARGET=shell
PACKAGE_NAME = myshell
PATH_TO_PACKAGE = $(PACKAGE_NAME)/usr/local/bin/

all: run

$(PATH_TO_PACKAGE)$(TARGET): $(SRC)
	mkdir -p $(PATH_TO_PACKAGE)
	g++ -g $(SRC) -o $(PATH_TO_PACKAGE)$(TARGET) $(LIBS)

run: $(PATH_TO_PACKAGE)$(TARGET)
	./$(PATH_TO_PACKAGE)$(TARGET)

create_package: $(PATH_TO_PACKAGE)$(TARGET)
	dpkg-deb --build $(PACKAGE_NAME)
	mkdir -p packages
	mv $(PACKAGE_NAME).deb packages

CRONFS_SRC = cronfs.cpp
CRONFS_TARGET = cronfs
CRONFS_PATH = $(PACKAGE_NAME)/usr/local/bin/

$(CRONFS_TARGET): $(CRONFS_SRC)
	mkdir -p $(CRONFS_PATH)
	g++ -g $(CRONFS_SRC) -o $(CRONFS_PATH)$(CRONFS_TARGET) -lfuse

run_cronfs: $(CRONFS_TARGET)
	./$(CRONFS_TARGET) $(MOUNTPOINT)
